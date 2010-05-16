/*
 * gui.cpp
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

#include <string>

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QTabWidget>
#include <QGridLayout>
#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>

#include <gta/gta.hpp>

#include "msg.h"
#include "opt.h"
#include "cio.h"
#include "str.h"
#include "intcheck.h"

#include "lib.h"
#include "cmds.h"
#include "gui.h"


extern "C" void gtatool_gui_help(void)
{
    msg::req_txt(
            "gui [<files...>]\n"
            "\n"
            "Starts a graphical user interface (GUI) and opens the given GTA files, if any.");
}


TaglistWidget::TaglistWidget(gta::header *header, enum type type, uintmax_t index, QWidget *parent)
    : QWidget(parent), _header(header), _type(type), _index(index),
    _cell_change_lock(true), _cell_change_add_mode(false)
{
    _tablewidget = new QTableWidget(this);
    _tablewidget->setColumnCount(2);
    QStringList header_labels;
    header_labels.append("Name");
    header_labels.append("Value");
    _tablewidget->setHorizontalHeaderLabels(header_labels);
    _tablewidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    _tablewidget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    _tablewidget->horizontalHeader()->hide();
    _tablewidget->verticalHeader()->hide();
    connect(_tablewidget, SIGNAL(itemSelectionChanged()), this, SLOT(selection_changed()));
    connect(_tablewidget, SIGNAL(cellChanged(int, int)), this, SLOT(cell_changed(int, int)));
    _remove_button = new QPushButton("Remove");
    _remove_button->setEnabled(false);
    connect(_remove_button, SIGNAL(pressed()), this, SLOT(remove()));
    _add_button = new QPushButton("Add");
    connect(_add_button, SIGNAL(pressed()), this, SLOT(add()));
    update();
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(_tablewidget, 0, 0, 1, 2);
    layout->addWidget(_remove_button, 1, 0, 1, 1);
    layout->addWidget(_add_button, 1, 1, 1, 1);
    layout->setRowStretch(0, 1);
    setLayout(layout);
}

TaglistWidget::~TaglistWidget()
{
}

void TaglistWidget::update()
{
    _cell_change_lock = true;
    _tablewidget->clearContents();
    const gta::taglist &taglist = (
              _type == global ? _header->global_taglist()
            : _type == dimension ? _header->dimension_taglist(_index)
            : _header->component_taglist(_index));
    _tablewidget->setRowCount(checked_cast<int>(taglist.tags()));
    QLabel size_dummy("Hg");
    int row_height = size_dummy.sizeHint().height() + 2;
    for (uintmax_t i = 0; i < taglist.tags(); i++)
    {
        int row = checked_cast<int>(i);
        _tablewidget->setItem(row, 0, new QTableWidgetItem(QString::fromUtf8(taglist.name(i))));
        _tablewidget->setItem(row, 1, new QTableWidgetItem(QString::fromUtf8(taglist.value(i))));
        _tablewidget->setRowHeight(row, row_height);
    }
    _cell_change_lock = false;
}

void TaglistWidget::selection_changed()
{
    _remove_button->setEnabled(!_tablewidget->selectedItems().empty());
}

void TaglistWidget::cell_changed(int row, int column)
{
    if (_cell_change_lock)
        return;

    uintmax_t index = row;
    try
    {
        if (column == 0)
        {
            std::string new_name = _tablewidget->item(row, 0)->text().toUtf8().data();
            if (new_name.length() == 0)
            {
                _cell_change_add_mode = false;
                throw exc("tag names must not be empty");
            }
            else if (new_name.find_first_of('=') != std::string::npos)
            {
                _cell_change_add_mode = false;
                throw exc("tag names must not contain '='");
            }
            if (_cell_change_add_mode)
            {
                _cell_change_add_mode = false;
                std::string new_value = _tablewidget->item(row, 1)->text().toUtf8().data();
                if (_type == global)
                {
                    _header->global_taglist().set(new_name.c_str(), new_value.c_str());
                }
                else if (_type == dimension)
                {
                    _header->dimension_taglist(_index).set(new_name.c_str(), new_value.c_str());
                }
                else
                {
                    _header->component_taglist(_index).set(new_name.c_str(), new_value.c_str());
                }
            }
            else
            {
                if (_type == global)
                {
                    std::string value = _header->global_taglist().value(index);
                    std::string old_name = _header->global_taglist().name(index);
                    _header->global_taglist().unset(old_name.c_str());
                    _header->global_taglist().set(new_name.c_str(), value.c_str());
                }
                else if (_type == dimension)
                {
                    std::string value = _header->dimension_taglist(_index).value(index);
                    std::string old_name = _header->dimension_taglist(_index).name(index);
                    _header->dimension_taglist(_index).unset(old_name.c_str());
                    _header->dimension_taglist(_index).set(new_name.c_str(), value.c_str());
                }
                else
                {
                    std::string value = _header->component_taglist(_index).value(index);
                    std::string old_name = _header->component_taglist(_index).name(index);
                    _header->component_taglist(_index).unset(old_name.c_str());
                    _header->component_taglist(_index).set(new_name.c_str(), value.c_str());
                }
            }
        }
        else
        {
            std::string new_value = _tablewidget->item(row, column)->text().toUtf8().data();
            if (_type == global)
            {
                std::string name = _header->global_taglist().name(index);
                _header->global_taglist().set(name.c_str(), new_value.c_str());
            }
            else if (_type == dimension)
            {
                std::string name = _header->dimension_taglist(_index).name(index);
                _header->dimension_taglist(_index).set(name.c_str(), new_value.c_str());
            }
            else
            {
                std::string name = _header->component_taglist(_index).name(index);
                _header->component_taglist(_index).set(name.c_str(), new_value.c_str());
            }
        }
        emit changed(_header, _type, _index);
    }
    catch (std::exception &e)
    {
        QMessageBox::critical(this, "Error", (std::string("Tag update failed: ") + e.what()).c_str());
    }
    update();
}

void TaglistWidget::add()
{
    _cell_change_lock = true;
    int row = _tablewidget->rowCount();
    _tablewidget->setRowCount(row + 1);
    _tablewidget->setItem(row, 0, new QTableWidgetItem(0));
    _tablewidget->setItem(row, 1, new QTableWidgetItem(QString("")));
    QLabel size_dummy("Hg");
    _tablewidget->setRowHeight(row, size_dummy.sizeHint().height() + 2);
    _tablewidget->setCurrentCell(row, 0);
    _cell_change_add_mode = true;
    _cell_change_lock = false;
    _tablewidget->editItem(_tablewidget->item(row, 0));
}

void TaglistWidget::remove()
{
    QList<QTableWidgetItem *> selected_items = _tablewidget->selectedItems();
    try
    {
        std::vector<std::string> selected_names(selected_items.size());
        for (int i = 0; i < selected_items.size(); i++)
        {
            uintmax_t index = selected_items[i]->row();
            if (_type == global)
            {
                selected_names[i] = _header->global_taglist().name(index);
            }
            else if (_type == dimension)
            {
                selected_names[i] = _header->dimension_taglist(_index).name(index);
            }
            else
            {
                selected_names[i] = _header->component_taglist(_index).name(index);
            }
        }
        for (size_t i = 0; i < selected_names.size(); i++)
        {
            if (_type == global)
            {
                _header->global_taglist().unset(selected_names[i].c_str());
            }
            else if (_type == dimension)
            {
                _header->dimension_taglist(_index).unset(selected_names[i].c_str());
            }
            else
            {
                _header->component_taglist(_index).unset(selected_names[i].c_str());
            }
        }
    }
    catch (std::exception &e)
    {
        QMessageBox::critical(this, "Error", (std::string("Tag removal failed: ") + e.what()).c_str());
    }
    update();
    emit changed(_header, _type, _index);
}

ArrayWidget::ArrayWidget(gta::header *header, QWidget *parent)
    : QWidget(parent), _header(header)
{
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(new QLabel("Dimensions:"), 0, 0, 1, 1);
    _dimensions_label = new QLabel("");
    layout->addWidget(_dimensions_label, 0, 1, 1, 3);
    layout->addWidget(new QLabel("Components:"), 1, 0, 1, 1);
    _components_label = new QLabel("");
    layout->addWidget(_components_label, 1, 1, 1, 3);
    layout->addWidget(new QLabel("Size:"), 2, 0, 1, 1);
    _size_label = new QLabel("");
    layout->addWidget(_size_label, 2, 1, 1, 3);
    _taglists_widget = new MyTabWidget;
    layout->addWidget(_taglists_widget, 3, 0, 1, 4);
    update();
    layout->setRowStretch(6, 1);
    layout->setColumnStretch(3, 1);
    setLayout(layout);
}

ArrayWidget::~ArrayWidget()
{
}

void ArrayWidget::taglist_changed(gta::header *, int type, uintmax_t index)
{
    if (type == TaglistWidget::global)
    {
        _taglists_widget->tabBar()->setTabTextColor(0, QColor("red"));
    }
    else if (type == TaglistWidget::dimension)
    {
        _taglists_widget->tabBar()->setTabTextColor(1 + index, QColor("red"));
    }
    else
    {
        _taglists_widget->tabBar()->setTabTextColor(1 + _header->dimensions() + index, QColor("red"));
    }
    emit changed(_header);
}

void ArrayWidget::saved()
{
    for (int i = 0; i < _taglists_widget->count(); i++)
    {
        _taglists_widget->tabBar()->setTabTextColor(i, QColor("black"));
    }
}

void ArrayWidget::update()
{
    std::string dimensions_string;
    for (uintmax_t i = 0; i < _header->dimensions(); i++)
    {
        dimensions_string += str::from(_header->dimension_size(i));
        if (i < _header->dimensions() - 1)
        {
            dimensions_string += " x ";
        }
    }
    _dimensions_label->setText(dimensions_string.c_str());
    std::string components_string;
    for (uintmax_t i = 0; i < _header->components(); i++)
    {
        components_string += type_to_string(_header->component_type(i), _header->component_size(i));
        if (i < _header->components() - 1)
        {
            components_string += ", ";
        }
    }
    _components_label->setText(components_string.c_str());
    _size_label->setText((str::from(_header->data_size()) + " bytes ("
                + str::human_readable_memsize(_header->data_size()) + ")").c_str());
    while (_taglists_widget->count() > 0)
    {
        QWidget *w = _taglists_widget->widget(0);
        _taglists_widget->removeTab(0);
        delete w;
    }
    TaglistWidget *global_taglist_widget = new TaglistWidget(_header, TaglistWidget::global);
    connect(global_taglist_widget, SIGNAL(changed(gta::header *, int, uintmax_t)),
            this, SLOT(taglist_changed(gta::header *, int, uintmax_t)));
    _taglists_widget->addTab(global_taglist_widget, QString("Global"));
    _taglists_widget->tabBar()->setTabTextColor(_taglists_widget->indexOf(global_taglist_widget), "black");
    for (uintmax_t i = 0; i < _header->dimensions(); i++)
    {
        TaglistWidget *dimension_taglist_widget = new TaglistWidget(_header, TaglistWidget::dimension, i);
        connect(dimension_taglist_widget, SIGNAL(changed(gta::header *, int, uintmax_t)),
                this, SLOT(taglist_changed(gta::header *, int, uintmax_t)));
        _taglists_widget->addTab(dimension_taglist_widget, QString((std::string("Dim ") + str::from(i)
                        /* + " (" + str::from(_header->dimension_size(i)) + ")" */).c_str()));
        _taglists_widget->tabBar()->setTabTextColor(_taglists_widget->indexOf(dimension_taglist_widget), "black");
    }
    for (uintmax_t i = 0; i < _header->components(); i++)
    {
        TaglistWidget *component_taglist_widget = new TaglistWidget(_header, TaglistWidget::component, i);
        connect(component_taglist_widget, SIGNAL(changed(gta::header *, int, uintmax_t)),
                this, SLOT(taglist_changed(gta::header *, int, uintmax_t)));
        _taglists_widget->addTab(component_taglist_widget, QString((std::string("Comp ") + str::from(i)
                        /* + " (" + type_to_string(_header->component_type(i), _header->component_size(i)) + ")" */).c_str()));
        _taglists_widget->tabBar()->setTabTextColor(_taglists_widget->indexOf(component_taglist_widget), "black");
    }
}

