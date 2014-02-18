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

#include <cstdlib>
#include <cmath>
#include <limits>

#include "renderer.hpp"

#include "base/dbg.h"
#include "base/msg.h"
#include "base/tmr.h"

#include "mode_2d_fs.glsl.h"


Renderer::Renderer(GLContext* ctx) :
    GLRenderer(ctx),
    _gta_data(NULL),
    _gta_data_owner(false),
    _gta_dirty(false),
    _gta_reupload(false),
    _minmaxhist(NULL),
    _minmaxhist_owner(false),
    _view_params(),
    _last_update(0),
    _need_rendering(true),
    _view_params_changed(false)
{
}

Renderer::~Renderer()
{
    if (_gta_data_owner)
        std::free(_gta_data);
    if (_minmaxhist_owner)
        delete _minmaxhist;
}

void Renderer::set_gta(const gta::header& hdr, const void* data, const MinMaxHist* minmaxhist)
{
    _gta_hdr = hdr;
    _gta_data = const_cast<void*>(data);
    _gta_dirty = true;
    _gta_reupload = true;
    _minmaxhist = const_cast<MinMaxHist*>(minmaxhist);
    _need_rendering = true;
}

void Renderer::set_view_params(const ViewParameters& view_params)
{
    _view_params = view_params;
    _view_params_changed = true;
    _need_rendering = true;
}

void Renderer::save(std::ostream& os) const
{
    s11n::save(os, _gta_dirty);
    if (_gta_dirty) {
        _gta_hdr.write_to(os);
        s11n::save(os, _gta_data, _gta_hdr.data_size());
        const_cast<Renderer*>(this)->_gta_dirty = false;
    }
    bool have_minmaxhist = _minmaxhist;
    s11n::save(os, have_minmaxhist);
    if (have_minmaxhist)
        s11n::save(os, *_minmaxhist);
    s11n::save(os, _view_params);
    s11n::save(os, _last_update);
    s11n::save(os, _view_params_changed);
    s11n::save(os, _need_rendering);
}

void Renderer::load(std::istream& is)
{
    s11n::load(is, _gta_dirty);
    if (_gta_dirty) {
        _gta_hdr.read_from(is);
        std::free(_gta_data);
        _gta_data = std::malloc(_gta_hdr.data_size());
        s11n::load(is, _gta_data, _gta_hdr.data_size());
        _gta_data_owner = true;
        _gta_reupload = true;
    }
    bool have_minmaxhist;
    s11n::load(is, have_minmaxhist);
    if (have_minmaxhist) {
        if (!_minmaxhist) {
            _minmaxhist = new MinMaxHist;
            _minmaxhist_owner = true;
        }
        s11n::load(is, *_minmaxhist);
    }
    s11n::load(is, _view_params);
    s11n::load(is, _last_update);
    s11n::load(is, _view_params_changed);
    s11n::load(is, _need_rendering);
}

void Renderer::update()
{
    long long now = timer::get(timer::monotonic);
    if (now - _last_update > 1000000) {
#if 0
        _rot_angle += 15.0f;
        if (_rot_angle >= 360.0f)
            _rot_angle = 0.0f;
#endif
        _last_update = now;
        //_need_rendering = true;
    }
}

void Renderer::init_gl_shared()
{
    _mode_2d.prg = 0;
    _mode_2d.gradient_tex = 0;
}

void Renderer::exit_gl_shared()
{
    if (!_view_params.mode_is_valid())
        return;

    if (_view_params.mode == ViewParameters::mode_2d) {
        delete_program(_mode_2d.prg);
        glDeleteTextures(1, &_mode_2d.gradient_tex);
    }

    if (_texs.size() > 0)
        glDeleteTextures(_texs.size(), &_texs[0]);
}

void Renderer::init_gl_window()
{
    if (!glewIsSupported("GL_VERSION_2_1")) {
        msg::err("Basic OpenGL features are missing!");
        std::exit(1);
    }
    glEnable(GL_DEPTH_TEST);
    check_error(HERE);
}

void Renderer::exit_gl_window()
{
#if 0
    if (!_view_params || !_view_params.mode_is_valid())
        return;

    if (_view_params._mode == ViewParameters::mode_2d) {
        TODO;
    }
#endif
    check_error(HERE);
}

bool Renderer::needs_rendering()
{
    return _need_rendering;
}

