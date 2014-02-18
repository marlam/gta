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

#ifndef GLWINDOW_H
#define GLWINDOW_H

#include "glcontext.hpp"
#include "glnavigator.hpp"


class GLWindow
{
private:
    GLContext* _shared_glctx;
    const GLNavigator* _navigator;

public:
    GLWindow(const GLNavigator* navigator) : _shared_glctx(0), _navigator(navigator)
    {
    }

    virtual ~GLWindow()
    {
    }

    GLEWContext* glewGetContext()
    {
        return _shared_glctx->glewGetContext();
    }

    void set_shared_context(GLContext* ctx)
    {
        _shared_glctx = ctx;
    }

    GLContext* get_shared_context()
    {
        return _shared_glctx;
    }

    const GLNavigator* get_navigator() const
    {
        return _navigator;
    }

    virtual void make_window_current() = 0;     // bind the context for this window
    virtual void done_window_current() = 0;     // unbind the context for this window
    virtual void make_shared_current() = 0;     // bind the shared context for this window
    virtual void done_shared_current() = 0;     // unbind the shared context for this window

    virtual void swap_buffers() = 0;            // swap buffers for this window

    virtual bool needs_rendering() = 0;         // whether this window requires a redraw

    // This function must setup up viewport, framebuffer, and projection
    // and modelview matrix, and optionally other things such as stencil buffers.
    // It should use the information provided by the navigator (if not NULL) to do this.
    virtual void render() = 0;
};

#endif
