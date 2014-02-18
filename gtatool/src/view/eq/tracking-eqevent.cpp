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

#include <cmath>

#include <GL/glew.h>
#include <eq/eq.h>

#include "base/msg.h"
#include "base/tmr.h"

#include "xgl/glvm.hpp"

#include "tracking.hpp"
#include "tracking-eqevent.hpp"

using namespace glvm;


TrackingDriverEQEvent::TrackingDriverEQEvent(
        const vec3 &pos_head,
        const vec3 &pos_flystick)
{
    for (int t = 0; t < 2; t++) {
        _data[t].rot[0] = 0.0f;
        _data[t].rot[1] = 0.0f;
        _data[t].rot[2] = 0.0f;
        _data[t].joy[0] = 0.0f;
        _data[t].joy[1] = 0.0f;
        _data[t].buttons = 0;
    }
    _data[0].pos[0] = pos_head.x;
    _data[0].pos[1] = pos_head.y;
    _data[0].pos[2] = pos_head.z;
    _data[1].pos[0] = pos_flystick.x;
    _data[1].pos[1] = pos_flystick.y;
    _data[1].pos[2] = pos_flystick.z;
}

bool TrackingDriverEQEvent::update()
{
    _timestamp = timer::get(timer::monotonic);
    return true;
}

bool TrackingDriverEQEvent::get(int type, int id,
        int64_t *timestamp,
        vec3 &pos, mat3 &rot,
        float joy[2], unsigned int *buttons)
{
    if (id != 0) {
        return false;
    }

    *timestamp = _timestamp;
    int t = (type == Tracking::BODY ? 0 : 1);
    float rx = _data[t].rot[0];
    float ry = _data[t].rot[1];
    float rz = _data[t].rot[2];
    pos[0] = _data[t].pos[0];
    pos[1] = _data[t].pos[1];
    pos[2] = _data[t].pos[2];
    float sinrx, cosrx, sinry, cosry, sinrz, cosrz;
    ::sincosf(rx, &sinrx, &cosrx);
    ::sincosf(ry, &sinry, &cosry);
    ::sincosf(rz, &sinrz, &cosrz);
    rot[0][0] = cosrz * cosry;
    rot[0][1] = sinrz * cosrx + cosrz * sinry * sinrx;
    rot[0][2] = sinrz * sinrx - cosrz * sinry * cosrx;
    rot[1][0] = - sinrz * cosry;
    rot[1][1] = cosrz * cosrx - sinrz * sinry * sinrx;
    rot[1][2] = cosrz * sinrx + sinrz * sinry * cosrx;
    rot[2][0] = sinry;
    rot[2][1] = - cosry * sinrx;
    rot[2][2] = cosry * cosrx;
    joy[0] = _data[t].joy[0];
    joy[1] = _data[t].joy[1];
    *buttons = _data[t].buttons;

    return true;
}

bool TrackingDriverEQEvent::handle_event(const eq::ConfigEvent* event)
{
    const float pos_delta = 0.05f;
    const float rot_delta = radians(5.0f);

    bool event_handled = false;
    if (event->data.type == eq::Event::KEY_PRESS) {
        event_handled = true;
        switch (event->data.keyPress.key) {
            case 'q':
                _data[0].pos[0] -= pos_delta;
                break;
            case 'w':
                _data[0].pos[0] += pos_delta;
                break;
            case 'a':
                _data[0].pos[1] -= pos_delta;
                break;
            case 's':
                _data[0].pos[1] += pos_delta;
                break;
            case 'z':
                _data[0].pos[2] -= pos_delta;
                break;
            case 'x':
                _data[0].pos[2] += pos_delta;
                break;
            case 'e':
                _data[0].rot[0] -= rot_delta;
                break;
            case 'r':
                _data[0].rot[0] += rot_delta;
                break;
            case 'd':
                _data[0].rot[1] -= rot_delta;
                break;
            case 'f':
                _data[0].rot[1] += rot_delta;
                break;
            case 'c':
                _data[0].rot[2] -= rot_delta;
                break;
            case 'v':
                _data[0].rot[2] += rot_delta;
                break;
            case 't':
                _data[1].pos[0] -= pos_delta;
                break;
            case 'y':
                _data[1].pos[0] += pos_delta;
                break;
            case 'g':
                _data[1].pos[1] -= pos_delta;
                break;
            case 'h':
                _data[1].pos[1] += pos_delta;
                break;
            case 'b':
                _data[1].pos[2] -= pos_delta;
                break;
            case 'n':
                _data[1].pos[2] += pos_delta;
                break;
            case 'u':
                _data[1].rot[0] -= rot_delta;
                break;
            case 'i':
                _data[1].rot[0] += rot_delta;
                break;
            case 'j':
                _data[1].rot[1] -= rot_delta;
                break;
            case 'k':
                _data[1].rot[1] += rot_delta;
                break;
            case 'm':
                _data[1].rot[2] -= rot_delta;
                break;
            case ',':
                _data[1].rot[2] += rot_delta;
                break;
            case eq::KC_F1:
                if (_data[1].buttons & (1 << 0))
                    _data[1].buttons &= !(1 << 0);
                else
                    _data[1].buttons |= (1 << 0);
                break;
            case eq::KC_F2:
                if (_data[1].buttons & (1 << 1))
                    _data[1].buttons &= !(1 << 1);
                else
                    _data[1].buttons |= (1 << 1);
                break;
            case eq::KC_F3:
                if (_data[1].buttons & (1 << 2))
                    _data[1].buttons &= !(1 << 2);
                else
                    _data[1].buttons |= (1 << 2);
                break;
            case eq::KC_F4:
                if (_data[1].buttons & (1 << 3))
                    _data[1].buttons &= !(1 << 3);
                else
                    _data[1].buttons |= (1 << 3);
                break;
            case eq::KC_F5:
                if (_data[1].buttons & (1 << 4))
                    _data[1].buttons &= !(1 << 4);
                else
                    _data[1].buttons |= (1 << 4);
                break;
            default:
                event_handled = false;
                break;
        }
    }
    if (event_handled) {
        msg::dbg("HEAD: %+3.2f %+3.2f %+3.2f  BODY: %+3.2f %+3.2f %+3.2f",
                _data[0].pos[0], _data[0].pos[1], _data[0].pos[2],
                _data[1].pos[0], _data[1].pos[1], _data[1].pos[2]);
    }
    return event_handled;
}
