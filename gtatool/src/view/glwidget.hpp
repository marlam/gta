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

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "config.h"

#include "xqglwidget.hpp"
#include "viewparams.hpp"
#include "xgl/navigator.hpp"


class GLWidget : public XQGLWidget, public GLNavigator
{
    Q_OBJECT

private:
    ViewParameters _view_params;
    Navigator _navigator;
    float _focal_length;

public:
    GLWidget(GLRendererFactory* glrenderer_factory, QWidget* parent = NULL, XQGLWidget* sharing_widget = NULL);

    // GLNavigator interfaces
    virtual bool scene_is_2d() const;
    virtual void scene_view_2d(glvm::vec2& translation_xy, glvm::vec3& scale) const;
    virtual void scene_view_3d(glvm::frust& frustum,
            glvm::vec3& viewer_pos, glvm::quat& viewer_rot,
            float& focal_length, float& eye_separation) const;

    // XQGLWidget interfaces
    virtual void scene_prerender();

    // Interaction
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent* event);

public slots:
    void set_view_params(const ViewParameters& view_parameters);
};

#endif
