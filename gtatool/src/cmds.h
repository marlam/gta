/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011
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

#ifndef CMDS_H
#define CMDS_H


/*
 * Search commands
 */

/* Get the total number of commands. */
int cmd_count();

/* Get the name of the command with the given index. */
const char *cmd_name(int cmd_index);

/* Get the category of the command with the given index. */
typedef enum
{
    component,
    dimension,
    array,
    stream,
    conversion,
    misc
} cmd_category_t;
cmd_category_t cmd_category(int cmd_index);

/* Check if a command is available in this version (otherwise it cannot be executed). */
bool cmd_is_available(int cmd_index);

/* Find the command with the given name. Return its index, or -1 if not found. */
int cmd_find(const char *cmd);


/*
 * Execute commands
 */

/* Open the given command. */
void cmd_open(int cmd_index);

/* Execute the help function of the previously opened command with the given index. */
void cmd_run_help(int cmd_index);

/* Execute the previously opened command with the given index. */
int cmd_run(int cmd_index, int argc, char *argv[]);

/* Close a previously opened command. */
void cmd_close(int cmd_index);


#endif
