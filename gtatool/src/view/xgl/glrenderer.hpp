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

#ifndef GLRENDERER_H
#define GLRENDERER_H

#include "base/ser.h"

#include "glcontext.hpp"


class GLRenderer : public serializable
{
private:
    GLContext* _glctx;  // each renderer is always bound to exactly one context!

protected:
    GLEWContext* glewGetContext()
    {
        return _glctx->glewGetContext();
    }

public:
    GLRenderer(GLContext* glctx) : _glctx(glctx)
    {
    }

    /* All following functions do not have access to a GL context. */

    // Called regularly at short intervals, comparable to the GLUT idle
    // function. Can be used for animation etc.
    virtual void update() = 0;

    // Called once per shared GL context to determine if re-rendering is
    // necessary.
    virtual bool needs_rendering() = 0;

    /* All following functions have a valid GL context. */

    // Called only once per shared GL context.
    // Use this to manage objects (textures, buffers, display lists, ...)
    virtual void init_gl_shared() = 0;
    virtual void exit_gl_shared() = 0;

    // Called only once per window GL context.
    // Use this to manage state (glEnable/glDisable etc)
    virtual void init_gl_window() = 0;
    virtual void exit_gl_window() = 0;

    // Called once per shared GL context before rendering.
    virtual void pre_render_shared() = 0;
    // Called once per window GL context before rendering.
    virtual void pre_render_window() = 0;
    // Called potentially multiple times for each window GL context.
    virtual void render() = 0;
    // Called once per window GL context after rendering.
    virtual void post_render_window() = 0;
    // Called once per shared GL context after rendering.
    virtual void post_render_shared() = 0;
};

#endif
