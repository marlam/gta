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

#ifndef GLNAVIGATOR_H
#define GLNAVIGATOR_H

#include "glvm.hpp"


class GLNavigator
{
public:
    GLNavigator()
    {
    }

    virtual ~GLNavigator()
    {
    }

    // Return true if the scene is 2D, false otherwise.
    virtual bool scene_is_2d() const = 0;
    // Set the viewing parameters for a 2D scene.
    virtual void scene_view_2d(
            glvm::vec2& translation_xy, glvm::vec3& scale) const = 0;
    // Set the viewing parameters for a 3D scene.
    virtual void scene_view_3d(glvm::frust& frustum,
            glvm::vec3& viewer_pos, glvm::quat& viewer_rot,
            float& focal_length, float& eye_separation) const = 0;
};

#endif
