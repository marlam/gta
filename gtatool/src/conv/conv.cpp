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

#include "config.h"

#include <vector>
#include <string>
#include <cctype>

#include "base/exc.h"
#include "base/msg.h"

#include "cmds.h"

#include "conv.h"
#include "filters.h"


int conv(bool import, const std::vector<std::string>& arguments, int argc, char *argv[])
{
    int retval = 0;
    try {
        std::string extension;
        int arg_index = (import ? 0 : arguments.size() - 1);
        size_t last_dot = arguments[arg_index].find_last_of('.');
        if (last_dot != std::string::npos) {
            extension = arguments[arg_index].substr(last_dot + 1);
            for (size_t i = 0; i < extension.size(); i++)
                extension[i] = std::tolower(extension[i]);
        }
        std::vector<std::string> filters = find_filters(extension, import);
        int filter_index = -1;
        for (size_t i = 0; i < filters.size(); i++) {
            std::string filter = (import ? std::string("from-") : std::string("to-")) + filters[i];
            int cmd_index = cmd_find(filter.c_str());
            if (cmd_index >= 0 && cmd_is_available(cmd_index)) {
                filter_index = cmd_index;
                break;
            }
        }
        if (filter_index < 0) {
            throw exc("automatic filter detection failed; please try manually.");
        } else {
            cmd_open(filter_index);
            retval = cmd_run(filter_index, argc, argv);
        }
    }
    catch (std::exception &e) {
        msg::err_txt("%s", e.what());
        return 1;
    }
    return retval;
}
