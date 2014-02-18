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

#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glew.h>

#include <gta/gta.hpp>

#include "minmaxhist.hpp"
#include "viewparams.hpp"

#include "xgl/glcontext.hpp"
#include "xgl/glrenderer.hpp"


class Renderer : public GLRenderer
{
private:
    // Current GTA
    gta::header _gta_hdr;
    void* _gta_data;
    bool _gta_data_owner;
    bool _gta_dirty;
    bool _gta_reupload;
    // GTA data properties
    MinMaxHist* _minmaxhist;
    bool _minmaxhist_owner;
    // View parameters
    ViewParameters _view_params;
    // General OpenGL objects and render state
    std::vector<GLuint> _texs;
    long long _last_update;
    bool _need_rendering;
    bool _view_params_changed;
    // Mode-specific OpenGL objects and render state: mode_2d
    struct {
        GLuint prg;
        GLuint gradient_tex;
    } _mode_2d;

public:
    Renderer(GLContext* glctx);
    ~Renderer();

    void set_gta(const gta::header& hdr, const void* data, const MinMaxHist* minmaxhist);
    void set_view_params(const ViewParameters& view_params);

    virtual void save(std::ostream& os) const;
    virtual void load(std::istream& is);

    virtual void update();

    virtual void init_gl_shared();
    virtual void exit_gl_shared();
    virtual void init_gl_window();
    virtual void exit_gl_window();
    virtual bool needs_rendering();
    virtual void pre_render_shared();
    virtual void pre_render_window();
    virtual void render();
    virtual void post_render_window();
    virtual void post_render_shared();

private:
    /* OpenGL helper functions */

    // Compile a shader and print warnings/errors if necessary.
    // The shader source is 'src', and its 'type' is GL_VERTEX_SHADER,
    // GL_FRAGMENT_SHADER or similar.
    // The shader 'name' is optional and only used in warning or error messages.
    // Returns a shader handle on success, and 0 on failure.
    GLuint compile_shader(GLenum type, const std::string& src,
            const std::string& name = std::string());

    // Link the shader program 'prg' and print warnings/errors if necessary.
    // The program 'name' is optional and only used in warning or error messages.
    // Returns 'prg' on success, and 0 on error.
    GLuint link_program(GLuint prg, const std::string& name = std::string());

    // Delete the shader program 'prg' and all associated shaders.
    void delete_program(GLuint prg);

    // Functions that check for errors and FBO status problems.
    // These functions are intended to be used inside assert(), like this:
    // assert(check_error(HERE));
    // In debug build, such checks give nice messages with file name and line
    // number, and in release builds they are completely removed.
    bool check_error(const std::string& where = std::string());
    bool check_fbo(const std::string& where = std::string());
};

class RendererFactory : public GLRendererFactory
{
public:
    virtual GLRenderer* create_renderer(GLContext* ctx)
    {
        return new Renderer(ctx);
    }
};

#endif
