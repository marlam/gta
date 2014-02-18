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

#include <QGLWidget>
#include <QMessageBox>
#include <QGridLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QImage>
#include <QKeyEvent>
#include <QThread>

#include "base/dbg.h"
#include "base/msg.h"

#include "xqglwidget.hpp"


class XQGLEmbeddedGLWidget : public QGLWidget
{
public:
    bool needs_rendering;

    XQGLEmbeddedGLWidget(QWidget* parent, XQGLEmbeddedGLWidget* sharing_widget = NULL) :
        QGLWidget(parent, sharing_widget), needs_rendering(true)
    {
        setAutoBufferSwap(false);
    }

protected:
    // Make sure that Qt does not get in our way: override all overridable
    // functions so that Qt cannot steal our GL context
    virtual void glDraw() {}
    virtual void glInit() {}
    virtual void initializeGL() {}
    virtual void initializeOverlayGL() {}
    virtual void paintGL() {}
    virtual void paintOverlayGL() {}
    virtual void resizeGL(int, int) {}
    virtual void resizeOverlayGL(int, int) {}
    virtual void updateGL() {}
    virtual void updateOverlayGL() {}
    virtual void actionEvent(QActionEvent*) {}
    virtual void changeEvent(QEvent*) {}
    virtual void closeEvent(QCloseEvent*) {}
    virtual void contextMenuEvent(QContextMenuEvent*) {}
    virtual void dragEnterEvent(QDragEnterEvent*) {}
    virtual void dragLeaveEvent(QDragLeaveEvent*) {}
    virtual void dragMoveEvent(QDragMoveEvent*) {}
    virtual void dropEvent(QDropEvent*) {}
    virtual void enterEvent(QEvent*) {}
    virtual void hideEvent(QHideEvent*) {}
    virtual void inputMethodEvent(QInputMethodEvent*) {}
    virtual void keyPressEvent(QKeyEvent* e) { static_cast<XQGLWidget*>(parentWidget())->keyPressEvent(e); }
    virtual void keyReleaseEvent(QKeyEvent* e) { static_cast<XQGLWidget*>(parentWidget())->keyReleaseEvent(e); }
    virtual void leaveEvent(QEvent*) {}
    virtual void mouseDoubleClickEvent(QMouseEvent* e) { static_cast<XQGLWidget*>(parentWidget())->mouseDoubleClickEvent(e); }
    virtual void mouseMoveEvent(QMouseEvent* e) { static_cast<XQGLWidget*>(parentWidget())->mouseMoveEvent(e); }
    virtual void mousePressEvent(QMouseEvent* e) { static_cast<XQGLWidget*>(parentWidget())->mousePressEvent(e); }
    virtual void mouseReleaseEvent(QMouseEvent* e) { static_cast<XQGLWidget*>(parentWidget())->mouseReleaseEvent(e); }
    virtual void moveEvent(QMoveEvent*) {}
    virtual void paintEvent(QPaintEvent*) { needs_rendering = true; }
    virtual void resizeEvent(QResizeEvent*) { needs_rendering = true; }
    virtual void showEvent(QShowEvent*) {}
    virtual void tabletEvent(QTabletEvent* e) { static_cast<XQGLWidget*>(parentWidget())->tabletEvent(e); }
    virtual void wheelEvent(QWheelEvent* e) { static_cast<XQGLWidget*>(parentWidget())->wheelEvent(e); }
};

XQGLWidget::XQGLWidget(GLRendererFactory* glrenderer_factory, const GLNavigator* navigator, QWidget* parent, XQGLWidget* sharing_widget) :
    QFrame(parent), GLWindow(navigator),
    _default_frame_color(palette().windowText().color()),
    _active_frame_color(_default_frame_color),
    _fullscreen_screens(0),
    _fullscreen(false)
#if WITH_GLS
    ,
    _gls_ctx(NULL),
    _gls_reinitialize(false),
    _gls_active(false),
    _gls_mode(GLS_MODE_RED_CYAN_DUBOIS),
    _gls_half(false),
    _gls_swap_eyes(false),
    _gls_last_active(_gls_active)
