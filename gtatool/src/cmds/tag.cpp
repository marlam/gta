/*
 * tag.cpp
 *
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010  Martin Lambers <marlam@marlam.de>
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

#include <sstream>
#include <cstdio>
#include <cctype>

#include <gta/gta.hpp>

#include "msg.h"
#include "opt.h"
#include "cio.h"
#include "str.h"

#include "lib.h"

class tag_command
{
private:

    int _cmd;
    uintmax_t _index;
    bool _index_all;
    std::string _name;
    std::string _value;

public:

    enum
    {
        GET_GLOBAL,
        SET_GLOBAL,
        UNSET_GLOBAL,
        UNSET_GLOBAL_ALL,
        GET_DIMENSION,
        SET_DIMENSION,
        UNSET_DIMENSION,
        UNSET_DIMENSION_ALL,
        GET_COMPONENT,
        SET_COMPONENT,
        UNSET_COMPONENT,
        UNSET_COMPONENT_ALL,
        UNSET_ALL
    };

    tag_command(int cmd, uintmax_t index, bool index_all, const std::string &name, const std::string &value)
        : _cmd(cmd), _index(index), _index_all(index_all), _name(name), _value(value)
    {
    }

    void apply(const std::string &filename, uintmax_t gta_index, gta::header &hdr) throw (std::exception)
    {
        std::string array_name = filename + " array " + str::str(gta_index);
        switch (_cmd)
        {
        case GET_GLOBAL:
            {
                const char *val = hdr.global_taglist().get(to_utf8(_name).c_str());
                msg::req(array_name + " global: " + _name + (val ? std::string("=") + from_utf8(val) : std::string(" not set")));
            }
            break;

        case SET_GLOBAL:
            {
                hdr.global_taglist().set(to_utf8(_name).c_str(), to_utf8(_value).c_str());
            }
            break;

        case UNSET_GLOBAL:
            {
                hdr.global_taglist().unset(to_utf8(_name).c_str());
            }
            break;

        case UNSET_GLOBAL_ALL:
            {
                hdr.global_taglist().unset_all();
            }
            break;

        case GET_DIMENSION:
            {
                if (hdr.dimensions() == 0)
                {
                    throw exc(array_name + ": array has no dimensions");
                }
                uintmax_t a = 0, b = hdr.dimensions() - 1;
                if (!_index_all)
                {
                    if (_index >= hdr.dimensions())
                    {
                        throw exc(array_name + ": dimension index too big");
                    }
                    a = _index;
                    b = _index;
                }
                for (uintmax_t i = a; i <= b; i++)
                {
                    const char *val = hdr.dimension_taglist(_index).get(to_utf8(_name).c_str());
                    msg::req(array_name + " dimension " + str::str(_index) + ": "
                            + _name + (val ? std::string("=") + from_utf8(val) : std::string(" not set")));
                }
            }
            break;

        case SET_DIMENSION:
            {
                if (hdr.dimensions() == 0)
                {
                    throw exc(array_name + ": array has no dimensions");
                }
                uintmax_t a = 0, b = hdr.dimensions() - 1;
                if (!_index_all)
                {
                    if (_index >= hdr.dimensions())
                    {
                        throw exc(array_name + "dimension index too big");
                    }
                    a = _index;
                    b = _index;
                }
                for (uintmax_t i = a; i <= b; i++)
                {
                    hdr.dimension_taglist(_index).set(to_utf8(_name).c_str(), to_utf8(_value).c_str());
                }
            }
            break;

        case UNSET_DIMENSION:
            {
                if (hdr.dimensions() == 0)
                {
                    throw exc(array_name + ": array has no dimensions");
                }
                uintmax_t a = 0, b = hdr.dimensions() - 1;
                if (!_index_all)
                {
                    if (_index >= hdr.dimensions())
                    {
                        throw exc(array_name + ": dimension index too big");
                    }
                    a = _index;
                    b = _index;
                }
                for (uintmax_t i = a; i <= b; i++)
                {
                    hdr.dimension_taglist(_index).unset(to_utf8(_name).c_str());
                }
            }
            break;

        case UNSET_DIMENSION_ALL:
            {
                if (hdr.dimensions() == 0)
                {
                    throw exc(array_name + ": array has no dimensions");
                }
                uintmax_t a = 0, b = hdr.dimensions() - 1;
                if (!_index_all)
                {
                    if (_index >= hdr.dimensions())
                    {
                        throw exc(array_name + ": dimension index too big");
                    }
                    a = _index;
                    b = _index;
                }
                for (uintmax_t i = a; i <= b; i++)
                {
                    hdr.dimension_taglist(_index).unset_all();
                }
            }
            break;

        case GET_COMPONENT:
            {
                if (hdr.components() == 0)
                {
                    throw exc(array_name + ": array has no components");
                }
                uintmax_t a = 0, b = hdr.components() - 1;
                if (!_index_all)
                {
                    if (_index >= hdr.components())
                    {
                        throw exc(array_name + ": component index too big");
                    }
                    a = _index;
                    b = _index;
                }
                for (uintmax_t i = a; i <= b; i++)
                {
                    const char *val = hdr.component_taglist(_index).get(to_utf8(_name).c_str());
                    msg::req(array_name + " component " + str::str(_index) + ": "
                            + _name + (val ? std::string("=") + from_utf8(val) : std::string(" not set")));
                }
            }
            break;

        case SET_COMPONENT:
            {
                if (hdr.components() == 0)
                {
                    throw exc(array_name + ": array has no components");
                }
                uintmax_t a = 0, b = hdr.components() - 1;
                if (!_index_all)
                {
                    if (_index >= hdr.components())
                    {
                        throw exc(array_name + ": component index too big");
                    }
                    a = _index;
                    b = _index;
                }
                for (uintmax_t i = a; i <= b; i++)
                {
                    hdr.component_taglist(_index).set(to_utf8(_name).c_str(), to_utf8(_value).c_str());
                }
            }
            break;

        case UNSET_COMPONENT:
            {
                if (hdr.components() == 0)
                {
                    throw exc(array_name + ": array has no components");
                }
                uintmax_t a = 0, b = hdr.components() - 1;
                if (!_index_all)
                {
                    if (_index >= hdr.components())
                    {
                        throw exc(array_name + ": component index too big");
                    }
                    a = _index;
                    b = _index;
                }
                for (uintmax_t i = a; i <= b; i++)
                {
                    hdr.component_taglist(_index).unset(to_utf8(_name).c_str());
                }
            }
            break;

        case UNSET_COMPONENT_ALL:
            {
                if (hdr.components() == 0)
                {
                    throw exc(array_name + ": array has no components");
                }
                uintmax_t a = 0, b = hdr.components() - 1;
                if (!_index_all)
                {
                    if (_index >= hdr.components())
                    {
                        throw exc(array_name + ": component index too big");
                    }
                    a = _index;
                    b = _index;
                }
                for (uintmax_t i = a; i <= b; i++)
                {
                    hdr.component_taglist(_index).unset_all();
                }
            }
            break;

        case UNSET_ALL:
            {
                hdr.global_taglist().unset_all();
                for (uintmax_t i = 0; i < hdr.dimensions(); i++)
                {
                    hdr.dimension_taglist(i).unset_all();
                }
                for (uintmax_t i = 0; i < hdr.components(); i++)
                {
                    hdr.component_taglist(i).unset_all();
                }
            }
            break;

        default:
            /* cannot happen */
            break;
        }
    }
};

