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

#ifndef XQGLWIDGET_H
#define XQGLWIDGET_H

#include "config.h"

#include <GL/glew.h>

#include <QFrame>
#include <QGLFormat>

#if WITH_GLS
# include <gls/gls.h>
#endif

#include "xgl/glvm.hpp"
#include "xgl/glwindow.hpp"
#include "xgl/glrenderer.hpp"


class XQGLEmbeddedGLWidget;
class QImage;
class QThread;

class XQGLWidget : public QFrame, public GLWindow
{
    Q_OBJECT

    friend class XQGLEmbeddedGLWidget;

private:
    // Embedded GL widget
    XQGLEmbeddedGLWidget* _glwidget;
    // The widget that shares our GL context (==this if we don't share)
    XQGLWidget* _sharing_widget;
    // Frame color management
    QColor _default_frame_color;
    QColor _active_frame_color;
    // Fullscreen management
    int _fullscreen_screens;
    bool _fullscreen;
    QRect _fullscreen_geom_bak;
    QColor _fullscreen_frame_color_bak;
    // Stereoscopic rendering management
#if WITH_GLS
    GLScontext* _gls_ctx;
    bool _gls_reinitialize;
    bool _gls_active;
    GLSmode _gls_mode;
    bool _gls_half;
    bool _gls_swap_eyes;
    bool _gls_last_active;
    GLSmode _gls_last_mode;
    bool _gls_last_half;
    bool _gls_last_swap_eyes;
    QPoint _gls_last_pos;
#endif

protected:
    virtual void focusInEvent(QFocusEvent*);

    // Helper functions
    bool fullscreen() const;
    void enter_fullscreen();
    void exit_fullscreen();
    int pos_x() const;
    int pos_y() const;
    float aspect_ratio() const { return static_cast<float>(width()) / height(); }

public:
    XQGLWidget(GLRendererFactory* glrenderer_factory, const GLNavigator* navigator, QWidget* parent = NULL, XQGLWidget* sharing_widget = NULL);
    ~XQGLWidget();

    // GLWindow interface
    virtual void make_window_current();
    virtual void done_window_current();
    virtual void make_shared_current();
    virtual void done_shared_current();
    virtual void swap_buffers();
    virtual bool needs_rendering();

    // Stereoscopic rendering support
    virtual void render();

    // This is the only thing you need to implement in a subclass:
    // Setup OpenGL state before rendering a frame. Usually only sets the viewport.
    virtual void scene_prerender() = 0;

    // Trigger re-rendering, e.g. on navigation events
    void trigger_rendering();

    // Advanced functionality
public slots:
    void set_active_frame_color(const QColor& color);
    void mark_active(bool active);
    void set_fullscreen_conf(int screens);
    void set_stereo3d_conf(int mode, bool half, bool swap_eyes);
public:
    static QGLFormat get_required_format(int mode);
    QImage* get_current_image();

    // Interaction
public:
    virtual void keyPressEvent(QKeyEvent* event);

signals:
    void got_focus(XQGLWidget*);
};

#endif
