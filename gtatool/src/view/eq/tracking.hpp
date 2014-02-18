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

#ifndef TRACKING_H
#define TRACKING_H

#include <stdint.h>

#include "xgl/glvm.hpp"


class TrackingDriver
{
public:
    TrackingDriver() {}
    virtual ~TrackingDriver() {}

    virtual bool update() = 0;

    virtual bool get(int type, int id,
            int64_t *timestamp,
            glvm::vec3 &pos, glvm::mat3 &rot,
            float joy[2], unsigned int *buttons) = 0;
};

class Tracking
{
private:
    static const int MAX_HANDLES = 16;
    static const int MAX_BUTTONS = 5;
    TrackingDriver *_drv;
    bool _delete_drv;
    int _handles;
    struct data {
        int type;                           // BODY or FLYSTICK
        int id;                             // id
        bool up_to_date;                    // Is info up to date?
        glvm::vec3 pos;                     // Position
        glvm::mat3 rot;                     // Rotation
        float joy[2];                       // Analog Joystick x and y, from [-1,+1]
        int64_t first_delay[MAX_BUTTONS];   // Button auto repeat: first delay
        int64_t delay[MAX_BUTTONS];         // Button auto repeat: subsequent delay
        unsigned int buttons;               // Buttons: state
        unsigned int pressed;               // Buttons: "pressed" event
        unsigned int released;              // Buttons: "released" event
        unsigned int repeating;             // Buttons: a repeated "pressed" event occured
        int64_t timestamp[MAX_BUTTONS];     // Buttons: time of last "pressed" event
    } _data[MAX_HANDLES];

public:
    static const int INVALID = -1;
    enum { BODY, FLYSTICK };

    Tracking(TrackingDriver *drv = NULL);
    ~Tracking();

    /**
     * \param type          Type of tracked target: #BODY or #FLYSTICK
     * \param id            ID of tracked target
     * \return              Handle of the tracked target
     *
     * Activates tracking for the given target. Returns a handle that
     * can be used to retrieve data about the target, or #INVALID
     * on error.
     */
    int track(int type, int id);

    /**
     * \param handle        Handle of a tracked target.
     *
     * Disables tracking for the given target.
     */
    void untrack(int handle);

    /**
     * \param handle        Handle of a tracked target.
     * \return              Whether new data is available.
     *
     * Retrieves new data for the tracked target.
     */
    bool update();

    /**
     * \param handle        Handle of a tracked target.
     * \return              Whether up-to-date information is available for the target.
     */
    bool up_to_date(int handle) const
    {
        return _data[handle].up_to_date;
    }

    /**
     * \param handle        Handle of a tracked target.
     * \return              The 4x4 matrix (rotation and translation) of the target.
     */
    const glvm::mat4 matrix(int handle) const
    {
        glvm::mat4 M = set(glvm::mat4(1.0f), _data[handle].rot);
        translation(M) = _data[handle].pos;
        return M;
    }

    /**
     * \param handle        Handle of a tracked target.
     * \return              The current position of the target.
     */
    const glvm::vec3 &pos(int handle) const
    {
        return _data[handle].pos;
    }

    /**
     * \param handle        Handle of a tracked target.
     * \return              The current rotation of the target.
     */
    const glvm::mat3 &rot(int handle) const
    {
        return _data[handle].rot;
    }

    /**
     * \param handle        Handle of a tracked target.
     * \param button        A button on the tracked target.
     * \param first_delay   Initial time in seconds after which a "pressed" event is repeated.
     * \param delay         Subsequent time in seconds after which a "pressed" event is repeated.
     *
     * Sets the delays for auto repeat. Auto repeat is disabled by
     * default. It can be manually disabled by setting
     * \a first_delay to DBL_MAX.
     */
    void set_auto_repeat(int handle, int button,
            int64_t first_delay = 500000, // 0.5 seconds
            int64_t delay = 33333);       // 1 / 30 seconds

    /**
     * \param handle        Handle of a tracked target.
     * \param button        A button on the tracked target.
     * \return              Whether the button is pressed.
     */
    bool button(int handle, int button) const
    {
        return (_data[handle].buttons & (1 << button));
    }

    /**
     * \param handle        Handle of a tracked target.
     * \param button        A button on the tracked target.
     * \return              Whether a "pressed" event occured.
     */
    bool button_pressed(int handle, int button) const
    {
        return (_data[handle].pressed & (1 << button));
    }

    /**
     * \param handle        Handle of a tracked target.
     * \param button        A button on the tracked target.
     * \return              Whether a "released" event occured.
     */
    bool button_released(int handle, int button) const
    {
        return (_data[handle].released & (1 << button));
    }

    /**
     * \param handle        Handle of a tracked target.
     * \return              The x value of the joystick on the target.
     */
    float joystick_x(int handle) const
    {
        return _data[handle].joy[0];
    }

    /**
     * \param handle        Handle of a tracked target.
     * \return              The y value of the joystick on the target.
     */
    float joystick_y(int handle) const
    {
        return _data[handle].joy[1];
    }
};

#endif
