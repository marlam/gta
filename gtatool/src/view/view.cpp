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

#include "config.h"

#include "base/msg.h"

#include "cmds.h"
#include "lib.h"

#include "widget.hpp"
#if WITH_EQUALIZER
# include "renderer.hpp"
# include "eq/eqwindow.hpp"
#endif


extern "C" void gtatool_view_help(void)
{
    msg::req_txt(
            "view [<files...>]\n"
            "\n"
            "Visualizes the content of the given GTA files, if any.");
}

extern "C" int gtatool_view(int argc, char *argv[])
{
#if WITH_EQUALIZER
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--eq-client") == 0) {
            RendererFactory renderer_factory;
            EQWindow* eqwindow = new EQWindow(&renderer_factory, NULL, 0, gtatool_argc, gtatool_argv);
            return 0;
        }
    }
#endif
    int i = cmd_find("gui");
    cmd_open(i);
    int ret = cmd_run(i, argc, argv);
    cmd_close(i);
    return ret;
}

extern "C" ViewWidget* gtatool_view_create(void)
{
    return new View;
}