void Renderer::pre_render_shared()
{
    if (!_view_params.mode_is_valid())
        return;

    if (_gta_reupload) {
        msg::dbg("Uploading GTA...");
        // Backup GL state
        GLint tex_bak, pub_bak, ua_bak;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex_bak);
        glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &pub_bak);
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &ua_bak);
        GLuint pbo;
        glGenBuffers(1, &pbo);
        assert(check_error(HERE));
        // Create textures
        if (_texs.size() > 0)
            glDeleteTextures(_texs.size(), &_texs[0]);
        _texs.resize(_gta_hdr.components());
        glGenTextures(_texs.size(), &_texs[0]);
        assert(check_error(HERE));
        for (uintmax_t c = 0; c < _gta_hdr.components(); c++) {
            msg::dbg(4, "component %d", static_cast<int>(c));
            gta::type t = _gta_hdr.component_type(c);
            // Setup texture for this component
            glBindTexture(GL_TEXTURE_2D, _texs[c]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            assert(check_error(HERE));
            GLint internal_format = 0;
            size_t internal_element_size = 0;
            GLint format = 0;
            GLint type = 0;
            switch (t) {
            case gta::int8:
                internal_format = GL_R8_SNORM; // loss of value -128 on denormalization!
                internal_element_size = sizeof(signed char);
                format = GL_RED;
                type = GL_BYTE;
                break;
            case gta::uint8:
                internal_format = GL_R8;
                internal_element_size = sizeof(unsigned char);
                format = GL_RED;
                type = GL_UNSIGNED_BYTE;
                break;
            case gta::int16:
                internal_format = GL_R16_SNORM; // loss of value -32768 on denormalization!
                internal_element_size = sizeof(short);
                format = GL_RED;
                type = GL_SHORT;
                break;
            case gta::uint16:
                internal_format = GL_R16;
                internal_element_size = sizeof(unsigned short);
                format = GL_RED;
                type = GL_UNSIGNED_SHORT;
                break;
            case gta::int32:
                internal_format = GL_R32F; // precision loss!
                internal_element_size = sizeof(int);
                format = GL_RED;
                type = GL_INT;
                break;
            case gta::uint32:
                internal_format = GL_R32F; // precision loss!
                internal_element_size = sizeof(unsigned int);
                format = GL_RED;
                type = GL_UNSIGNED_INT;
                break;
            case gta::int64:
                internal_format = GL_R32F; // precision loss!
                internal_element_size = sizeof(float);
                format = GL_RED;
                type = GL_FLOAT;
                break;
            case gta::uint64:
                internal_format = GL_R32F; // precision loss!
                internal_element_size = sizeof(float);
                format = GL_RED;
                type = GL_FLOAT;
                break;
            case gta::float32:
                internal_format = GL_R32F; // may lose sepcial values, e.g. NaN
                internal_element_size = sizeof(float);
                format = GL_RED;
                type = GL_FLOAT;
                break;
            case gta::float64:
                internal_format = GL_R32F; // precision loss!
                internal_element_size = sizeof(float);
                format = GL_RED;
                type = GL_FLOAT;
                break;
            case gta::cfloat32:
                internal_format = GL_RG32F; // may lose sepcial values, e.g. NaN
                internal_element_size = 2 * sizeof(float);
                format = GL_RG;
                type = GL_FLOAT;
                break;
            case gta::cfloat64:
                internal_format = GL_RG32F; // precision loss!
                internal_element_size = 2 * sizeof(float);
                format = GL_RG;
                type = GL_FLOAT;
                break;
            default:
                // cannot happen
                assert(false);
                break;
            }
            size_t w = _gta_hdr.dimension_size(0);
            size_t h = (_gta_hdr.dimensions() < 2 ? 1 : _gta_hdr.dimension_size(1));
            // Upload the data
            const unsigned char* gta_data = static_cast<unsigned char*>(_gta_data);
            size_t gta_element_size = _gta_hdr.element_size();
            size_t gta_component_offset = static_cast<const unsigned char*>(
                    _gta_hdr.component(static_cast<const void*>(0), c))
                - static_cast<unsigned char*>(0);
            size_t component_line_size = w * internal_element_size;
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
            glBufferData(GL_PIXEL_UNPACK_BUFFER, component_line_size * h, NULL, GL_STREAM_DRAW);
            unsigned char* component_data = static_cast<unsigned char*>(
                    glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
            assert(check_error(HERE));
            assert(component_data);
            assert(reinterpret_cast<uintptr_t>(component_data) % 4 == 0);
            if (t == gta::int64) {
                for (size_t i = 0; i < w * h; i++) {
                    int64_t v;
                    float vf;
                    std::memcpy(&v, &gta_data[i * gta_element_size + gta_component_offset], sizeof(int64_t));
                    vf = v;
                    std::memcpy(&component_data[i * internal_element_size], &vf, internal_element_size);
                }
            } else if (t == gta::uint64) {
                for (size_t i = 0; i < w * h; i++) {
                    uint64_t v;
                    float vf;
                    std::memcpy(&v, &gta_data[i * gta_element_size + gta_component_offset], sizeof(uint64_t));
                    vf = v;
                    std::memcpy(&component_data[i * internal_element_size], &vf, internal_element_size);
                }
            } else if (t == gta::float64) {
                for (size_t i = 0; i < w * h; i++) {
                    double v;
                    float vf;
                    std::memcpy(&v, &gta_data[i * gta_element_size + gta_component_offset], sizeof(double));
                    vf = v;
                    std::memcpy(&component_data[i * internal_element_size], &vf, internal_element_size);
                }
            } else if (t == gta::cfloat64) {
                for (size_t i = 0; i < w * h; i++) {
                    double v[2];
                    float vf[2];
                    std::memcpy(v, &gta_data[i * gta_element_size + gta_component_offset], 2 * sizeof(double));
                    vf[0] = v[0];
                    vf[1] = v[1];
                    std::memcpy(&component_data[i * internal_element_size], vf, internal_element_size);
                }
            } else {
                for (size_t i = 0; i < w * h; i++) {
                    std::memcpy(&component_data[i * internal_element_size],
                            &gta_data[i * gta_element_size + gta_component_offset],
                            internal_element_size);
                }
            }
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
            assert(check_error(HERE));
            glPixelStorei(GL_UNPACK_ALIGNMENT,
                      component_line_size % 4 == 0 ? 4
                    : component_line_size % 2 == 0 ? 2
                    : 1);
            assert(check_error(HERE));
            glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0, format, type, NULL);
            assert(check_error(HERE));
        }
        // Restore GL state
        glDeleteBuffers(1, &pbo);
        glBindTexture(GL_TEXTURE_2D, tex_bak);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pub_bak);
        glPixelStorei(GL_UNPACK_ALIGNMENT, ua_bak);
        _gta_reupload = false;
        assert(check_error(HERE));
        msg::dbg("... done");
    }

    // Initialize and update OpenGL objects for the given modes.
    // This includes changes due to changed viewing parameters according to _view_params_changed.
    if (_view_params.mode == ViewParameters::mode_2d) {
        if (_mode_2d.prg == 0) {
            GLuint fs = compile_shader(GL_FRAGMENT_SHADER, MODE_2D_FS_GLSL_STR, "mode_2d");
            assert(fs);
            _mode_2d.prg = glCreateProgram();
            glAttachShader(_mode_2d.prg, fs);
            _mode_2d.prg = link_program(_mode_2d.prg);
            assert(_mode_2d.prg);
        }
        if (_mode_2d.gradient_tex == 0) {
            glGenTextures(1, &_mode_2d.gradient_tex);
            glBindTexture(GL_TEXTURE_2D, _mode_2d.gradient_tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            assert(check_error(HERE));
        }
        if (_view_params_changed) {
            int c = _view_params.mode_2d_global.component;
            const ViewParameters::mode_2d_component_t& mode_2d_component = _view_params.mode_2d_components[c];
            if (mode_2d_component.gradient && mode_2d_component.gradient_length > 0) {
                glBindTexture(GL_TEXTURE_2D, _mode_2d.gradient_tex);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mode_2d_component.gradient_length, 1, 0,
                        GL_RGB, GL_UNSIGNED_BYTE, mode_2d_component.gradient_colors);
            }
        }
    }

    _view_params_changed = false;
    assert(check_error(HERE));
}

