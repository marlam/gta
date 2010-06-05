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
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>
#include <QTabWidget>
#include <QDir>
#include <QStringList>
#include <QComboBox>

#include <gta/gta.hpp>

#include "cio.h"        // For a fixed off_t on W32


class MyTabWidget : public QTabWidget
{
    /* The only purpose of this class is to get access to tabBar(),
     * which is protected in QTabWidget */
public:
    MyTabWidget(QWidget *parent = NULL) : QTabWidget(parent) {}
    ~MyTabWidget() {}
    QTabBar *tabBar() const { return QTabWidget::tabBar(); }
};

class TaglistWidget : public QWidget
{
Q_OBJECT

public:
    enum type { global, dimension, component };

private:
    gta::header *_header;
    enum type _type;
    uintmax_t _index;
    bool _cell_change_lock;
    bool _cell_change_add_mode;
    QTableWidget *_tablewidget;
    QPushButton *_remove_button;
    QPushButton *_add_button;

private slots:
    void cell_changed(int row, int column);
    void selection_changed();
    void remove();
    void add();

public:
    TaglistWidget(gta::header *header, enum type type, uintmax_t index = 0, QWidget *parent = NULL);
    ~TaglistWidget();

public slots:
    void update();

signals:
    void changed(gta::header *header, int type, uintmax_t index);
};

class ArrayWidget : public QWidget
{
Q_OBJECT

private:
    gta::header *_header;
    QLabel *_dimensions_label;
    QLabel *_components_label;
    QLabel *_size_label;
    QComboBox *_compression_combobox;
    MyTabWidget *_taglists_widget;

private slots:
    void compression_changed(int index);
    void taglist_changed(gta::header *header, int type, uintmax_t index);

public:
    ArrayWidget(gta::header *_header, QWidget *parent = NULL);
    ~ArrayWidget();

    void saved();

public slots:
    void update();

signals:
    void changed(gta::header *header);
};

class FileWidget : public QWidget
{
Q_OBJECT

private:
    FILE *_f;
    std::string _name;
    bool _is_changed;
    std::vector<gta::header *> _headers;
    std::vector<off_t> _offsets;
    MyTabWidget *_arrays_widget;

private slots:
    void array_changed(gta::header *header);

public:

    FileWidget(FILE *f, const std::string &name,
            const std::vector<gta::header *> &headers,
            const std::vector<off_t> &offsets,
            QWidget *parent = NULL);
    ~FileWidget();

    FILE *file() const
    {
        return _f;
    }

    const std::string &name() const
    {
        return _name;
    }

    const std::vector<gta::header *> &headers() const
    {
        return _headers;
    }

    const std::vector<off_t> &offsets() const
    {
        return _offsets;
    }

    MyTabWidget *arrays_widget()
    {
        return _arrays_widget;
    }

    bool is_changed() const
    {
        return _is_changed;
    }

    void saved(FILE *f);

    void set_name(const std::string &name);

signals:
    void changed(const std::string &name);
};

class GUI : public QMainWindow
{
Q_OBJECT

private:
    MyTabWidget *_files_widget;
    QDir _last_file_open_dir;
    QDir _last_file_save_as_dir;

    bool check_have_file();
    bool check_file_saved();
    bool check_all_files_saved();
    QStringList file_open_dialog(const QStringList &filters = QStringList());
    QString file_save_dialog(const QString &default_suffix = "gta", const QStringList &filters = QStringList("GTA files (*.gta)"),
            const QString &existing_name = QString());
    int run(const std::string &cmd, const std::vector<std::string> &argv,
            std::string &std_err, FILE *std_out = NULL, FILE *std_in = NULL);
    void import_from(const std::string &cmd, const std::vector<std::string> &options, const QStringList &filters);
    void export_to(const std::string &cmd, const std::vector<std::string> &options, const QString &default_suffix, const QStringList &filters);

private slots:
    void file_changed(const std::string &name);

protected:
    void closeEvent(QCloseEvent *event);	
	
public:
    GUI();
    ~GUI();

    void open(const std::string &filename);

private slots:
    void file_open();
    void file_save();
    void file_save_as();
    void file_save_all();
    void file_close();
    void file_close_all();
    void file_import_dcmtk();
    void file_import_exr();
    void file_import_gdal();
    void file_import_magick();
    void file_import_pfs();
    void file_import_raw();
    void file_export_exr();
    void file_export_gdal();
    void file_export_magick();
    void file_export_pfs();
    void file_export_raw();
    void stream_merge();
    void stream_split();
    void stream_extract();
    void help_about();
};

#endif
