/*
 * gui.h
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

#ifndef GUI_H
#define GUI_H

#include "config.h"

#include <string>
#include <vector>

#include <QMainWindow>
#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QTabWidget>

#include <gta/gta.hpp>

#include "cio.h"        // For a fixed off_t on W32


class TaglistWidget : public QWidget
{
Q_OBJECT

public:
    enum type { global, dimension, component };

private:
    gta::header *_header;
    enum type _type;
    uintmax_t _index;
    QTableWidget *_tablewidget;

public:
    TaglistWidget(gta::header *header, enum type type, uintmax_t index = 0, QWidget *parent = NULL);
    ~TaglistWidget();

public slots:
    void update();

signals:
    void changed(gta::header *header, enum type type, uintmax_t index);
};

class ArrayWidget : public QWidget
{
Q_OBJECT

private:
    gta::header *_header;
    QLabel *_dimensions_label;
    QLabel *_components_label;
    QLabel *_size_label;
    QTabWidget *_taglists_widget;

public:
    ArrayWidget(gta::header *_header, QWidget *parent = NULL);
    ~ArrayWidget();

public slots:
    void update();

signals:
    void changed(gta::header *header);
};

class FileWidget : public QWidget
{
Q_OBJECT

private:
    std::string _name;
    std::string _temp_name;
    bool _is_changed;
    std::vector<gta::header *> _headers;
    std::vector<off_t> _offsets;
    QTabWidget *_arrays_widget;

public:

    FileWidget(const std::string &name, const std::string &temp_name,
            const std::vector<gta::header *> &headers,
            const std::vector<off_t> &offsets,
            QWidget *parent = NULL);
    ~FileWidget();

    const std::vector<gta::header *> &headers() const
    {
        return _headers;
    }

    const std::string &name() const
    {
        return _name;
    }

    const std::string &temp_name() const
    {
        return _temp_name;
    }

public slots:
    void changed();    
};

class GUI : public QMainWindow
{
Q_OBJECT

private:
    QTabWidget *_tabwidget;

protected:
    void closeEvent(QCloseEvent *event);	
	
public:
    GUI();
    ~GUI();

    void open(const std::string &filename);

private slots:
    void help_about();
};

#endif