FileWidget::FileWidget(FILE *f, const std::string &name, const std::string &temp_name,
        const std::vector<gta::header *> &headers,
        const std::vector<off_t> &offsets,
        QWidget *parent)
    : QWidget(parent),
    _f(f), _name(name), _temp_name(temp_name), _is_changed(name.length() == 0),
    _headers(headers), _offsets(offsets)
{
    _arrays_widget = new MyTabWidget;
    for (size_t i = 0; i < headers.size(); i++)
    {
        ArrayWidget *aw = new ArrayWidget(headers[i]);
        connect(aw, SIGNAL(changed(gta::header *)), this, SLOT(array_changed(gta::header *)));
        _arrays_widget->addTab(aw, QString((std::string("Array ") + str::from(i)).c_str()));
        _arrays_widget->tabBar()->setTabTextColor(_arrays_widget->indexOf(aw), "black");
    }    
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(_arrays_widget, 0, 0);
    layout->setRowStretch(0, 1);
    layout->setColumnStretch(0, 1);
    setLayout(layout);
}

FileWidget::~FileWidget()
{
}

void FileWidget::array_changed(gta::header *header)
{
    size_t array_index = 0;
    for (size_t i = 0; i < _headers.size(); i++)
    {
        if (_headers[i] == header)
        {
            array_index = i;
            break;
        }
    }
    _arrays_widget->tabBar()->setTabTextColor(array_index, QColor("red"));
    _is_changed = true;
    emit changed(_name, _temp_name);
}

