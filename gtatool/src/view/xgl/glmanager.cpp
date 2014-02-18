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

#include "base/dbg.h"
#include "base/msg.h"
#include "base/tmr.h"

#include "glrenderer.hpp"
#include "glmanager.hpp"


GLManager::GLManager() :
    _ticks_available(0),
    _tick_intervals_index(0),
    _fps(0.0f)
{
}

GLManager::~GLManager()
{
}

void GLManager::tick()
{
    long long now = timer::get(timer::monotonic);
    if (_ticks_available == 0) {
        _last_tick = now;
    }
    _tick_intervals[_tick_intervals_index] = now - _last_tick;
    _last_tick = now;
    _tick_intervals_index++;
    if (_tick_intervals_index >= _ticks)
        _tick_intervals_index = 0;
    if (_ticks_available < _ticks) {
        _ticks_available++;
    } else {
        long long tick_interval_avg = 0;
        for (int i = 0; i < _ticks; i++)
            tick_interval_avg += _tick_intervals[i];
        tick_interval_avg /= _ticks;
        _fps = 1e6f / tick_interval_avg;
    }
}

void GLManager::add_window(GLWindow* wnd)
{
    bool done = false;
    GLContext* ctx = wnd->get_shared_context();
    for (unsigned int i = 0; i < _glcontexts.size(); i++) {
        assert(_glcontexts[i].size() > 0);
        if (_glcontexts[i][0]->get_shared_context() == ctx) {
            _glcontexts[i].push_back(wnd);
            done = true;
            break;
        }
    }
    if (!done) {
        _glcontexts.push_back(std::vector<GLWindow*>());
        _glcontexts[_glcontexts.size() - 1].push_back(wnd);
    }
}

void GLManager::remove_window(GLWindow* wnd)
{
    for (unsigned int i = 0; i < _glcontexts.size(); i++) {
        for (unsigned int j = 0; j < _glcontexts[i].size(); j++) {
            if (_glcontexts[i][j] == wnd)
                _glcontexts[i].erase(_glcontexts[i].begin() + j);
            if (_glcontexts[i].size() == 0)
                _glcontexts.erase(_glcontexts.begin() + i);
            break;
        }
    }
}

void GLManager::init_gl()
{
    for (unsigned int i = 0; i < _glcontexts.size(); i++) {
        assert(_glcontexts[i].size() > 0);
        msg::dbg("init ctx %u...", i);
        _glcontexts[i][0]->make_shared_current();
        _glcontexts[i][0]->get_shared_context()->get_renderer()->init_gl_shared();
        for (unsigned int j = 0; j < _glcontexts[i].size(); j++) {
            _glcontexts[i][j]->make_window_current();
            _glcontexts[i][j]->get_shared_context()->get_renderer()->init_gl_window();
        }
        msg::dbg("init ctx %u done", i);
    }
}

void GLManager::exit_gl()
{
    for (unsigned int i = 0; i < _glcontexts.size(); i++) {
        assert(_glcontexts[i].size() > 0);
        msg::dbg("exit ctx %u...", i);
        for (unsigned int j = 0; j < _glcontexts[i].size(); j++) {
            _glcontexts[i][j]->make_window_current();
            _glcontexts[i][j]->get_shared_context()->get_renderer()->exit_gl_window();
        }
        _glcontexts[i][0]->make_shared_current();
        _glcontexts[i][0]->get_shared_context()->get_renderer()->exit_gl_shared();
        msg::dbg("exit ctx %u done", i);
    }
}

bool GLManager::render()
{
    bool rendered = false;
    for (unsigned int i = 0; i < _glcontexts.size(); i++) {
        assert(_glcontexts[i].size() > 0);
        bool ctx_must_render = _glcontexts[i][0]->get_shared_context()->get_renderer()->needs_rendering();
        if (ctx_must_render)
            msg::dbg("ctx %u triggers rendering", i);
        bool wnd_must_render = ctx_must_render;
        if (!wnd_must_render) {
            for (unsigned int j = 0; j < _glcontexts[i].size(); j++) {
                if (_glcontexts[i][j]->needs_rendering()) {
                    msg::dbg("ctx %u wnd %u triggers rendering", i, j);
                    wnd_must_render = true;
                    break;
                }
            }
        }
        if (wnd_must_render) {
            _glcontexts[i][0]->make_shared_current();
            _glcontexts[i][0]->get_shared_context()->get_renderer()->pre_render_shared();
            for (unsigned int j = 0; j < _glcontexts[i].size(); j++) {
                if (ctx_must_render || _glcontexts[i][j]->needs_rendering()) {
                    msg::dbg("rendering ctx %u wnd %u", i, j);
                    _glcontexts[i][j]->make_window_current();
                    _glcontexts[i][j]->get_shared_context()->get_renderer()->pre_render_window();
                    _glcontexts[i][j]->render();
                    _glcontexts[i][j]->get_shared_context()->get_renderer()->post_render_window();
                    _glcontexts[i][j]->swap_buffers();
                }
            }
            _glcontexts[i][0]->make_shared_current();
            _glcontexts[i][0]->get_shared_context()->get_renderer()->post_render_shared();
            rendered = true;
        }
    }
    if (rendered)
        tick();
    return rendered;
}

void GLManager::update()
{
    for (unsigned int i = 0; i < _glcontexts.size(); i++) {
        assert(_glcontexts[i].size() > 0);
        _glcontexts[i][0]->get_shared_context()->get_renderer()->update();
    }
}

std::vector<GLRenderer*> GLManager::get_renderers()
{
    std::vector<GLRenderer*> r;
    for (unsigned int i = 0; i < _glcontexts.size(); i++) {
        assert(_glcontexts[i].size() > 0);
        r.push_back(_glcontexts[i][0]->get_shared_context()->get_renderer());
    }
    return r;
}