std::vector<tag_command> tag_commands;

class opt_tag_command : public opt::option
{
    private:
        int _tag_cmd;

        bool parse_index(const std::string &arg, uintmax_t *index, bool *index_all)
        {
            if (arg.compare("all") == 0)
            {
                *index = 0;
                *index_all = true;
            }
            else
            {
                std::stringstream ss(arg);
                *index_all = false;
                ss >> *index;
                if (ss.fail())
                {
                    return false;
                }
            }
            return true;
        }

        bool parse_namevalue_pair(const std::string &arg, std::string *name, std::string *value)
        {
            size_t eq = arg.find_first_of('=');
            if (eq == 0 || eq == std::string::npos)
            {
                return false;
            }
            *name = arg.substr(0, eq);
            *value = arg.substr(eq + 1, std::string::npos);
            return true;
        }

        bool parse_indexname_pair(const std::string &arg, uintmax_t *index, bool *index_all, std::string *name)
        {
            std::string index_string;

            size_t comma = arg.find_first_of(',');
            if (comma == 0 || comma == std::string::npos)
            {
                return false;
            }
            index_string = arg.substr(0, comma);
            if (!parse_index(index_string, index, index_all))
            {
                return false;
            }
            *name = arg.substr(comma + 1, std::string::npos);
            return true;
        }