void FileWidget::saved(FILE *f)
{
    _is_changed = false;
    _f = f;
    for (int i = 0; i < _arrays_widget->count(); i++)
    {
        ArrayWidget *aw = reinterpret_cast<ArrayWidget *>(_arrays_widget->widget(i));
        aw->saved();
        _arrays_widget->tabBar()->setTabTextColor(i, QColor("black"));
    }
}

void FileWidget::set_name(const std::string &name)
{
    _name = name;
    _is_changed = true;
}

GUI::GUI()
{
    setWindowTitle(PACKAGE_NAME);
    setWindowIcon(QIcon(":gui.png"));
    QWidget *widget = new QWidget;
    QGridLayout *layout = new QGridLayout;
    _files_widget = new MyTabWidget;
    layout->addWidget(_files_widget, 0, 0);
    layout->setRowStretch(0, 1);
    layout->setColumnStretch(0, 1);
    widget->setLayout(layout);
    setCentralWidget(widget);

    QMenu *file_menu = menuBar()->addMenu(tr("&File"));
    QAction *file_open_action = new QAction(tr("&Open..."), this);
    file_open_action->setShortcut(tr("Ctrl+O"));
    connect(file_open_action, SIGNAL(triggered()), this, SLOT(file_open()));
    file_menu->addAction(file_open_action);
    QAction *file_save_action = new QAction(tr("&Save"), this);
    file_save_action->setShortcut(tr("Ctrl+S"));
    connect(file_save_action, SIGNAL(triggered()), this, SLOT(file_save()));
    file_menu->addAction(file_save_action);
    QAction *file_save_as_action = new QAction(tr("Save &as..."), this);
    connect(file_save_as_action, SIGNAL(triggered()), this, SLOT(file_save_as()));
    file_menu->addAction(file_save_as_action);
    QAction *file_save_all_action = new QAction(tr("Save all"), this);
    connect(file_save_all_action, SIGNAL(triggered()), this, SLOT(file_save_all()));
    file_menu->addAction(file_save_all_action);
    QAction *file_close_action = new QAction(tr("&Close"), this);
    file_close_action->setShortcut(tr("Ctrl+C"));
    connect(file_close_action, SIGNAL(triggered()), this, SLOT(file_close()));
    file_menu->addAction(file_close_action);
    QAction *file_close_all_action = new QAction(tr("Close all"), this);
    connect(file_close_all_action, SIGNAL(triggered()), this, SLOT(file_close_all()));
    file_menu->addAction(file_close_all_action);
    file_menu->addSeparator();
    QAction *quit_action = new QAction(tr("&Quit"), this);
    quit_action->setShortcut(tr("Ctrl+Q"));
    connect(quit_action, SIGNAL(triggered()), this, SLOT(close()));
    file_menu->addAction(quit_action);

    QMenu *help_menu = menuBar()->addMenu(tr("&Help"));
    QAction *help_about_action = new QAction(tr("&About"), this);
    connect(help_about_action, SIGNAL(triggered()), this, SLOT(help_about()));
    help_menu->addAction(help_about_action);
}

