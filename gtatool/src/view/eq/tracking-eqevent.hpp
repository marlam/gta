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

#ifndef TRACKING_EQEVENT_H
#define TRACKING_EQEVENT_H

#include <eq/eq.h>

#include "xgl/glvm.hpp"

#include "tracking.hpp"


class TrackingDriverEQEvent : public TrackingDriver
{
private:
    int64_t _timestamp;
    struct Data
    {
        float pos[3];
        float rot[3];
        float joy[2];
        unsigned int buttons;
    } _data[2];

public:
    TrackingDriverEQEvent(
            const glvm::vec3 &pos_head = glvm::vec3(0.0f),
            const glvm::vec3 &pos_flystick = glvm::vec3(0.0f));

    bool update();

    bool get(int type, int id,
            int64_t *timestamp,
            glvm::vec3 &pos, glvm::mat3 &rot,
            float joy[2], unsigned int buttons[1]);

    bool handle_event(const eq::ConfigEvent* event);
};

#endif