void Renderer::pre_render_window()
{
#if 0
    if (!_view_params || !_view_params.mode_is_valid())
        return;

    if (_view_params._mode == ViewParameters::mode_2d) {
        TODO;
    }
#endif
    assert(check_error(HERE));
}

void Renderer::render()
{
    assert(check_error(HERE));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!_view_params.mode_is_valid()) {
        _need_rendering = false;
        return;
    }

    if (_view_params.mode == ViewParameters::mode_2d) {
        assert(check_error(HERE));
        assert(glIsProgram(_mode_2d.prg));
        glUseProgram(_mode_2d.prg);
        assert(check_error(HERE));
        const ViewParameters::mode_2d_global_t& mode_2d_global = _view_params.mode_2d_global;
        int component = mode_2d_global.component;
        assert(component >= 0 && component <= static_cast<int>(_gta_hdr.components()));
        // Set up input data
        if (component < static_cast<int>(_gta_hdr.components())) { // single component mode (no color)
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, _texs[component]);
            GLint components[3] = { 0, 0, 0 };
            glUniform1iv(glGetUniformLocation(_mode_2d.prg, "components"), 3, components);
            glUniform1i(glGetUniformLocation(_mode_2d.prg, "is_complex"),
                    _gta_hdr.component_type(component) == gta::cfloat32
                    || _gta_hdr.component_type(component) == gta::cfloat64);
            glUniform1i(glGetUniformLocation(_mode_2d.prg, "colorspace"),
                    static_cast<int>(ViewParameters::colorspace_null));
            glUniform1f(glGetUniformLocation(_mode_2d.prg, "denorm_factor"),
                    _gta_hdr.component_type(component) == gta::int8 ? 127.0f
                    : _gta_hdr.component_type(component) == gta::uint8 ? 255.0f
                    : _gta_hdr.component_type(component) == gta::int16 ? 32767.0f
                    : _gta_hdr.component_type(component) == gta::uint16 ? 65535.0f
                    : _gta_hdr.component_type(component) == gta::int32 ? 32767.0f
                    : _gta_hdr.component_type(component) == gta::uint32 ? 65535.0f
                    : 1.0f);
        } else { // color mode (needs three components)
            for (int i = 0; i < 3; i++) {
                assert(mode_2d_global.color_components[i] >= 0);
                assert(mode_2d_global.color_components[i] < static_cast<int>(_gta_hdr.components()));
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, _texs[mode_2d_global.color_components[i]]);
            }
            GLint components[3] = { 0, 1, 2 };
            glUniform1iv(glGetUniformLocation(_mode_2d.prg, "components"), 3, components);
            glUniform1i(glGetUniformLocation(_mode_2d.prg, "is_complex"), 0);
            glUniform1i(glGetUniformLocation(_mode_2d.prg, "colorspace"),
                    static_cast<int>(mode_2d_global.colorspace));
            glUniform1f(glGetUniformLocation(_mode_2d.prg, "denorm_factor"), 1.0f);
        }
        assert(check_error(HERE));
        // Set up processing parameters
        const ViewParameters::mode_2d_component_t& mode_2d_component = _view_params.mode_2d_components[component];
        glUniform1f(glGetUniformLocation(_mode_2d.prg, "minval"), mode_2d_component.range_min);
        glUniform1f(glGetUniformLocation(_mode_2d.prg, "maxval"), mode_2d_component.range_max);
        bool do_gamma = (mode_2d_component.gamma
                && (mode_2d_component.gamma_value < 1.0f || mode_2d_component.gamma_value > 1.0f));
        glUniform1i(glGetUniformLocation(_mode_2d.prg, "do_gamma"), do_gamma ? 1 : 0);
        glUniform1f(glGetUniformLocation(_mode_2d.prg, "gamma"), mode_2d_component.gamma_value);
        bool do_urq = (mode_2d_component.urq && mode_2d_component.urq_value > 1.0f);
        glUniform1i(glGetUniformLocation(_mode_2d.prg, "do_urq"), do_urq ? 1 : 0);
        glUniform1f(glGetUniformLocation(_mode_2d.prg, "urq"), mode_2d_component.urq_value);
        glUniform1i(glGetUniformLocation(_mode_2d.prg, "do_jetcolor"),
                (mode_2d_component.jetcolor && !mode_2d_component.gradient) ? 1 : 0);
        glUniform1i(glGetUniformLocation(_mode_2d.prg, "jetcolor_cyclic"), mode_2d_component.jetcolor_cyclic);
        assert(check_error(HERE));
        glUniform1i(glGetUniformLocation(_mode_2d.prg, "do_gradient"), mode_2d_component.gradient);
        glUniform1i(glGetUniformLocation(_mode_2d.prg, "gradient_tex"), 3);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, _mode_2d.gradient_tex);
        glUniform1i(glGetUniformLocation(_mode_2d.prg, "coloring_inverse"), mode_2d_component.coloring_inverse);
        glUniform1f(glGetUniformLocation(_mode_2d.prg, "coloring_start"), mode_2d_component.coloring_start);
        glUniform1f(glGetUniformLocation(_mode_2d.prg, "coloring_lightvar"), mode_2d_component.coloring_lightvar);
        // Determine quad dimensions
        float ql = -1.0f, qr = +1.0f, qb = -1.0f, qt = +1.0f;
        float quad_aspect = mode_2d_global.array_aspect * mode_2d_global.sample_aspect;
        if (quad_aspect > 1.0f) {
            qb = -1.0f / quad_aspect;
            qt = +1.0f / quad_aspect;
        } else if (quad_aspect < 1.0f) {
            ql = -quad_aspect;
            qr = +quad_aspect;
        }
        // Draw quad
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(ql, qb);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(qr, qb);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(qr, qt);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(ql, qt);
        glEnd();
        assert(check_error(HERE));
    }

    _need_rendering = false;
}

