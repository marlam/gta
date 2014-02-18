/*
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2014
 * Computer Graphics Group, University of Siegen, Germany.
 * Written by Martin Lambers <martin.lambers@uni-siegen.de>.
 * See http://www.cg.informatik.uni-siegen.de/ for contact information.
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

#include <limits>
#include <cstring>

#include "base/msg.h"

#include "xgl/glvm.hpp"

#include "tracking.hpp"
#include "tracking-eqevent.hpp"

using namespace glvm;


Tracking::Tracking(TrackingDriver *drv)
{
    _drv = drv ? drv : new TrackingDriverEQEvent();
    _delete_drv = !drv;
    _handles = 0;
}

Tracking::~Tracking()
{
    if (_delete_drv)
        delete _drv;
}

int Tracking::track(int type, int id)
{
    if (_handles == MAX_HANDLES)
        return INVALID;

    int h = _handles;
    _data[h].type = type;
    _data[h].id = id;
    _data[h].up_to_date = false;
    _data[h].pos = vec3(0.0f);
    _data[h].rot = mat3(1.0f);
    _data[h].joy[0] = 0.0f;
    _data[h].joy[1] = 0.0f;
    for (int b = 0; b < MAX_BUTTONS; b++) {
        _data[h].first_delay[b] = std::numeric_limits<int64_t>::max();
        _data[h].delay[b] = std::numeric_limits<int64_t>::max();
    }
    _data[h].buttons = 0;
    _data[h].pressed = 0;
    _data[h].released = 0;
    _data[h].repeating = 0;
    _handles++;
    return h;
}

void Tracking::untrack(int handle)
{
    memmove(&(_data[handle]), &(_data[handle + 1]), (_handles - handle - 1) * sizeof(struct data));
    _handles--;
}

void Tracking::set_auto_repeat(int handle, int button, int64_t first_delay, int64_t delay)
{
    _data[handle].first_delay[button] = first_delay;
    _data[handle].delay[button] = delay;
}

bool Tracking::update()
{
    if (!_drv->update()) {
        return false;
    }

    bool have_info = false;
    for (int h = 0; h < _handles; h++) {
        int64_t timestamp;
        unsigned int old_buttons = _data[h].buttons;

        _data[h].up_to_date = false;
        if (!_drv->get(_data[h].type, _data[h].id, &timestamp,
                    _data[h].pos, _data[h].rot, _data[h].joy, &_data[h].buttons)) {
            msg::wrn("Tracking: No up-to-date information available for target %d", h);
            continue;
        }
        _data[h].up_to_date = true;
        have_info = true;

        // Button events
        _data[h].pressed = 0;
        _data[h].released = 0;
        for (int b = 0; b < MAX_BUTTONS; b++) {
            unsigned int button_flag = 1 << b;
            if (_data[h].buttons & button_flag) {
                if (!(old_buttons & button_flag)) {
                    _data[h].pressed |= button_flag;
                    _data[h].timestamp[b] = timestamp;
                    _data[h].repeating &= !button_flag;
                } else if (!(_data[h].repeating & button_flag)
                        && timestamp - _data[h].timestamp[b] > _data[h].first_delay[b]) {
                    _data[h].pressed |= button_flag;
                    _data[h].timestamp[b] = timestamp;
                    _data[h].repeating |= button_flag;
                } else if (_data[h].repeating & button_flag
                        && timestamp - _data[h].timestamp[b] > _data[h].delay[b]) {
                    _data[h].pressed |= button_flag;
                    _data[h].timestamp[b] = timestamp;
                }
            } else {
                if (old_buttons & button_flag) {
                    _data[h].released |= button_flag;
                }
            }
        }
    }

    return have_info;
}