GUI::~GUI()
{
}

void GUI::closeEvent(QCloseEvent *event)
{
    file_close_all();
    if (_files_widget->count() == 0)
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void GUI::file_changed(const std::string &name, const std::string &temp_name)
{
    int file_index = 0;
    for (int i = 0; i < _files_widget->count(); i++)
    {
        FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->widget(i));
        if (fw->name().compare(name) == 0 && fw->temp_name().compare(temp_name) == 0)
        {
            file_index = i;
            break;
        }
    }
    _files_widget->tabBar()->setTabTextColor(file_index, QColor("red"));
}

void GUI::open(const std::string &filename)
{
    QFileInfo file_info(cio::to_sys(filename).c_str());
    for (int i = 0; i < _files_widget->count(); i++)
    {
        FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->widget(i));
        QFileInfo existing_file_info(cio::to_sys(fw->name()).c_str());
        if (file_info.canonicalFilePath() == existing_file_info.canonicalFilePath())
        {
            _files_widget->setCurrentWidget(fw);
            return;
        }
    }
    std::vector<gta::header *> headers;
    std::vector<off_t> offsets;
    try
    {
        FILE *f = cio::open(filename, "r");
        while (cio::has_more(f, filename))
        {
            off_t offset = cio::tell(f, filename);
            gta::header *hdr = new gta::header;
            hdr->read_from(f);
            hdr->skip_data(f);
            headers.push_back(hdr);
            offsets.push_back(offset);
        }
        if (headers.size() == 0)
        {
            cio::close(f, filename);
            QMessageBox::critical(this, "Error", "File is empty");
        }
        else
        {
            FileWidget *fw = new FileWidget(f, filename, "", headers, offsets);
            connect(fw, SIGNAL(changed(const std::string &, const std::string &)),
                    this, SLOT(file_changed(const std::string &, const std::string &)));
            _files_widget->addTab(fw, QString(cio::to_sys(cio::basename(filename)).c_str()));
            _files_widget->tabBar()->setTabTextColor(_files_widget->indexOf(fw), "black");
            _files_widget->setCurrentWidget(fw);
        }
    }
    catch (std::exception &e)
    {
        for (size_t i = 0; i < headers.size(); i++)
        {
            delete headers[i];
        }
        QMessageBox::critical(this, "Error", e.what());
    }
}