        bool parse_indexnamevalue_tupel(const std::string &arg, uintmax_t *index, bool *index_all,
                std::string *name, std::string *value)
        {
            std::string index_string;
            std::string namevalue_string;

            size_t comma = arg.find_first_of(',');
            if (comma == 0 || comma == std::string::npos)
            {
                return false;
            }
            index_string = arg.substr(0, comma);
            namevalue_string = arg.substr(comma + 1, std::string::npos);
            if (!parse_index(index_string, index, index_all))
            {
                return false;
            }
            return parse_namevalue_pair(namevalue_string, name, value);
        }

    public:

        opt_tag_command(const std::string &longname, int tag_cmd)
            : opt::option(longname, '\0', opt::optional), _tag_cmd(tag_cmd)
        {
        }
        ~opt_tag_command()
        {
        }

        enum opt::option::argument_policy argument_policy() const
        {
            return opt::option::optional_argument;
        }

        bool parse_argument(const std::string &s)
        {
            uintmax_t index;
            bool index_all;
            std::string name;
            std::string value;

            if (_tag_cmd == tag_command::UNSET_GLOBAL_ALL || _tag_cmd == tag_command::UNSET_ALL)
            {
                if (!s.empty())
                {
                    return false;
                }
            }
            else
            {
                if (s.empty())
                {
                    return false;
                }
            }

            switch (_tag_cmd)
            {
            case tag_command::GET_GLOBAL:
                tag_commands.push_back(tag_command(_tag_cmd, 0, false, s, ""));
                break;

            case tag_command::SET_GLOBAL:
                if (!parse_namevalue_pair(s, &name, &value))
                    return false;
                tag_commands.push_back(tag_command(_tag_cmd, 0, false, name, value));
                break;

            case tag_command::UNSET_GLOBAL:
                tag_commands.push_back(tag_command(_tag_cmd, 0, false, s, ""));
                break;

            case tag_command::UNSET_GLOBAL_ALL:
                tag_commands.push_back(tag_command(_tag_cmd, 0, false, "", ""));
                break;

            case tag_command::GET_DIMENSION:
                if (!parse_indexname_pair(s, &index, &index_all, &name))
                    return false;
                tag_commands.push_back(tag_command(_tag_cmd, index, index_all, name, ""));
                break;

            case tag_command::SET_DIMENSION:
                if (!parse_indexnamevalue_tupel(s, &index, &index_all, &name, &value))
                    return false;
                tag_commands.push_back(tag_command(_tag_cmd, index, index_all, name, value));
                break;

            case tag_command::UNSET_DIMENSION:
                if (!parse_indexname_pair(s, &index, &index_all, &name))
                    return false;
                tag_commands.push_back(tag_command(_tag_cmd, index, index_all, name, ""));
                break;

            case tag_command::UNSET_DIMENSION_ALL:
                if (!parse_index(s, &index, &index_all))
                    return false;
                tag_commands.push_back(tag_command(_tag_cmd, index, index_all, "", ""));
                break;

            case tag_command::GET_COMPONENT:
                if (!parse_indexname_pair(s, &index, &index_all, &name))
                    return false;
                tag_commands.push_back(tag_command(_tag_cmd, index, index_all, name, ""));
                break;

            case tag_command::SET_COMPONENT:
                if (!parse_indexnamevalue_tupel(s, &index, &index_all, &name, &value))
                    return false;
                tag_commands.push_back(tag_command(_tag_cmd, index, index_all, name, value));
                break;

            case tag_command::UNSET_COMPONENT:
                if (!parse_indexname_pair(s, &index, &index_all, &name))
                    return false;
                tag_commands.push_back(tag_command(_tag_cmd, index, index_all, name, ""));
                break;

            case tag_command::UNSET_COMPONENT_ALL:
                if (!parse_index(s, &index, &index_all))
                    return false;
                tag_commands.push_back(tag_command(_tag_cmd, index, index_all, "", ""));
                break;

            case tag_command::UNSET_ALL:
                tag_commands.push_back(tag_command(_tag_cmd, 0, false, "", ""));
                break;

            default:
                /* cannot happen */
                break;
            }
            return true;
        }
};