void Renderer::post_render_window()
{
#if 0
    if (!_view_params || !_view_params.mode_is_valid())
        return;

    if (_view_params._mode == ViewParameters::mode_2d) {
        TODO;
    }
#endif
    assert(check_error(HERE));
}

void Renderer::post_render_shared()
{
#if 0
    if (!_view_params || !_view_params.mode_is_valid())
        return;

    if (_view_params._mode == ViewParameters::mode_2d) {
        TODO;
    }
#endif
    assert(check_error(HERE));
}

/* OpenGL helper functions */

static void kill_crlf(char* str)
{
    size_t l = std::strlen(str);
    if (l > 0 && str[l - 1] == '\n')
        str[--l] = '\0';
    if (l > 0 && str[l - 1] == '\r')
        str[l - 1] = '\0';
}

GLuint Renderer::compile_shader(GLenum type, const std::string& src, const std::string& name)
{
    GLuint shader = glCreateShader(type);
    const GLchar* glsrc = src.c_str();
    glShaderSource(shader, 1, &glsrc, NULL);
    glCompileShader(shader);

    std::string log;
    GLint e, l;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &e);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &l);
    if (l > 0) {
        char* tmplog = new char[l];
        glGetShaderInfoLog(shader, l, NULL, tmplog);
        kill_crlf(tmplog);
        log = std::string(tmplog);
        delete[] tmplog;
    } else {
        log = std::string("");
    }

    if (e && log.length() > 0) {
        msg::wrn("OpenGL %s '%s': compiler warning:",
                type == GL_VERTEX_SHADER ? "vertex shader"
                : type == GL_GEOMETRY_SHADER ? "geometry shader"
                : type == GL_TESS_EVALUATION_SHADER ? "tess. eval. shader"
                : type == GL_TESS_CONTROL_SHADER ? "tess. control shader"
                : "fragment shader",
                name.c_str());
        msg::wrn_txt("%s", log.c_str());
    } else if (e != GL_TRUE) {
        msg::err("OpenGL %s '%s': compiler error:",
                type == GL_VERTEX_SHADER ? "vertex shader"
                : type == GL_GEOMETRY_SHADER ? "geometry shader"
                : type == GL_TESS_EVALUATION_SHADER ? "tess. eval. shader"
                : type == GL_TESS_CONTROL_SHADER ? "tess. control shader"
                : "fragment shader",
                name.c_str());
        msg::err_txt("%s", log.c_str());
        shader = 0;
    }
    return shader;
}

