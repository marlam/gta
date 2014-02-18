/*
 * Copyright (C) 2014
 * Martin Lambers <marlam@marlam.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <GL/glew.h>

#include <QKeyEvent>

#include "base/dbg.h"
#include "base/msg.h"

#include "xgl/glvm.hpp"
#include "xgl/navigator.hpp"

#include "glwidget.hpp"


GLWidget::GLWidget(GLRendererFactory* glrenderer_factory, QWidget* parent, XQGLWidget* sharing_widget) :
    XQGLWidget(glrenderer_factory, this, parent, sharing_widget)
{
    setMinimumSize(QSize(64, 64));
    set_active_frame_color(Qt::red);
    // all scenes are always centered around 0 and have radius 1
    _navigator.set_scene(glvm::vec3(0.0f), 1.0f);
    _focal_length = glvm::length(_navigator.get_viewer_pos());
}

bool GLWidget::scene_is_2d() const
{
    return _view_params.mode_is_2d();
}

void GLWidget::scene_view_2d(
        glvm::vec2& translation_xy, glvm::vec3& scale) const
{
    assert(scene_is_2d());
    translation_xy = _navigator.get_translation_2d();
    scale = glvm::vec3(_navigator.get_scale_2d());
}

void GLWidget::scene_view_3d(glvm::frust& frustum,
        glvm::vec3& viewer_pos, glvm::quat& viewer_rot,
        float& focal_length, float& eye_separation) const
{
    assert(!scene_is_2d());
    frustum = glvm::perspective(glvm::radians(50.0f), aspect_ratio(), 0.1f, 100.0f);
    viewer_pos = _navigator.get_viewer_pos();
    viewer_rot = _navigator.get_viewer_rot();
    focal_length = _focal_length;
    eye_separation = focal_length / 30.0f;
}

void GLWidget::scene_prerender()
{
    glViewport(0, 0, width(), height());
    _navigator.set_viewport(glvm::ivec4(0, 0, width(), height()));
}

void GLWidget::keyPressEvent(QKeyEvent* event)
{
    // Let the super class handle generic keys
    XQGLWidget::keyPressEvent(event);
    if (event->key() == Qt::Key_Space) {
        _navigator.reset();
        trigger_rendering();
    }
}

void GLWidget::mousePressEvent(QMouseEvent* event)
{
    glvm::ivec2 pos = glvm::ivec2(event->pos().x(), event->pos().y());
    if (scene_is_2d()) {
        if (event->buttons() & Qt::LeftButton || event->buttons() & Qt::MidButton)
            _navigator.start_shift_2d(pos);
        else if (event->buttons() & Qt::RightButton)
            _navigator.start_zoom_2d(pos);
    } else {
        if (event->buttons() & Qt::LeftButton)
            _navigator.start_rot(pos);
        else if (event->buttons() & Qt::MidButton)
            _navigator.start_shift(pos);
        else if (event->buttons() & Qt::RightButton)
            _navigator.start_zoom(pos);
    }
}

void GLWidget::mouseMoveEvent(QMouseEvent* event)
{
    bool changed = false;
    glvm::ivec2 pos = glvm::ivec2(event->pos().x(), event->pos().y());
    if (scene_is_2d()) {
        if (event->buttons() & Qt::LeftButton || event->buttons() & Qt::MidButton) {
            _navigator.shift_2d(pos);
            changed = true;
        } else if (event->buttons() & Qt::RightButton) {
            _navigator.zoom_2d(pos);
            changed = true;
        }
    } else {
        if (event->buttons() & Qt::LeftButton) {
            _navigator.rot(pos);
            changed = true;
        } else if (event->buttons() & Qt::MidButton) {
            _navigator.shift(pos);
            changed = true;
        } else if (event->buttons() & Qt::RightButton) {
            _navigator.zoom(pos);
            changed = true;
        }
    }
    if (changed)
        trigger_rendering();
}

void GLWidget::wheelEvent(QWheelEvent* event)
{
#if QT_VERSION < 0x050000
    float degrees = event->delta() / 8.0f;
#else
    float degrees = event->angleDelta().y() / 8.0f;
#endif
    float rad = glvm::radians(degrees);
    if (scene_is_2d()) {
        _navigator.zoom_2d(rad);
    } else {
        _navigator.zoom(rad);
    }
    trigger_rendering();
}

void GLWidget::set_view_params(const ViewParameters& view_params)
{
    _view_params = view_params;
}
