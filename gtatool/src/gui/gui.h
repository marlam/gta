/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010, 2011, 2012, 2013
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
#include <QLineEdit>
#include <QTabWidget>
#include <QDir>
#include <QStringList>
#include <QComboBox>
#include <QSpinBox>
#include <QGridLayout>
#include <QFileSystemWatcher>

#include <gta/gta.hpp>


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
    size_t _index;
    gta::header *_header;
    QLineEdit *_dimensions_ledt;
    QLineEdit *_components_ledt;
    QLineEdit *_size_ledt;
    QComboBox *_compression_combobox;
    MyTabWidget *_taglists_widget;

private slots:
    void compression_changed(int index);
    void taglist_changed(gta::header *header, int type, uintmax_t index);

public:
    ArrayWidget(size_t index, gta::header *_header, QWidget *parent = NULL);
    ~ArrayWidget();

    void saved();

public slots:
    void update();

signals:
    void changed(size_t index);
};

class FileWidget : public QWidget
{
Q_OBJECT

private:
    std::string _file_name;
    std::string _save_name;
    bool _is_changed;
    std::vector<gta::header *> _headers;
    std::vector<bool> _array_changed;
    QLabel *_array_label;
    QSpinBox *_array_spinbox;
    QGridLayout *_array_widget_layout;
    ArrayWidget* _array_widget;

private slots:
    void update_label();
    void update_array();
    void array_changed(size_t index);

public:

    FileWidget(const std::string &file_name, const std::string &save_name,
            const std::vector<gta::header *> &headers,
            QWidget *parent = NULL);
    ~FileWidget();

    const std::string &file_name() const
    {
        return _file_name;
    }

    const std::string &save_name() const
    {
        return _save_name;
    }

    bool is_changed() const
    {
        return _is_changed;
    }

    bool is_saved() const
    {
        return (_file_name.compare(_save_name) == 0 && !_is_changed);
    }

    const std::vector<gta::header *> &headers() const
    {
        return _headers;
    }

    int array_index() const
    {
        return _array_spinbox->value();
    }

    void set_file_name(const std::string &file_name);

    void saved_to(const std::string &save_name);

signals:
    void changed(const std::string &file_name, const std::string &save_name);
};

class GUI : public QMainWindow
{
Q_OBJECT

private:
    MyTabWidget *_files_widget;
    QFileSystemWatcher* _files_watcher;
    QDir _last_dir;

    bool check_have_file();
    bool check_file_unchanged();
    bool check_all_files_unchanged();
    QStringList file_open_dialog(const QStringList &filters = QStringList());
    QString file_save_dialog(const QString &default_suffix = "gta", const QStringList &filters = QStringList("GTA files (*.gta)"),
            const QString &existing_name = QString());
    int run(const std::string &cmd, const std::vector<std::string> &argv,
            std::string &std_err, FILE *std_out = NULL, FILE *std_in = NULL);
    void output_cmd(const std::string &cmd, const std::vector<std::string> &args, const std::string &output_name);
    void import_from(const std::string &cmd, const std::vector<std::string> &options, const QStringList &filters);
    void export_to(const std::string &cmd, const std::vector<std::string> &options, const QString &default_suffix, const QStringList &filters);

private slots:
    void file_changed(const std::string &file_name, const std::string &save_name);
    void file_changed_on_disk(const QString& name);
    void tab_close(int index);

protected:
    void closeEvent(QCloseEvent *event);	
	
public:
    GUI();
    ~GUI();

    void open(const std::string &file_name, const std::string &save_name, int tab_index = -1);

private slots:
    void file_open();
    void file_save();
    void file_save_as();
    void file_save_all();
    void file_close();
    void file_close_all();
    void file_import();
    void file_import_csv();
    void file_import_datraw();
    void file_import_dcmtk();
    void file_import_exr();
    void file_import_ffmpeg();
    void file_import_gdal();
    void file_import_jpeg();
    void file_import_magick();
    void file_import_mat();
    void file_import_netcdf();
    void file_import_pcd();
    void file_import_pfs();
    void file_import_ply();
    void file_import_pvm();
    void file_import_rat();
    void file_import_raw();
    void file_import_sndfile();
    void file_import_teem();
    void file_export();
    void file_export_csv();
    void file_export_datraw();
    void file_export_exr();
    void file_export_gdal();
    void file_export_jpeg();
    void file_export_magick();
    void file_export_mat();
    void file_export_netcdf();
    void file_export_pcd();
    void file_export_pfs();
    void file_export_ply();
    void file_export_pvm();
    void file_export_rat();
    void file_export_raw();
    void file_export_sndfile();
    void file_export_teem();
    void stream_extract();
    void stream_foreach();
    void stream_grep();
    void stream_merge();
    void stream_split();
    void array_combine();
    void array_create();
    void array_diff();
    void array_extract();
    void array_fill();
    void array_merge();
    void array_resize();
    void array_set();
    void dimension_add();
    void dimension_extract();
    void dimension_flatten();
    void dimension_merge();
    void dimension_reorder();
    void dimension_reverse();
    void dimension_split();
    void component_add();
    void component_compute();
    void component_convert();
    void component_extract();
    void component_merge();
    void component_reorder();
    void component_set();
    void component_split();
    void help_overview();
    void help_website();
    void help_about();
};

#endif
