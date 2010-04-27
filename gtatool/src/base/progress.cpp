/*
 * progress.cpp
 *
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2009, 2010  Martin Lambers <marlam@marlam.de>
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
#include <cmath>

#include "tools.h"
#include "progress.h"


progress::progress() throw ()
{
}

progress::~progress() throw ()
{
}

void progress::start(unsigned long long events, unsigned long long eta_events) throw ()
{
    _start.set(Timer::realtime);
    _events = events;
    _max_eta_samples = eta_events;
    _current_event = 0;
}

void progress::set(unsigned long long current_event) throw ()
{
    _current_event = current_event;
    if (_max_eta_samples > 0)
    {
        Timer now(Timer::realtime);
        if (_eta_samples.size() == _max_eta_samples)
        {
            _eta_samples.pop();
        }
        _eta_samples.push(std::pair<unsigned long long, Timer>(_current_event, now));
    }
}

progress& progress::operator++() throw ()
{
    set(_current_event + 1);
    return *this;
}

void progress::operator++(int) throw ()
{
    operator++();
}

unsigned long long progress::events() const throw ()
{
    return _events;
}

unsigned long long progress::current_event() const throw ()
{
    return _current_event;
}

double progress::get() const throw ()
{
    double p = static_cast<double>(_current_event) / static_cast<double>(_events);
    if (!std::isfinite(p))
    {
        p = 0.0;
    }
    p = tools::clamp(p, 0.0, 1.0);
    return p;
}

Timer progress::eta() const throw ()
{
    Timer now(Timer::realtime);
    double seconds_per_event;
    unsigned long long missing_events;
    double remaining_seconds;
    Timer end;

    if (_eta_samples.size() < 2)
    {
        if (_current_event == 0)
        {
            seconds_per_event = std::numeric_limits<double>::max();
        }
        else
        {
            seconds_per_event = (now - _start).seconds() / static_cast<double>(_current_event);
        }
    }
    else
    {
        unsigned long long eta_sample_event = _eta_samples.front().first;
        Timer eta_sample_time = _eta_samples.front().second;
        seconds_per_event = (now - eta_sample_time).seconds() / static_cast<double>(_current_event - eta_sample_event);
    }
    missing_events = _events - _current_event;
    remaining_seconds = static_cast<double>(missing_events) * seconds_per_event;
    end = now;
    end += Timer(remaining_seconds);
    return end;
}