extern "C" void gtatool_tag_help(void)
{
    msg::req_txt(
            "tag [--get-global=<name>] [--set-global=<name=value>] [--unset-global=<name>] [--unset-global-all] "
            "[--get-dimension=<dim>,<name>] [--set-dimension=<dim>,<name=value>] [--unset-dimension=<dim>,<name>] [--unset-dimension-all=<dim>] "
            "[--get-component=<cmp>,<name>] [--set-component=<cmp>,<name=value>] [--unset-component=<cmp>,<name>] [--unset-component-all=<cmp>] "
            "[--unset-all] [<files...>]\n"
            "\n"
            "Get and set GTA tags. The input GTAs are modified (if requested) and written to standard output.");
}

extern "C" int gtatool_tag(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt_tag_command get_global("get-global", tag_command::GET_GLOBAL);
    options.push_back(&get_global);
    opt_tag_command set_global("set-global", tag_command::SET_GLOBAL);
    options.push_back(&set_global);
    opt_tag_command unset_global("unset-global", tag_command::UNSET_GLOBAL);
    options.push_back(&unset_global);
    opt_tag_command unset_global_all("unset-global-all", tag_command::UNSET_GLOBAL_ALL);
    options.push_back(&unset_global_all);
    opt_tag_command get_dimension("get-dimension", tag_command::GET_DIMENSION);
    options.push_back(&get_dimension);
    opt_tag_command set_dimension("set-dimension", tag_command::SET_DIMENSION);
    options.push_back(&set_dimension);
    opt_tag_command unset_dimension("unset-dimension", tag_command::UNSET_DIMENSION);
    options.push_back(&unset_dimension);
    opt_tag_command unset_dimension_all("unset-dimension-all", tag_command::UNSET_DIMENSION_ALL);
    options.push_back(&unset_dimension_all);
    opt_tag_command get_component("get-component", tag_command::GET_COMPONENT);
    options.push_back(&get_component);
    opt_tag_command set_component("set-component", tag_command::SET_COMPONENT);
    options.push_back(&set_component);
    opt_tag_command unset_component("unset-component", tag_command::UNSET_COMPONENT);
    options.push_back(&unset_component);
    opt_tag_command unset_component_all("unset-component-all", tag_command::UNSET_COMPONENT_ALL);
    options.push_back(&unset_component_all);
    opt_tag_command unset_all("unset-all", tag_command::UNSET_ALL);
    options.push_back(&unset_all);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_tag_help();
        return 0;
    }

    if (cio::isatty(stdout))
    {
        msg::err("refusing to write to a tty");
        return 1;
    }

    try
    {
        gta::header hdr;
        // Loop over all input files
        size_t arg = 0;
        do
        {
            std::string finame = (arguments.size() == 0 ? "standard input" : arguments[arg]);
            FILE *fi = (arguments.size() == 0 ? stdin : cio::open(finame, "r"));

            // Loop over all GTAs inside the current file
            uintmax_t array = 0;
            while (cio::has_more(fi, finame))
            {
                // Read the GTA header and apply tag commands
                hdr.read_from(fi);
                for (uintmax_t i = 0; i < tag_commands.size(); i++)
                {
                    tag_commands[i].apply(finame, array, hdr);
                }
                // Write the GTA header
                hdr.write_to(stdout);
                // Copy the GTA data
                hdr.copy_data(fi, hdr, stdout);
                array++;
            }
            if (fi != stdin)
            {
                cio::close(fi);
            }
            arg++;
        }
        while (arg < arguments.size());
    }
    catch (std::exception &e)
    {
        msg::err("%s", e.what());
        return 1;
    }

    return 0;
}
