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

#ifndef GLCONTEXT_H
#define GLCONTEXT_H

#include <GL/glew.h>

class GLRenderer;
class GLContext;

class GLRendererFactory
{
public:
    GLRendererFactory() {}
    virtual ~GLRendererFactory() {}
    virtual GLRenderer* create_renderer(GLContext* ctx) = 0;
};

class GLContext
{
private:
    GLEWContext _glewctx;
    bool _glew_initialized;
    GLRendererFactory* _glrenderer_factory;
    GLRenderer* _glrenderer;

public:
    GLContext(GLRendererFactory* glrenderer_factory) :
        _glew_initialized(false),
        _glrenderer_factory(glrenderer_factory), _glrenderer(0)
    {
    }

    GLRenderer* get_renderer()
    {
        if (!_glrenderer)
            _glrenderer = _glrenderer_factory->create_renderer(this);
        return _glrenderer;
    }

    GLEWContext* glewGetContext()
    {
        if (!_glew_initialized) {
            glewContextInit(&_glewctx);
            _glew_initialized = true;
        }
        return &_glewctx;
    }
};

#endif
