/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2013
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

#ifndef VIEW_WIDGET_H
#define VIEW_WIDGET_H

#include "config.h"

#include <vector>

#include <QMainWindow>
class QSettings;

#include <gta/gta.hpp>

#include "base/fio.h" // for a fixed off_t on W32


class ViewWidget : public QMainWindow
{
Q_OBJECT

public:
    virtual void init(
            int* argc, char** argv, QSettings* settings,
            const std::string& file_name,
            const std::string& save_name,
            const std::vector<gta::header*>& headers,
            const std::vector<off_t>& offsets) = 0;

    virtual void set_current(size_t index) = 0;

signals:
    void closed();
};

#endif