GLuint Renderer::link_program(GLuint prg, const std::string& name)
{
    glLinkProgram(prg);

    std::string log;
    GLint e, l;
    glGetProgramiv(prg, GL_LINK_STATUS, &e);
    glGetProgramiv(prg, GL_INFO_LOG_LENGTH, &l);
    if (l > 0) {
        char *tmplog = new char[l];
        glGetProgramInfoLog(prg, l, NULL, tmplog);
        kill_crlf(tmplog);
        log = std::string(tmplog);
        delete[] tmplog;
    } else {
        log = std::string("");
    }

    if (e && log.length() > 0) {
        msg::wrn("OpenGL program '%s': linker warning:", name.c_str());
        msg::wrn_txt("%s", log.c_str());
    } else if (e != GL_TRUE) {
        msg::err("OpenGL program '%s': linker error:", name.c_str());
        msg::err_txt("%s", log.c_str());
        delete_program(prg);
        prg = 0;
    }
    return prg;
}

void Renderer::delete_program(GLuint prg)
{
    if (glIsProgram(prg)) {
        GLint shader_count;
        glGetProgramiv(prg, GL_ATTACHED_SHADERS, &shader_count);
        GLuint* shaders = new GLuint[shader_count];
        glGetAttachedShaders(prg, shader_count, NULL, shaders);
        for (int i = 0; i < shader_count; i++)
            glDeleteShader(shaders[i]);
        delete[] shaders;
        glDeleteProgram(prg);
    }
}

bool Renderer::check_fbo(const std::string& where)
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::string pfx = (where.length() > 0 ? where + ": " : "");
        msg::err("%sOpenGL FBO error 0x%04X", pfx.c_str(), status);
        return false;
    }
    return true;
}

bool Renderer::check_error(const std::string& where)
{
    GLenum e = glGetError();
    if (e != GL_NO_ERROR) {
        std::string pfx = (where.length() > 0 ? where + ": " : "");
        msg::err("%sOpenGL error 0x%04X", pfx.c_str(), e);
        return false;
    }
    return true;
}