void GUI::file_open()
{
    QFileDialog *file_dialog = new QFileDialog(this);
    file_dialog->setWindowTitle(tr("Open"));
    file_dialog->setAcceptMode(QFileDialog::AcceptOpen);
    file_dialog->setFileMode(QFileDialog::ExistingFiles);
    if (_last_file_open_dir.exists())
    {
        file_dialog->setDirectory(_last_file_open_dir);
    }
    QStringList filters;
    filters << tr("GTA files (*.gta)") << tr("All files (*)");
    file_dialog->setFilters(filters);
    if (!file_dialog->exec())
    {
        return;
    }
    QStringList file_names = file_dialog->selectedFiles();
    file_names.sort();
    for (int i = 0; i < file_names.size(); i++)
    {
        open(qPrintable(file_names[i]));
    }
    _last_file_open_dir = file_dialog->directory();
}

void GUI::file_save()
{
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    if (!fw->is_changed())
    {
        return;
    }
    if (fw->name().length() == 0)
    {
        file_save_as();
        return;
    }
    try
    {
        cio::rewind(fw->file(), fw->name());
        FILE *fo = cio::open(fw->name() + ".tmp", "w");
        for (size_t i = 0; i < fw->headers().size(); i++)
        {
            gta::header dummy_header;
            dummy_header.read_from(fw->file());
            fw->headers()[i]->write_to(fo);
            dummy_header.copy_data(fw->file(), *(fw->headers()[i]), fo);
        }
#if W32
        /* Windows is too stupid to do this right */
        cio::close(fw->file(), fw->name());
        cio::close(fo, fw->name() + ".tmp");
        cio::remove(fw->name());
        cio::rename(fw->name() + ".tmp", fw->name());
        fo = cio::open(fw->name(), "r");
#else
        cio::rename(fw->name() + ".tmp", fw->name());
        cio::close(fw->file(), fw->name());
#endif
        fw->saved(fo);
        _files_widget->tabBar()->setTabTextColor(_files_widget->indexOf(fw), "black");
    }
    catch (std::exception &e)
    {
        QMessageBox::critical(this, "Error", (std::string("Cannot save file: ") + e.what()).c_str());
    }
}