#endif
{
    if (sharing_widget) {
        _glwidget = new XQGLEmbeddedGLWidget(this, sharing_widget->_glwidget);
        _sharing_widget = sharing_widget;
        assert(_glwidget->isSharing());
        assert(_sharing_widget->_glwidget->isSharing());
        set_shared_context(_sharing_widget->get_shared_context());
    } else {
        _glwidget = new XQGLEmbeddedGLWidget(this);
        _sharing_widget = this;
        set_shared_context(new GLContext(glrenderer_factory));
    }

    QGridLayout *layout = new QGridLayout();
    layout->addWidget(_glwidget, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setRowStretch(0, 1);
    layout->setColumnStretch(0, 1);
    setLayout(layout);
    setFocusPolicy(Qt::StrongFocus);
    setFrameStyle(QFrame::Box | QFrame::Plain);
    setMouseTracking(true);

    show();
    QApplication::processEvents(); // force window to become visible and GL context to become valid
    if (!_glwidget->context()->isValid()) {
        QMessageBox::critical(this, "Error", "Cannot get valid OpenGL context.");
        std::exit(1);
    }
}

XQGLWidget::~XQGLWidget()
{
    if (_sharing_widget == this)
        delete get_shared_context();
}

void XQGLWidget::focusInEvent(QFocusEvent*)
{
    emit got_focus(this);
}

bool XQGLWidget::fullscreen() const
{
    return _fullscreen;
}

void XQGLWidget::enter_fullscreen()
{
    if (!_fullscreen) {
        if (windowFlags() & Qt::Window) {
            // Save the window geometry here so that we can restore it later.
            _fullscreen_geom_bak = geometry();
        } else {
            // Turn this into a window.
            _fullscreen_geom_bak = QRect();
            setWindowFlags(Qt::Window);
        }
        // Determine combined geometry of the chosen screens.
        int screens = _fullscreen_screens;
        int screen_count = 0;
        QRect geom;
        for (int i = 0; i < std::min(QApplication::desktop()->screenCount(), 16); i++) {
            if (screens & (1 << i)) {
                if (geom.isNull())
                    geom = QApplication::desktop()->screenGeometry(i);
                else
                    geom = geom.united(QApplication::desktop()->screenGeometry(i));
                screen_count++;
            }
        }
        if (geom.isNull()) {
            // Use default screen
            geom = QApplication::desktop()->screenGeometry(-1);
        }
        Qt::WindowFlags new_window_flags = windowFlags()
            | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint;
        // In the dual and multi screen cases we need to bypass the window manager
        // on X11 because Qt does not support _NET_WM_FULLSCREEN_MONITORS, and thus
        // the window manager would always restrict the fullscreen window to one screen.
        // Note: it may be better to set _NET_WM_FULLSCREEN_MONITORS ourselves, but that
        // would also require the window manager to support this extension...
        if (screen_count > 1)
            new_window_flags |= Qt::X11BypassWindowManagerHint;
        setWindowFlags(new_window_flags);
        setWindowState(windowState() | Qt::WindowFullScreen);
        setGeometry(geom);
        show();
        raise();
        activateWindow();
        _fullscreen = true;
        setFocus(Qt::OtherFocusReason);
    }
}

void XQGLWidget::exit_fullscreen()
{
    if (_fullscreen) {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
        Qt::WindowFlags new_flags = windowFlags();
        new_flags &= ~Qt::X11BypassWindowManagerHint;
        new_flags &= ~Qt::FramelessWindowHint;
        new_flags &= ~Qt::WindowStaysOnTopHint;
        // Re-embed us into the parent widget if necessary
        if (_fullscreen_geom_bak.isNull()) {
            new_flags &= ~Qt::Window;
            new_flags |= Qt::Widget;
        }
        setWindowFlags(new_flags);
        if (!_fullscreen_geom_bak.isNull())
            setGeometry(_fullscreen_geom_bak);
        QApplication::processEvents();
        show();
        raise();
        _fullscreen = false;
        setFocus(Qt::OtherFocusReason);
    }
}

int XQGLWidget::pos_x() const
{
    if (_fullscreen)
        return 0;
    else
        return mapToGlobal(QPoint(0, 0)).x();
}

int XQGLWidget::pos_y() const
{
    if (_fullscreen)
        return 0;
    else
        return QApplication::desktop()->screenGeometry(this).height() - 1
            - mapToGlobal(QPoint(0, height() - 1)).y();
}

void XQGLWidget::make_window_current()
{
    _glwidget->makeCurrent();
    assert(_glwidget->context()->isValid());
}

void XQGLWidget::done_window_current()
{
    _glwidget->doneCurrent();
}

void XQGLWidget::make_shared_current()
{
    _sharing_widget->_glwidget->makeCurrent();
    assert(_sharing_widget->_glwidget->context()->isValid());
}

void XQGLWidget::done_shared_current()
{
    _sharing_widget->_glwidget->doneCurrent();
}

void XQGLWidget::swap_buffers()
{
    _glwidget->swapBuffers();
}

bool XQGLWidget::needs_rendering()
{
    if (_glwidget->needs_rendering)
        return true;
#if WITH_GLS
    if (_gls_active != _gls_last_active)
        return true;
    if (!_gls_active)
        return false;
    if (_gls_mode == GLS_MODE_ALTERNATING || _gls_mode != _gls_last_mode)
        return true;
    if (_gls_half != _gls_last_half)
        return true;
    if (_gls_swap_eyes != _gls_last_swap_eyes)
        return true;
    if ((_gls_mode == GLS_MODE_EVEN_ODD_ROWS
                || _gls_mode == GLS_MODE_EVEN_ODD_COLUMNS
                || _gls_mode == GLS_MODE_CHECKERBOARD)
            && mapToGlobal(QPoint(0, 0)) != _gls_last_pos)
        return true;
#endif
    return false;
}

void XQGLWidget::render()
{
    glvm::frust frustum;
    glvm::vec2 translation_2d;
    glvm::vec3 scale_2d;
    glvm::vec3 viewer_pos;
    glvm::quat viewer_rot;
    float focal_length;
    float eye_separation;

    if (get_navigator()->scene_is_2d()) {
        get_navigator()->scene_view_2d(translation_2d, scale_2d);
        float l = -1.0f, r = +1.0f, b = -1.0f, t = +1.0f;
        float ar = static_cast<float>(width()) / height();
        if (ar > 1.0f) {
            l = -ar;
            r = +ar;
        } else if (ar < 1.0f) {
            b = -(1.0f / ar);
            t = +(1.0f / ar);
        }
        frustum = glvm::frust(l, r, b, t, -1.0f, +1.0f);
    } else {
        get_navigator()->scene_view_3d(frustum, viewer_pos, viewer_rot, focal_length, eye_separation);
    }
    scene_prerender();
#if WITH_GLS
    if (_gls_active) {
        if (_gls_reinitialize && _gls_ctx) {
            glsDestroyContext(_gls_ctx);
            _gls_ctx = NULL;
        }
        if (!_gls_ctx)
            _gls_ctx = glsCreateContext();
        if (!_gls_ctx)
            dbg::crash();
        glsClear(_gls_ctx);
        glsSetViewportScreenCoords(_gls_ctx, pos_x(), pos_y());
        glvm::vec3 rightside = viewer_rot * glvm::vec3(1.0f, 0.0f, 0.0f);
        if (get_navigator()->scene_is_2d()) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(frustum.l(), frustum.r(), frustum.b(), frustum.t(), frustum.n(), frustum.f());
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glTranslatef(translation_2d.x, translation_2d.y, 0.0f);
            glScalef(scale_2d.x, scale_2d.y, scale_2d.z);
            get_shared_context()->get_renderer()->render();
            if (glsIsViewRequired(_gls_ctx, _gls_mode, _gls_swap_eyes, GLS_VIEW_LEFT))
                glsSubmitView(_gls_ctx, GLS_VIEW_LEFT);
            if (glsIsViewRequired(_gls_ctx, _gls_mode, _gls_swap_eyes, GLS_VIEW_RIGHT))
                glsSubmitView(_gls_ctx, GLS_VIEW_RIGHT);
        } else {
            if (glsIsViewRequired(_gls_ctx, _gls_mode, _gls_swap_eyes, GLS_VIEW_LEFT)) {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                glsFrustum(frustum.l(), frustum.r(), frustum.b(), frustum.t(), frustum.n(), frustum.f(),
                        focal_length, eye_separation, GLS_VIEW_LEFT);
                glMatrixMode(GL_MODELVIEW);
                glLoadMatrixf(glvm::translate(glvm::toMat4(-viewer_rot),
                            -(viewer_pos - eye_separation / 2.0f * rightside)).vl);
                get_shared_context()->get_renderer()->render();
                glsSubmitView(_gls_ctx, GLS_VIEW_LEFT);
            }
            if (glsIsViewRequired(_gls_ctx, _gls_mode, _gls_swap_eyes, GLS_VIEW_RIGHT)) {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                glsFrustum(frustum.l(), frustum.r(), frustum.b(), frustum.t(), frustum.n(), frustum.f(),
                        focal_length, eye_separation, GLS_VIEW_RIGHT);
                glMatrixMode(GL_MODELVIEW);
                glLoadMatrixf(glvm::translate(glvm::toMat4(-viewer_rot),
                            -(viewer_pos + eye_separation / 2.0f * rightside)).vl);
                get_shared_context()->get_renderer()->render();
                glsSubmitView(_gls_ctx, GLS_VIEW_RIGHT);
            }
        }
        GLint vp_bak[4];
        if ((_gls_mode == GLS_MODE_LEFT_RIGHT || _gls_mode == GLS_MODE_TOP_BOTTOM) && !_gls_half) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glGetIntegerv(GL_VIEWPORT, vp_bak);
            if (_gls_mode == GLS_MODE_LEFT_RIGHT)
                glViewport(vp_bak[0], vp_bak[1] + vp_bak[3] / 4, vp_bak[2], vp_bak[3] / 2);
            else
                glViewport(vp_bak[0] + vp_bak[2] / 4, vp_bak[1], vp_bak[2] / 2, vp_bak[3]);
        }
        glsDrawSubmittedViews(_gls_ctx, _gls_mode, _gls_swap_eyes);
        if ((_gls_mode == GLS_MODE_LEFT_RIGHT || _gls_mode == GLS_MODE_TOP_BOTTOM) && !_gls_half) {
            glViewport(vp_bak[0], vp_bak[1], vp_bak[2], vp_bak[3]);
        }
        _gls_last_mode = _gls_mode;
        _gls_last_half = _gls_half;
        _gls_last_swap_eyes = _gls_swap_eyes;
        _gls_last_pos = mapToGlobal(QPoint(0, 0));
    } else {
#endif
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (get_navigator()->scene_is_2d()) {
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(frustum.l(), frustum.r(), frustum.b(), frustum.t(), frustum.n(), frustum.f());
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glTranslatef(translation_2d.x, translation_2d.y, 0.0f);
            glScalef(scale_2d.x, scale_2d.y, scale_2d.z);
        } else {
            glMatrixMode(GL_PROJECTION);
            glLoadMatrixf(glvm::toMat4(frustum).vl);
            glMatrixMode(GL_MODELVIEW);
            glLoadMatrixf(glvm::translate(glvm::toMat4(-viewer_rot), -viewer_pos).vl);
        }
        get_shared_context()->get_renderer()->render();
#if WITH_GLS
    }
    _gls_last_active = _gls_active;
