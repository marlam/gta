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

#ifndef EQWINDOW_H
#define EQWINDOW_H

#include "xgl/glwindow.hpp"

class eq_node_factory;
class eq_config;

class EQWindow : public GLWindow
{
private:
    bool _valid;                                // Is this object usable?
    eq_node_factory* _eq_node_factory;          // Equalizer node factory
    eq_config* _eq_config;                      // Master Equalizer configuration

public:
    EQWindow(GLRendererFactory* renderer_factory, const GLNavigator* navigator, int tracking, int* argc, char* argv[]);
    ~EQWindow();

    virtual void make_window_current() {}
    virtual void done_window_current() {}
    virtual void make_shared_current() {}
    virtual void done_shared_current() {}
    virtual void swap_buffers() {}
    virtual bool needs_rendering() { return true; }
    virtual void render();

    bool running() const;
};

#endif
