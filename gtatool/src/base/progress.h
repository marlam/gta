/*
 * progress.h
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

#ifndef PROGRESS_H
#define PROGRESS_H

#include <map>
#include <queue>

#include "timer.h"


class progress
{
private:
    Timer _start;
    unsigned long long _events;
    unsigned long long _max_eta_samples;
    unsigned long long _current_event;
    std::queue<std::pair<unsigned long long, Timer> > _eta_samples;

public:
    progress() throw ();
    ~progress() throw ();

    void start(unsigned long long events, unsigned long long eta_events = 0) throw ();
    void set(unsigned long long current_event) throw ();
    progress& operator++() throw ();    // prefix
    void operator++(int) throw();       // postfix

    unsigned long long events() const throw ();
    unsigned long long current_event() const throw ();
    double get() const throw ();
    Timer eta() const throw ();
};

#endif