#endif
    _glwidget->needs_rendering = false;
}

void XQGLWidget::trigger_rendering()
{
    _glwidget->needs_rendering = true;
}

void XQGLWidget::set_active_frame_color(const QColor& color)
{
    _active_frame_color = color;
}

void XQGLWidget::mark_active(bool active)
{
    QPalette p(palette());
    p.setColor(QPalette::WindowText,
            (active && !_fullscreen) ? _active_frame_color : _default_frame_color);
    setPalette(p);
}

void XQGLWidget::set_fullscreen_conf(int screens)
{
    _fullscreen_screens = screens;
}

QGLFormat XQGLWidget::get_required_format(int gls_mode)
{
    QGLFormat fmt(
              QGL::DoubleBuffer
            | QGL::DepthBuffer
            | QGL::Rgba
            | QGL::DirectRendering
            | QGL::NoSampleBuffers
            | QGL::NoAlphaChannel
            | QGL::NoAccumBuffer
            | QGL::NoStencilBuffer
            | QGL::NoStereoBuffers
            | QGL::NoOverlay
            | QGL::NoSampleBuffers);
    //fmt.setDepthBufferSize(24);
#if WITH_GLS
    fmt.setSwapInterval(gls_mode == GLS_MODE_ALTERNATING);
    fmt.setStereo(gls_mode == GLS_MODE_QUAD_BUFFER_STEREO);
#else
    fmt.setSwapInterval(0);
    fmt.setStereo(false);
#endif
    return fmt;
}

void XQGLWidget::set_stereo3d_conf(int mode, bool half, bool swap_eyes)
{
#if WITH_GLS
    _gls_mode = static_cast<GLSmode>(mode);
    _gls_half = half;
    _gls_swap_eyes = swap_eyes;
#endif
}

QImage* XQGLWidget::get_current_image()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QImage* img = new QImage(_glwidget->grabFrameBuffer());
    QApplication::restoreOverrideCursor();
    return img;
}

void XQGLWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_F
#if QT_VERSION >= 0x050000
            || event->matches(QKeySequence::FullScreen)
#endif
            ) {
        if (_fullscreen)
            exit_fullscreen();
        else
            enter_fullscreen();
    } else if (event->key() == Qt::Key_Escape) {
        exit_fullscreen();
#if WITH_GLS
    } else if (event->key() == Qt::Key_S) {
        _gls_active = !_gls_active;
#endif
    }
}