void GUI::file_save_as()
{
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    QDir file_dialog_dir;
    if (fw->name().length() == 0)
    {
        file_dialog_dir = QDir(QFileInfo(cio::to_sys(fw->name()).c_str()).absolutePath());
    }
    else
    {
        file_dialog_dir = _last_file_save_as_dir;
    }
    QFileDialog *file_dialog = new QFileDialog(this);
    file_dialog->setWindowTitle(tr("Save as"));
    file_dialog->setAcceptMode(QFileDialog::AcceptSave);
    file_dialog->setFileMode(QFileDialog::AnyFile);
    file_dialog->setDefaultSuffix("gta");
    if (file_dialog_dir.exists())
    {
        file_dialog->setDirectory(file_dialog_dir);
    }
    QStringList filters;
    filters << tr("GTA files (*.gta)") << tr("All files (*)");
    file_dialog->setFilters(filters);
    if (!file_dialog->exec())
    {
        return;
    }
    QString file_name = file_dialog->selectedFiles().at(0);
    QFileInfo file_info(file_name);
    for (int i = 0; i < _files_widget->count(); i++)
    {
        FileWidget *existing_fw = reinterpret_cast<FileWidget *>(_files_widget->widget(i));
        QFileInfo existing_file_info(cio::to_sys(existing_fw->name()).c_str());
        if (file_info.canonicalFilePath() == existing_file_info.canonicalFilePath())
        {
            QMessageBox::critical(this, "Error", "This file is currently opened. Close it first.");
            return;
        }
    }
    fw->set_name(cio::from_sys(qPrintable(file_name)));
    _files_widget->setTabText(_files_widget->indexOf(fw), QString(cio::to_sys(cio::basename(fw->name())).c_str()));
    file_save();
    _last_file_save_as_dir = file_dialog->directory();
}

void GUI::file_save_all()
{
    int old_index = _files_widget->currentIndex();
    for (int i = 0; i < _files_widget->count(); i++)
    {
        _files_widget->setCurrentIndex(i);
        file_save();
    }
    _files_widget->setCurrentIndex(old_index);
}

void GUI::file_close()
{
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    if (fw->is_changed())
    {
        if (QMessageBox::question(this, "Close file", "File is not saved. Close anyway?",
                    QMessageBox::Close | QMessageBox::Cancel, QMessageBox::Cancel)
                != QMessageBox::Close)
        {
            return;
        }
    }
    _files_widget->removeTab(_files_widget->indexOf(fw));
}

void GUI::file_close_all()
{
    for (int i = 0; i < _files_widget->count(); i++)
    {
        FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->widget(i));
        if (fw->is_changed())
        {
            if (QMessageBox::question(this, "Close all files", "Some files are not saved. Close anyway?",
                        QMessageBox::Close | QMessageBox::Cancel, QMessageBox::Cancel)
                    != QMessageBox::Close)
            {
                return;
            }
            break;
        }
    }
    while (_files_widget->count() > 0)
    {
        _files_widget->removeTab(0);
    }
}

void GUI::help_about()
{
    QMessageBox::about(this, tr("About " PACKAGE_NAME), tr(
                "<p>This is %1 version %2, using libgta version %3.</p>"
                "<p>See <a href=\"%4\">%5</a> for more information on this software.</p>"
                "<p>Copyright (C) 2010 Martin Lambers.<br>"
                "This is free software. You may redistribute copies of it under the terms of "
                "the <a href=\"http://www.gnu.org/licenses/gpl.html\">GNU General Public License</a>."
                "There is NO WARRANTY, to the extent permitted by law.</p>")
            .arg(PACKAGE_NAME).arg(VERSION).arg(gta::version()).arg(PACKAGE_URL).arg(PACKAGE_URL));
}

extern "C" int gtatool_gui(int argc, char *argv[])
{
    /* Let Qt handle the command line first, so that Qt options work */
    QApplication *app = new QApplication(argc, argv);
    /* Now handle our own command line options / arguments */
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_gui_help();
        return 0;
    }
    /* Run the GUI */
    int retval = 0;
    try
    {
        GUI *gui = new GUI();
        gui->show();
        for (size_t i = 0; i < arguments.size(); i++)
        {
            gui->open(cio::from_sys(arguments[i]));
        }
        retval = app->exec();
        delete gui;
        delete app;
    }
    catch (std::exception &e)
    {
        msg::err("GUI failure: %s", e.what());
        retval = 1;
    }
    catch (...)
    {
        msg::err("GUI failure");
        retval = 1;
    }
    return retval;
}
