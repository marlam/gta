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

#ifndef GLMANAGER_H
#define GLMANAGER_H

#include <vector>
#include <stdexcept>

#include "glwindow.hpp"

class GLManager
{
private:
    // For each shared GLContext a vector of GLWindows that share this context
    std::vector<std::vector<GLWindow*> > _glcontexts;
    // FPS measurement
    static const int _ticks = 8;
    int _ticks_available;
    long long _tick_intervals[_ticks];
    int _tick_intervals_index;
    long long _last_tick;
    float _fps;

    // Helper functions
    void tick();

public:
    GLManager();
    virtual ~GLManager();

    void add_window(GLWindow* wnd);
    void remove_window(GLWindow* wnd);

    float fps() const
    {
        return _fps;
    }

    // Use this to initialize and deinitialize GL.
    void init_gl();
    void exit_gl();

    // Use this to render. This function returns true if some rendering was done
    // (might not be the case if no window or context needs rendering).
    bool render();

    // Update all renderers (give them the opportunity for animation etc)
    void update();

    // Get all your renderers (to manipulate them directly)
    std::vector<GLRenderer*> get_renderers();
};

#endif
