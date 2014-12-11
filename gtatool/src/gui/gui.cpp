/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014
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

#include <string>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#if W32
#   define WIN32_LEAN_AND_MEAN
#   define _WIN32_WINNT 0x0502
#   include <windows.h>
#endif

#include <QtPlugin>
#include <QApplication>
#include <QMainWindow>
#include <QSettings>
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
#include <QThread>
#include <QLineEdit>
#include <QRadioButton>
#include <QTextCodec>
#include <QCheckBox>
#include <QDesktopServices>
#include <QUrl>

#include <gta/gta.hpp>

#include "base/msg.h"
#include "base/opt.h"
#include "base/fio.h"
#include "base/str.h"
#include "base/chk.h"

#include "lib.h"
#include "cmds.h"
#include "gui.hpp"

#include "viewwidget.hpp"


extern "C" void gtatool_gui_help(void)
{
    msg::req_txt(
            "gui [<files...>]\n"
            "\n"
            "Starts a graphical user interface (GUI) and opens the given GTA files, if any.");
}

// View initialization
static QSettings* global_settings = NULL;
#if DYNAMIC_MODULES || !WITH_GLEWMX
static ViewWidget* (*gtatool_view_create)(void) = NULL;
#else
extern "C" ViewWidget* gtatool_view_create(void);
#endif

// Helper functions: convert path names between our representation and Qt's representation
static QString to_qt(const std::string& path)
{
    return QTextCodec::codecForLocale()->toUnicode(fio::to_sys(path).c_str());
}
static std::string from_qt(const QString& path)
{
    return fio::from_sys(qPrintable(path));
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
#if QT_VERSION < 0x050000
    _tablewidget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#else
    _tablewidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#endif
    _tablewidget->horizontalHeader()->hide();
    _tablewidget->verticalHeader()->hide();
    connect(_tablewidget, SIGNAL(itemSelectionChanged()), this, SLOT(selection_changed()));
    connect(_tablewidget, SIGNAL(cellChanged(int, int)), this, SLOT(cell_changed(int, int)));
    _remove_button = new QPushButton("Remove selected tags");
    _remove_button->setEnabled(false);
    connect(_remove_button, SIGNAL(pressed()), this, SLOT(remove()));
    _add_button = new QPushButton("Add tag");
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
            std::string new_name = _tablewidget->item(row, 0)->text().toUtf8().constData();
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
                std::string new_value = _tablewidget->item(row, 1)->text().toUtf8().constData();
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
            std::string new_value = _tablewidget->item(row, column)->text().toUtf8().constData();
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

ArrayWidget::ArrayWidget(size_t index, gta::header *header, QWidget *parent)
    : QWidget(parent), _index(index), _header(header)
{
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(new QLabel("Dimensions:"), 0, 0, 1, 1);
    _dimensions_ledt = new QLineEdit("");
    _dimensions_ledt->setReadOnly(true);
    layout->addWidget(_dimensions_ledt, 0, 1, 1, 3);
    layout->addWidget(new QLabel("Components:"), 1, 0, 1, 1);
    _components_ledt = new QLineEdit("");
    _components_ledt->setReadOnly(true);
    layout->addWidget(_components_ledt, 1, 1, 1, 3);
    layout->addWidget(new QLabel("Size:"), 2, 0, 1, 1);
    _size_ledt = new QLineEdit("");
    _size_ledt->setReadOnly(true);
    layout->addWidget(_size_ledt, 2, 1, 1, 3);
    layout->addWidget(new QLabel("Compression:"), 3, 0, 1, 1);
    _compression_combobox = new QComboBox();
    _compression_combobox->setEditable(false);
    // the order of entries corresponds to the gta_compression_t enumeration
    _compression_combobox->addItem("none");
    _compression_combobox->addItem("Zlib default level");
    _compression_combobox->addItem("Bzip2");
    _compression_combobox->addItem("XZ");
    _compression_combobox->addItem("Zlib level 1");
    _compression_combobox->addItem("Zlib level 2");
    _compression_combobox->addItem("Zlib level 3");
    _compression_combobox->addItem("Zlib level 4");
    _compression_combobox->addItem("Zlib level 5");
    _compression_combobox->addItem("Zlib level 6");
    _compression_combobox->addItem("Zlib level 7");
    _compression_combobox->addItem("Zlib level 8");
    _compression_combobox->addItem("Zlib level 9");
    _compression_combobox->setCurrentIndex(header->compression());
    connect(_compression_combobox, SIGNAL(activated(int)), this, SLOT(compression_changed(int)));
    layout->addWidget(_compression_combobox, 3, 1, 1, 2);
    _taglists_widget = new MyTabWidget;
    layout->addWidget(_taglists_widget, 4, 0, 1, 4);
    update();
    layout->setRowStretch(4, 1);
    layout->setColumnStretch(3, 1);
    setLayout(layout);
}

ArrayWidget::~ArrayWidget()
{
}

void ArrayWidget::compression_changed(int index)
{
    if (index != static_cast<int>(_header->compression()))
    {
        _header->set_compression(static_cast<gta::compression>(index));
        emit changed(_index);
    }
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
    emit changed(_index);
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
    if (_header->dimensions() == 0)
    {
        dimensions_string += "0 (empty)";
    }
    else if (_header->dimensions() == 1)
    {
        dimensions_string += " elements";
    }
    else
    {
        dimensions_string += " (";
        dimensions_string += str::from(_header->elements());
        dimensions_string += " elements)";
    }
    _dimensions_ledt->setText(dimensions_string.c_str());
    _dimensions_ledt->setCursorPosition(0);
    std::string components_string;
    for (uintmax_t i = 0; i < _header->components(); i++)
    {
        components_string += type_to_string(_header->component_type(i), _header->component_size(i));
        if (i < _header->components() - 1)
        {
            components_string += ", ";
        }
    }
    if (_header->components() == 0)
    {
        components_string += "none";
    }
    _components_ledt->setText(components_string.c_str());
    _components_ledt->setCursorPosition(0);
    std::string size_string = str::from(_header->data_size()) + " bytes";
    if (_header->data_size() >= 1024)
    {
        size_string += " (" + str::human_readable_memsize(_header->data_size()) + ")";
    }
    _size_ledt->setText(size_string.c_str());
    _size_ledt->setCursorPosition(0);
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

FileWidget::FileWidget(const std::string &file_name, const std::string &save_name,
        const std::vector<gta::header*>& headers, const std::vector<off_t>& offsets,
        QWidget *parent)
    : QWidget(parent),
    _file_name(file_name), _save_name(save_name), _is_changed(false),
    _headers(headers), _offsets(offsets), _array_changed(_headers.size(), false),
    _view_widget(NULL)
{
    _array_label = new QLabel("Array index:");
    _array_spinbox = new QSpinBox;
    _array_spinbox->setRange(0, checked_cast<int>(_headers.size() - 1));
    _array_spinbox->setValue(0);
    connect(_array_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update_array()));
    QGridLayout *l0 = new QGridLayout;
    l0->addWidget(_array_label, 0, 0);
    l0->addWidget(_array_spinbox, 0, 1);
    l0->addWidget(new QLabel(QString("(Total: ") + QString::number(_headers.size()) + QString(")")), 0, 2);
    _view_button = new QPushButton("View");
    _view_button->setEnabled(cmd_is_available(cmd_find("view")));
    connect(_view_button, SIGNAL(clicked()), this, SLOT(open_view()));
    l0->addWidget(_view_button, 0, 4);
    l0->addItem(new QSpacerItem(0, _array_label->minimumSizeHint().height() / 3 * 2,
                QSizePolicy::Minimum, QSizePolicy::Fixed), 1, 0, 1, 4);
    l0->setColumnStretch(3, 1);
    _array_widget_layout = new QGridLayout;
    _array_widget = NULL;
    update_array();
    QGridLayout *layout = new QGridLayout;
    layout->addLayout(l0, 0, 0);
    layout->addLayout(_array_widget_layout, 1, 0);
    layout->setRowStretch(1, 1);
    setLayout(layout);
}

FileWidget::~FileWidget()
{
    if (_view_widget) {
        _view_widget->close();
        //delete _view_widget;
    }
    if (_save_name.length() > 0 && _save_name.compare(_file_name) != 0) {
        try { fio::remove(_save_name); } catch (...) {}
    }
    for (size_t i = 0; i < _headers.size(); i++)
        delete _headers[i];
}

void FileWidget::update_label()
{
    size_t index = _array_spinbox->value();
    QPalette p = _array_label->palette();
    p.setColor(_array_label->foregroundRole(),
            _array_changed[index] ? QColor("red") : QColor("black"));
    _array_label->setPalette(p);
}

void FileWidget::update_array()
{
    size_t index = _array_spinbox->value();
    if (_array_widget) {
        _array_widget_layout->removeWidget(_array_widget);
        delete _array_widget;
    }
    _array_widget = new ArrayWidget(index, _headers[index]);
    connect(_array_widget, SIGNAL(changed(size_t)), this, SLOT(array_changed(size_t)));
    _array_widget->layout()->setContentsMargins(0, 0, 0, 0);
    _array_widget_layout->addWidget(_array_widget, 0, 0);
    update_label();
    if (_view_widget && !_view_widget->isHidden())
        _view_widget->set_current(index);
}

void FileWidget::array_changed(size_t index)
{
    _array_changed[index] = true;
    update_label();
    _is_changed = true;
    emit changed(_file_name, _save_name);
}

void FileWidget::set_file_name(const std::string &file_name)
{
    _file_name = file_name;
}

void FileWidget::saved_to(const std::string &save_name)
{
    if (_save_name.length() > 0 && _save_name.compare(_file_name) != 0)
    {
        try { fio::remove(_save_name); } catch (...) {}
    }
    _save_name = save_name;
    _is_changed = false;
    if (is_saved())
    {
        _array_widget->saved();
        for (size_t i = 0; i < _headers.size(); i++)
            _array_changed[i] = false;
        update_label();
    }
}

void FileWidget::open_view()
{
#if DYNAMIC_MODULES
    if (!gtatool_view_create) {
        int i = cmd_find("view");
        cmd_open(i);
        gtatool_view_create = reinterpret_cast<ViewWidget* (*)(void)>(
                cmd_symbol(i, "gtatool_view_create"));
    }
#endif
    if (_view_widget && !_view_widget->isHidden()) {
        _view_widget->raise();
    } else {
        delete _view_widget;
        _view_widget = NULL;
    }
    if (!_view_widget) {
        _view_widget = gtatool_view_create();
        connect(_view_widget, SIGNAL(closed()), this, SLOT(view_closed()));
        connect(_view_widget, SIGNAL(quit()), this, SLOT(request_quit()));
        _view_widget->init(gtatool_argc, gtatool_argv, global_settings,
                _file_name, _save_name, _headers, _offsets);
    }
    _view_widget->set_current(_array_spinbox->value());
    _view_button->setText("Update view");
}

void FileWidget::view_closed()
{
    _view_button->setText("View");
}

void FileWidget::request_quit()
{
    emit quit();
}

GUI::GUI()
{
    setWindowTitle(PACKAGE_NAME);
    setWindowIcon(QIcon(":gui.png"));
    QWidget *widget = new QWidget;
    QGridLayout *layout = new QGridLayout;
    _files_widget = new MyTabWidget;
    _files_widget->setTabsClosable(true);
    _files_widget->setMovable(true);
    connect(_files_widget, SIGNAL(tabCloseRequested(int)), this, SLOT(tab_close(int)));
    layout->addWidget(_files_widget, 0, 0);
    layout->setRowStretch(0, 1);
    layout->setColumnStretch(0, 1);
    widget->setLayout(layout);
    setCentralWidget(widget);

    QMenu *file_menu = menuBar()->addMenu(tr("&File"));
    QAction *file_open_action = new QAction(tr("&Open..."), this);
    file_open_action->setShortcut(QKeySequence::Open);
    connect(file_open_action, SIGNAL(triggered()), this, SLOT(file_open()));
    file_menu->addAction(file_open_action);
    QAction *file_save_action = new QAction(tr("&Save"), this);
    file_save_action->setShortcut(QKeySequence::Save);
    connect(file_save_action, SIGNAL(triggered()), this, SLOT(file_save()));
    file_menu->addAction(file_save_action);
    QAction *file_save_as_action = new QAction(tr("Save &as..."), this);
    connect(file_save_as_action, SIGNAL(triggered()), this, SLOT(file_save_as()));
    file_menu->addAction(file_save_as_action);
    QAction *file_save_all_action = new QAction(tr("Save all"), this);
    connect(file_save_all_action, SIGNAL(triggered()), this, SLOT(file_save_all()));
    file_menu->addAction(file_save_all_action);
    QAction *file_close_action = new QAction(tr("&Close"), this);
    file_close_action->setShortcut(QKeySequence::Close);
    connect(file_close_action, SIGNAL(triggered()), this, SLOT(file_close()));
    file_menu->addAction(file_close_action);
    QAction *file_close_all_action = new QAction(tr("Close all"), this);
    connect(file_close_all_action, SIGNAL(triggered()), this, SLOT(file_close_all()));
    file_menu->addAction(file_close_all_action);
    file_menu->addSeparator();
    QAction *file_import_action = new QAction(tr("Automatic &import..."), this);
    file_import_action->setShortcut(tr("Ctrl+I"));
    connect(file_import_action, SIGNAL(triggered()), this, SLOT(file_import()));
    file_menu->addAction(file_import_action);
    QMenu *file_import_menu = file_menu->addMenu(tr("Manual import"));
    QAction *file_import_csv_action = new QAction(tr("CSV data..."), this);
    connect(file_import_csv_action, SIGNAL(triggered()), this, SLOT(file_import_csv()));
    file_import_csv_action->setEnabled(cmd_is_available(cmd_find("from-csv")));
    file_import_menu->addAction(file_import_csv_action);
    QAction *file_import_datraw_action = new QAction(tr("Volume data in .dat/.raw format..."), this);
    connect(file_import_datraw_action, SIGNAL(triggered()), this, SLOT(file_import_datraw()));
    file_import_datraw_action->setEnabled(cmd_is_available(cmd_find("from-datraw")));
    file_import_menu->addAction(file_import_datraw_action);
    QAction *file_import_dcmtk_action = new QAction(tr("DICOM files (via DCMTK)..."), this);
    connect(file_import_dcmtk_action, SIGNAL(triggered()), this, SLOT(file_import_dcmtk()));
    file_import_dcmtk_action->setEnabled(cmd_is_available(cmd_find("from-dcmtk")));
    file_import_menu->addAction(file_import_dcmtk_action);
    QAction *file_import_exr_action = new QAction(tr("EXR HDR images (via OpenEXR)..."), this);
    connect(file_import_exr_action, SIGNAL(triggered()), this, SLOT(file_import_exr()));
    file_import_exr_action->setEnabled(cmd_is_available(cmd_find("from-exr")));
    file_import_menu->addAction(file_import_exr_action);
    QAction *file_import_ffmpeg_action = new QAction(tr("Multimedia data (via FFmpeg)..."), this);
    connect(file_import_ffmpeg_action, SIGNAL(triggered()), this, SLOT(file_import_ffmpeg()));
    file_import_ffmpeg_action->setEnabled(cmd_is_available(cmd_find("from-ffmpeg")));
    file_import_menu->addAction(file_import_ffmpeg_action);
    QAction *file_import_gdal_action = new QAction(tr("Remote Sensing data (via GDAL)..."), this);
    connect(file_import_gdal_action, SIGNAL(triggered()), this, SLOT(file_import_gdal()));
    file_import_gdal_action->setEnabled(cmd_is_available(cmd_find("from-gdal")));
    file_import_menu->addAction(file_import_gdal_action);
    QAction *file_import_jpeg_action = new QAction(tr("JPEG images (via libjpeg)..."), this);
    connect(file_import_jpeg_action, SIGNAL(triggered()), this, SLOT(file_import_jpeg()));
    file_import_jpeg_action->setEnabled(cmd_is_available(cmd_find("from-jpeg")));
    file_import_menu->addAction(file_import_jpeg_action);
    QAction *file_import_magick_action = new QAction(tr("Image data (via " MAGICK_FLAVOR ")..."), this);
    connect(file_import_magick_action, SIGNAL(triggered()), this, SLOT(file_import_magick()));
    file_import_magick_action->setEnabled(cmd_is_available(cmd_find("from-magick")));
    file_import_menu->addAction(file_import_magick_action);
    QAction *file_import_mat_action = new QAction(tr("MATLAB data (via matio)..."), this);
    connect(file_import_mat_action, SIGNAL(triggered()), this, SLOT(file_import_mat()));
    file_import_mat_action->setEnabled(cmd_is_available(cmd_find("from-mat")));
    file_import_menu->addAction(file_import_mat_action);
    QAction *file_import_netcdf_action = new QAction(tr("NetCDF data (via NetCDF)..."), this);
    connect(file_import_netcdf_action, SIGNAL(triggered()), this, SLOT(file_import_netcdf()));
    file_import_netcdf_action->setEnabled(cmd_is_available(cmd_find("from-netcdf")));
    file_import_menu->addAction(file_import_netcdf_action);
    QAction *file_import_pcd_action = new QAction(tr("PCD point cloud data (via PCL)..."), this);
    connect(file_import_pcd_action, SIGNAL(triggered()), this, SLOT(file_import_pcd()));
    file_import_pcd_action->setEnabled(cmd_is_available(cmd_find("from-pcd")));
    file_import_menu->addAction(file_import_pcd_action);
    QAction *file_import_pfs_action = new QAction(tr("PFS floating point data (via PFS)..."), this);
    connect(file_import_pfs_action, SIGNAL(triggered()), this, SLOT(file_import_pfs()));
    file_import_pfs_action->setEnabled(cmd_is_available(cmd_find("from-pfs")));
    file_import_menu->addAction(file_import_pfs_action);
    QAction *file_import_ply_action = new QAction(tr("PLY geometry data..."), this);
    connect(file_import_ply_action, SIGNAL(triggered()), this, SLOT(file_import_ply()));
    file_import_ply_action->setEnabled(cmd_is_available(cmd_find("from-ply")));
    file_import_menu->addAction(file_import_ply_action);
    QAction *file_import_png_action = new QAction(tr("PNG image data..."), this);
    connect(file_import_png_action, SIGNAL(triggered()), this, SLOT(file_import_png()));
    file_import_png_action->setEnabled(cmd_is_available(cmd_find("from-png")));
    file_import_menu->addAction(file_import_png_action);
    QAction *file_import_pvm_action = new QAction(tr("PVM volume data..."), this);
    connect(file_import_pvm_action, SIGNAL(triggered()), this, SLOT(file_import_pvm()));
    file_import_pvm_action->setEnabled(cmd_is_available(cmd_find("from-pvm")));
    file_import_menu->addAction(file_import_pvm_action);
    QAction *file_import_rat_action = new QAction(tr("RAT RadarTools data..."), this);
    connect(file_import_rat_action, SIGNAL(triggered()), this, SLOT(file_import_rat()));
    file_import_rat_action->setEnabled(cmd_is_available(cmd_find("from-rat")));
    file_import_menu->addAction(file_import_rat_action);
    QAction *file_import_raw_action = new QAction(tr("Raw data..."), this);
    connect(file_import_raw_action, SIGNAL(triggered()), this, SLOT(file_import_raw()));
    file_import_raw_action->setEnabled(cmd_is_available(cmd_find("from-raw")));
    file_import_menu->addAction(file_import_raw_action);
    QAction *file_import_sndfile_action = new QAction(tr("Audio data (via sndfile)..."), this);
    connect(file_import_sndfile_action, SIGNAL(triggered()), this, SLOT(file_import_sndfile()));
    file_import_sndfile_action->setEnabled(cmd_is_available(cmd_find("from-sndfile")));
    file_import_menu->addAction(file_import_sndfile_action);
    QAction *file_import_teem_action = new QAction(tr("NRRD data (via teem)..."), this);
    connect(file_import_teem_action, SIGNAL(triggered()), this, SLOT(file_import_teem()));
    file_import_teem_action->setEnabled(cmd_is_available(cmd_find("from-teem")));
    file_import_menu->addAction(file_import_teem_action);
    QAction *file_export_action = new QAction(tr("Automatic &export..."), this);
    file_export_action->setShortcut(tr("Ctrl+E"));
    connect(file_export_action, SIGNAL(triggered()), this, SLOT(file_export()));
    file_menu->addAction(file_export_action);
    QMenu *file_export_menu = file_menu->addMenu(tr("Manual export"));
    QAction *file_export_csv_action = new QAction(tr("CSV data..."), this);
    connect(file_export_csv_action, SIGNAL(triggered()), this, SLOT(file_export_csv()));
    file_export_csv_action->setEnabled(cmd_is_available(cmd_find("to-csv")));
    file_export_menu->addAction(file_export_csv_action);
    QAction *file_export_datraw_action = new QAction(tr("Volume data in .dat/.raw format..."), this);
    connect(file_export_datraw_action, SIGNAL(triggered()), this, SLOT(file_export_datraw()));
    file_export_datraw_action->setEnabled(cmd_is_available(cmd_find("to-datraw")));
    file_export_menu->addAction(file_export_datraw_action);
    QAction *file_export_exr_action = new QAction(tr("EXR HDR images (via EXR)..."), this);
    connect(file_export_exr_action, SIGNAL(triggered()), this, SLOT(file_export_exr()));
    file_export_exr_action->setEnabled(cmd_is_available(cmd_find("to-exr")));
    file_export_menu->addAction(file_export_exr_action);
    QAction *file_export_gdal_action = new QAction(tr("Remote Sensing data (via GDAL)..."), this);
    connect(file_export_gdal_action, SIGNAL(triggered()), this, SLOT(file_export_gdal()));
    file_export_gdal_action->setEnabled(cmd_is_available(cmd_find("to-gdal")));
    file_export_menu->addAction(file_export_gdal_action);
    QAction *file_export_jpeg_action = new QAction(tr("JPEG images (via libjpeg)..."), this);
    connect(file_export_jpeg_action, SIGNAL(triggered()), this, SLOT(file_export_jpeg()));
    file_export_jpeg_action->setEnabled(cmd_is_available(cmd_find("to-jpeg")));
    file_export_menu->addAction(file_export_jpeg_action);
    QAction *file_export_magick_action = new QAction(tr("Image data (via " MAGICK_FLAVOR ")..."), this);
    connect(file_export_magick_action, SIGNAL(triggered()), this, SLOT(file_export_magick()));
    file_export_magick_action->setEnabled(cmd_is_available(cmd_find("to-magick")));
    file_export_menu->addAction(file_export_magick_action);
    QAction *file_export_mat_action = new QAction(tr("MATLAB data (via matio)..."), this);
    connect(file_export_mat_action, SIGNAL(triggered()), this, SLOT(file_export_mat()));
    file_export_mat_action->setEnabled(cmd_is_available(cmd_find("to-mat")));
    file_export_menu->addAction(file_export_mat_action);
    QAction *file_export_netcdf_action = new QAction(tr("NetCDF data (via NetCDF)..."), this);
    connect(file_export_netcdf_action, SIGNAL(triggered()), this, SLOT(file_export_netcdf()));
    file_export_netcdf_action->setEnabled(cmd_is_available(cmd_find("to-netcdf")));
    file_export_menu->addAction(file_export_netcdf_action);
    QAction *file_export_pcd_action = new QAction(tr("PCD point cloud data (via PCL)..."), this);
    connect(file_export_pcd_action, SIGNAL(triggered()), this, SLOT(file_export_pcd()));
    file_export_pcd_action->setEnabled(cmd_is_available(cmd_find("to-pcd")));
    file_export_menu->addAction(file_export_pcd_action);
    QAction *file_export_pfs_action = new QAction(tr("PFS floating point data (via PFS)..."), this);
    connect(file_export_pfs_action, SIGNAL(triggered()), this, SLOT(file_export_pfs()));
    file_export_pfs_action->setEnabled(cmd_is_available(cmd_find("to-pfs")));
    file_export_menu->addAction(file_export_pfs_action);
    QAction *file_export_ply_action = new QAction(tr("PLY geometry data..."), this);
    connect(file_export_ply_action, SIGNAL(triggered()), this, SLOT(file_export_ply()));
    file_export_ply_action->setEnabled(cmd_is_available(cmd_find("to-ply")));
    file_export_menu->addAction(file_export_ply_action);
    QAction *file_export_png_action = new QAction(tr("PVM volume data..."), this);
    connect(file_export_png_action, SIGNAL(triggered()), this, SLOT(file_export_png()));
    file_export_png_action->setEnabled(cmd_is_available(cmd_find("to-png")));
    file_export_menu->addAction(file_export_png_action);
    QAction *file_export_pvm_action = new QAction(tr("PVM volume data..."), this);
    connect(file_export_pvm_action, SIGNAL(triggered()), this, SLOT(file_export_pvm()));
    file_export_pvm_action->setEnabled(cmd_is_available(cmd_find("to-pvm")));
    file_export_menu->addAction(file_export_pvm_action);
    QAction *file_export_rat_action = new QAction(tr("RAT RadarTools data..."), this);
    connect(file_export_rat_action, SIGNAL(triggered()), this, SLOT(file_export_rat()));
    file_export_rat_action->setEnabled(cmd_is_available(cmd_find("to-rat")));
    file_export_menu->addAction(file_export_rat_action);
    QAction *file_export_raw_action = new QAction(tr("Raw data..."), this);
    connect(file_export_raw_action, SIGNAL(triggered()), this, SLOT(file_export_raw()));
    file_export_raw_action->setEnabled(cmd_is_available(cmd_find("to-raw")));
    file_export_menu->addAction(file_export_raw_action);
    QAction *file_export_sndfile_action = new QAction(tr("WAV audio (via sndfile)..."), this);
    connect(file_export_sndfile_action, SIGNAL(triggered()), this, SLOT(file_export_sndfile()));
    file_export_sndfile_action->setEnabled(cmd_is_available(cmd_find("to-sndfile")));
    file_export_menu->addAction(file_export_sndfile_action);
    QAction *file_export_teem_action = new QAction(tr("NRRD data (via teem)..."), this);
    connect(file_export_teem_action, SIGNAL(triggered()), this, SLOT(file_export_teem()));
    file_export_teem_action->setEnabled(cmd_is_available(cmd_find("to-teem")));
    file_export_menu->addAction(file_export_teem_action);
    file_menu->addSeparator();
    QAction *quit_action = new QAction(tr("&Quit"), this);
    quit_action->setShortcut(tr("Ctrl+Q")); // QKeySequence::Quit is not reliable
    connect(quit_action, SIGNAL(triggered()), this, SLOT(close()));
    file_menu->addAction(quit_action);

    QMenu *stream_menu = menuBar()->addMenu(tr("&Stream"));
    QAction *stream_extract_action = new QAction(tr("&Extract current array..."), this);
    connect(stream_extract_action, SIGNAL(triggered()), this, SLOT(stream_extract()));
    stream_menu->addAction(stream_extract_action);
    QAction *stream_split_action = new QAction(tr("&Split current file..."), this);
    connect(stream_split_action, SIGNAL(triggered()), this, SLOT(stream_split()));
    stream_menu->addAction(stream_split_action);
    QAction *stream_merge_action = new QAction(tr("&Merge open files..."), this);
    connect(stream_merge_action, SIGNAL(triggered()), this, SLOT(stream_merge()));
    stream_menu->addAction(stream_merge_action);
    QAction *stream_foreach_action = new QAction(tr("&Run command for each array in current file..."), this);
    connect(stream_foreach_action, SIGNAL(triggered()), this, SLOT(stream_foreach()));
    stream_menu->addAction(stream_foreach_action);
    QAction *stream_grep_action = new QAction(tr("Select specific arrays from current file (&grep)..."), this);
    connect(stream_grep_action, SIGNAL(triggered()), this, SLOT(stream_grep()));
    stream_menu->addAction(stream_grep_action);

    QMenu *array_menu = menuBar()->addMenu(tr("&Arrays"));
    QAction *array_create_action = new QAction(tr("&Create array..."), this);
    connect(array_create_action, SIGNAL(triggered()), this, SLOT(array_create()));
    array_menu->addAction(array_create_action);
    QAction *array_resize_action = new QAction(tr("&Resize arrays..."), this);
    connect(array_resize_action, SIGNAL(triggered()), this, SLOT(array_resize()));
    array_menu->addAction(array_resize_action);
    QAction *array_fill_action = new QAction(tr("&Fill sub-arrays..."), this);
    connect(array_fill_action, SIGNAL(triggered()), this, SLOT(array_fill()));
    array_menu->addAction(array_fill_action);
    QAction *array_extract_action = new QAction(tr("&Extract sub-arrays..."), this);
    connect(array_extract_action, SIGNAL(triggered()), this, SLOT(array_extract()));
    array_menu->addAction(array_extract_action);
    QAction *array_set_action = new QAction(tr("&Set sub-arrays from other arrays..."), this);
    connect(array_set_action, SIGNAL(triggered()), this, SLOT(array_set()));
    array_menu->addAction(array_set_action);
    QAction *array_merge_action = new QAction(tr("&Merge arrays from open files..."), this);
    connect(array_merge_action, SIGNAL(triggered()), this, SLOT(array_merge()));
    array_menu->addAction(array_merge_action);
    QAction *array_diff_action = new QAction(tr("Compute &difference of two open files..."), this);
    connect(array_diff_action, SIGNAL(triggered()), this, SLOT(array_diff()));
    array_menu->addAction(array_diff_action);
    QAction *array_combine_action = new QAction(tr("&Combine arrays from open files..."), this);
    connect(array_combine_action, SIGNAL(triggered()), this, SLOT(array_combine()));
    array_menu->addAction(array_combine_action);

    QMenu *dimension_menu = menuBar()->addMenu(tr("&Dimensions"));
    QAction *dimension_add_action = new QAction(tr("&Add dimension to current array..."), this);
    connect(dimension_add_action, SIGNAL(triggered()), this, SLOT(dimension_add()));
    dimension_menu->addAction(dimension_add_action);
    QAction *dimension_extract_action = new QAction(tr("&Extract dimension from current array..."), this);
    connect(dimension_extract_action, SIGNAL(triggered()), this, SLOT(dimension_extract()));
    dimension_menu->addAction(dimension_extract_action);
    QAction *dimension_reverse_action = new QAction(tr("Re&verse current array in one dimension..."), this);
    connect(dimension_reverse_action, SIGNAL(triggered()), this, SLOT(dimension_reverse()));
    dimension_menu->addAction(dimension_reverse_action);
    QAction *dimension_reorder_action = new QAction(tr("&Reorder dimensions of current array..."), this);
    connect(dimension_reorder_action, SIGNAL(triggered()), this, SLOT(dimension_reorder()));
    dimension_menu->addAction(dimension_reorder_action);
    QAction *dimension_merge_action = new QAction(tr("&Merge arrays from open files into new dimension..."), this);
    connect(dimension_merge_action, SIGNAL(triggered()), this, SLOT(dimension_merge()));
    dimension_menu->addAction(dimension_merge_action);
    QAction *dimension_split_action = new QAction(tr("&Split current array along one dimension..."), this);
    connect(dimension_split_action, SIGNAL(triggered()), this, SLOT(dimension_split()));
    dimension_menu->addAction(dimension_split_action);
    QAction *dimension_flatten_action = new QAction(tr("&Flatten dimensions of current array (make it one-dimensional)..."), this);
    connect(dimension_flatten_action, SIGNAL(triggered()), this, SLOT(dimension_flatten()));
    dimension_menu->addAction(dimension_flatten_action);

    QMenu *component_menu = menuBar()->addMenu(tr("&Components"));
    QAction *component_add_action = new QAction(tr("&Add components to current array..."), this);
    connect(component_add_action, SIGNAL(triggered()), this, SLOT(component_add()));
    component_menu->addAction(component_add_action);
    QAction *component_convert_action = new QAction(tr("&Convert component types of current array..."), this);
    connect(component_convert_action, SIGNAL(triggered()), this, SLOT(component_convert()));
    component_menu->addAction(component_convert_action);
    QAction *component_extract_action = new QAction(tr("&Extract components from current array..."), this);
    connect(component_extract_action, SIGNAL(triggered()), this, SLOT(component_extract()));
    component_menu->addAction(component_extract_action);
    QAction *component_reorder_action = new QAction(tr("&Reorder components of current array..."), this);
    connect(component_reorder_action, SIGNAL(triggered()), this, SLOT(component_reorder()));
    component_menu->addAction(component_reorder_action);
    QAction *component_set_action = new QAction(tr("&Set component values for current array..."), this);
    connect(component_set_action, SIGNAL(triggered()), this, SLOT(component_set()));
    component_menu->addAction(component_set_action);
    QAction *component_compute_action = new QAction(tr("Recompute component values for current array..."), this);
    connect(component_compute_action, SIGNAL(triggered()), this, SLOT(component_compute()));
    if (!cmd_is_available(cmd_find("component-compute")))
    {
        component_compute_action->setEnabled(false);
    }
    component_menu->addAction(component_compute_action);
    QAction *component_split_action = new QAction(tr("S&plit components of current array..."), this);
    connect(component_split_action, SIGNAL(triggered()), this, SLOT(component_split()));
    component_menu->addAction(component_split_action);
    QAction *component_merge_action = new QAction(tr("&Merge array components of open files..."), this);
    connect(component_merge_action, SIGNAL(triggered()), this, SLOT(component_merge()));
    component_menu->addAction(component_merge_action);

    QMenu *help_menu = menuBar()->addMenu(tr("&Help"));
    QAction *help_overview_act = new QAction(tr("&Overview"), this);
    connect(help_overview_act, SIGNAL(triggered()), this, SLOT(help_overview()));
    help_menu->addAction(help_overview_act);
    QAction *help_website_act = new QAction(tr("&Website..."), this);
    connect(help_website_act, SIGNAL(triggered()), this, SLOT(help_website()));
    help_menu->addAction(help_website_act);
    QAction *help_about_action = new QAction(tr("&About"), this);
    connect(help_about_action, SIGNAL(triggered()), this, SLOT(help_about()));
    help_menu->addAction(help_about_action);

    resize(menuBar()->sizeHint().width(), 200);

    restoreGeometry(global_settings->value("gui/windowgeometry").toByteArray());
    restoreState(global_settings->value("gui/windowstate").toByteArray());

    _files_watcher = new QFileSystemWatcher(this);
    connect(_files_watcher, SIGNAL(fileChanged(const QString&)), this, SLOT(file_changed_on_disk(const QString&)));
}

GUI::~GUI()
{
}

void GUI::closeEvent(QCloseEvent *event)
{
    file_close_all();
    if (_files_widget->count() == 0)
    {
        global_settings->setValue("gui/windowgeometry", saveGeometry());
        global_settings->setValue("gui/windowstate", saveState());
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

bool GUI::check_have_file()
{
    if (_files_widget->count() == 0)
    {
        QMessageBox::critical(this, "Error", "No files are opened.");
        return false;
    }
    return true;
}

bool GUI::check_file_unchanged()
{
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    if (!fw)
    {
        return false;
    }
    else if (fw->is_changed())
    {
        try
        {
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            FILE *fi = fio::open(fw->save_name(), "r");
            FILE *save_file;
            std::string save_name = fio::mktempfile(&save_file);
            for (size_t i = 0; i < fw->headers().size(); i++)
            {
                gta::header dummy_header;
                dummy_header.read_from(fi);
                fw->headers()[i]->write_to(save_file);
                dummy_header.copy_data(fi, *(fw->headers()[i]), save_file);
            }
            fio::close(save_file, save_name);
            fio::close(fi, fw->file_name());
            fw->saved_to(save_name);
            QApplication::restoreOverrideCursor();
        }
        catch (std::exception &e)
        {
            QApplication::restoreOverrideCursor();
            QMessageBox::critical(this, "Error", (std::string("Cannot write temporary GTA file: ") + e.what()).c_str());
            return false;
        }
    }
    return true;
}

bool GUI::check_all_files_unchanged()
{
    bool all_unchanged = true;
    int old_index = _files_widget->currentIndex();
    for (int i = 0; i < _files_widget->count(); i++)
    {
        _files_widget->setCurrentIndex(i);
        if (!check_file_unchanged())
        {
            all_unchanged = false;
            break;
        }
    }
    _files_widget->setCurrentIndex(old_index);
    return all_unchanged;
}

void GUI::file_changed(const std::string &file_name, const std::string &save_name)
{
    int file_index = 0;
    for (int i = 0; i < _files_widget->count(); i++)
    {
        FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->widget(i));
        if (fw->file_name().compare(file_name) == 0
                && fw->save_name().compare(save_name) == 0)
        {
            file_index = i;
            break;
        }
    }
    _files_widget->tabBar()->setTabTextColor(file_index, QColor("red"));
}

void GUI::file_changed_on_disk(const QString& fn)
{
    if (QFileInfo(fn).size() == 0) {
        // Ignore this notification: Most likely the file was just truncated
        // (e.g. by stdout redirection in the shell) and a subsequent
        // notification will tell use when the actual file update is complete.
        return;
    }
    std::string file_name = from_qt(fn);
    FileWidget* fw = NULL;
    int fi;
    for (fi = 0; fi < _files_widget->count(); fi++) {
        fw = reinterpret_cast<FileWidget *>(_files_widget->widget(fi));
        if (fw->file_name().compare(file_name) == 0)
            break;
    }
    _files_watcher->removePath(fn);
    bool changes_lost = !fw || !fw->is_saved();
    _files_widget->removeTab(fi);
    delete fw;
    open(file_name, file_name, fi);
    if (changes_lost) {
        QMessageBox::warning(this, "Warning", QString("File %1 was changed on disk. Changes are lost.").arg(fn));
    }
}

void GUI::tab_close(int index)
{
    int old_index = _files_widget->currentIndex();
    _files_widget->setCurrentIndex(index);
    file_close();
    if (index < old_index)
        old_index--;
    _files_widget->setCurrentIndex(old_index);
}

QStringList GUI::file_open_dialog(const QStringList &filters)
{
    QFileDialog *file_dialog = new QFileDialog(this);
    file_dialog->setWindowTitle(tr("Open"));
    file_dialog->setAcceptMode(QFileDialog::AcceptOpen);
    file_dialog->setFileMode(QFileDialog::ExistingFiles);
    QDir last_dir = QDir(global_settings->value("general/last-dir").toString());
    if (last_dir.exists())
        file_dialog->setDirectory(last_dir);
    QStringList complete_filters;
    complete_filters << filters << tr("All files (*)");
    file_dialog->setNameFilters(complete_filters);
    QStringList file_names;
    if (file_dialog->exec()) {
        file_names = file_dialog->selectedFiles();
        file_names.sort();
        global_settings->setValue("general/last-dir", file_dialog->directory().path());
    }
    return file_names;
}

QString GUI::file_save_dialog(const QString &default_suffix, const QStringList &filters, const QString &existing_name)
{
    QDir last_dir = QDir(global_settings->value("general/last-dir").toString());
    QDir file_dialog_dir;
    if (!existing_name.isEmpty())
        file_dialog_dir = QDir(QFileInfo(existing_name).absolutePath());
    else
        file_dialog_dir = last_dir;
    QFileDialog *file_dialog = new QFileDialog(this);
    file_dialog->setWindowTitle(tr("Save"));
    file_dialog->setAcceptMode(QFileDialog::AcceptSave);
    file_dialog->setFileMode(QFileDialog::AnyFile);
    if (!default_suffix.isEmpty())
        file_dialog->setDefaultSuffix(default_suffix);
    if (file_dialog_dir.exists())
        file_dialog->setDirectory(file_dialog_dir);
    QStringList complete_filters;
    complete_filters << filters;
    complete_filters << tr("All files (*)");
    file_dialog->setNameFilters(complete_filters);
    QString file_name;
    if (file_dialog->exec()) {
        file_name = file_dialog->selectedFiles().at(0);
        QFileInfo file_info(file_name);
        global_settings->setValue("general/last-dir", file_dialog->directory().path());
        for (int i = 0; i < _files_widget->count(); i++) {
            FileWidget *existing_fw = reinterpret_cast<FileWidget *>(_files_widget->widget(i));
            if (existing_fw->file_name().length() > 0) {
                QFileInfo existing_file_info(to_qt(existing_fw->file_name()));
                if (existing_file_info.canonicalFilePath().length() > 0
                        && file_info.canonicalFilePath() == existing_file_info.canonicalFilePath()) {
                    QMessageBox::critical(this, "Error", "This file is currently opened. Close it first.");
                    file_name = QString();
                    break;
                }
            }
        }
    }
    return file_name;
}

class CmdThread : public QThread
{
private:
    int _cmd_index;
    int _argc;
    char **_argv;

public:
    CmdThread(int cmd_index, int argc, char **argv)
        : _cmd_index(cmd_index), _argc(argc), _argv(argv)
    {
    }
    ~CmdThread()
    {
    }

    int retval;
    void run()
    {
        retval = cmd_run(_cmd_index, _argc, _argv);
    }
};

int GUI::run(const std::string &cmd, const std::vector<std::string> &args,
        std::string &std_err, FILE *std_out, FILE *std_in)
{
    /* prepare */
    std::vector<char *> argv;
    argv.push_back(::strdup(cmd.c_str()));
    for (size_t i = 0; i < args.size(); i++)
    {
        argv.push_back(::strdup(args[i].c_str()));
    }
    argv.push_back(NULL);
    for (size_t i = 0; i < argv.size() - 1; i++)
    {
        if (!argv[i])
        {
            for (size_t j = 0; j < i; j++)
            {
                ::free(argv[i]);
            }
            std_err = ::strerror(ENOMEM);
            return 1;
        }
    }
    /* save environment */
    FILE *std_err_bak = msg::file();
    FILE *std_out_bak = gtatool_stdout;
    FILE *std_in_bak = gtatool_stdin;
    std::string msg_prg_name_bak = msg::program_name();
    int msg_columns_bak = msg::columns();
    /* modify environment */
    FILE *std_err_tmp;
    try
    {
        std_err_tmp = fio::tempfile();
    }
    catch (std::exception &e)
    {
        std_err = e.what();
        for (size_t i = 0; i < argv.size() - 1; i++)
        {
            ::free(argv[i]);
        }
        return 1;
    }
    msg::set_file(std_err_tmp);
    if (std_out)
    {
        gtatool_stdout = std_out;
    }
    if (std_in)
    {
        gtatool_stdin = std_in;
    }
    msg::set_program_name("");
    msg::set_columns(80);
    /* run command */
    int cmd_index = cmd_find(cmd.c_str());
    cmd_open(cmd_index);
    std::string mbox_text = "<p>Running command</p><code>";
    mbox_text += cmd;
    /*
    mbox_text + " ";
    for (size_t i = 0; i < args.size(); i++)
    {
        mbox_text += args[i] + " ";
    }
    */
    mbox_text += "</code>";
    QDialog *mbox = new QDialog(this);
    mbox->setModal(true);
    mbox->setWindowTitle("Please wait");
    QGridLayout *mbox_layout = new QGridLayout;
    QLabel *mbox_label = new QLabel(mbox_text.c_str());
    mbox_layout->addWidget(mbox_label, 0, 0);
    mbox->setLayout(mbox_layout);
    mbox->show();
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    CmdThread cmd_thread(cmd_index, argv.size() - 1, &(argv[0]));
    cmd_thread.start();
    while (!cmd_thread.isFinished())
    {
        QCoreApplication::processEvents();
        ::usleep(10000);
    }
    int retval = cmd_thread.retval;
    QApplication::restoreOverrideCursor();
    mbox->hide();
    delete mbox;
    for (size_t i = 0; i < argv.size() - 1; i++)
    {
        ::free(argv[i]);
    }
    cmd_close(cmd_index);
    /* restore environment */
    msg::set_file(std_err_bak);
    gtatool_stdout = std_out_bak;
    gtatool_stdin = std_in_bak;
    msg::set_program_name(msg_prg_name_bak);
    msg::set_columns(msg_columns_bak);
    /* read messages */
    try
    {
        fio::rewind(std_err_tmp);
        std_err = "";
        int c;
        while ((c = fio::getc(std_err_tmp)) != EOF)
        {
            std_err.append(1, c);
        }
    }
    catch (std::exception &e)
    {
        std_err = e.what();
        retval = 1;
    }
    try
    {
        fio::close(std_err_tmp);
    }
    catch (...)
    {
    }
    return retval;
}

void GUI::output_cmd(const std::string &cmd, const std::vector<std::string> &args, const std::string &output_name)
{
    try
    {
        FILE *save_file;
        std::string save_name = fio::mktempfile(&save_file);
        std::string std_err;
        int retval = run(cmd, args, std_err, save_file, NULL);
        fio::close(save_file, save_name);
        if (retval != 0)
        {
            try { fio::remove(save_name); } catch (...) {}
            std::string errmsg = "<p>Command <code>";
            errmsg += cmd;
            errmsg += "</code> failed.</p>";
            /*
            std::string errmsg = "<p>Command failed.</p>";
            errmsg += "<p>Command line:</p><pre>";
            errmsg += cmd + " ";
            for (size_t i = 0; i < args.size(); i++)
            {
                errmsg += args[i] + " ";
            }
            errmsg += "</pre>";
            */
            errmsg += "<p>Error message:</p><pre>";
            errmsg += std_err;
            errmsg += "</pre>";
            throw exc(errmsg);
        }
        else if (std_err.length() > 0)
        {
            // print warnings (if any) so that they are not lost
            if (std_err[std_err.length() - 1] == '\n')
                std_err.resize(std_err.length() - 1);
            msg::req_txt(std_err);
        }
        open(output_name, save_name);
    }
    catch (std::exception &e)
    {
        QMessageBox::critical(this, "Error", QTextCodec::codecForLocale()->toUnicode(e.what()));
    }
}

void GUI::import_from(const std::string &cmd, const std::vector<std::string> &options, const QStringList &filters)
{
    QStringList open_file_names = file_open_dialog(filters);
    for (int i = 0; i < open_file_names.size(); i++)
    {
        try
        {
            std::vector<std::string> args = options;
            args.push_back(fio::to_sys(from_qt(open_file_names[i])));
            std::string output_name(qPrintable(open_file_names[i]));
            size_t last_slash = output_name.find_last_of('/');
            size_t last_dot = output_name.find_last_of('.');
            if (last_dot != std::string::npos && (last_slash == std::string::npos || last_dot > last_slash))
            {
                output_name.replace(last_dot, output_name.length() - last_dot, ".gta");
                while (fio::test_e(output_name))
                {
                    output_name.insert(last_dot, "-new");
                }
            }
            else
            {
                output_name += ".gta";
            }
            output_cmd(cmd, args, output_name);
        }
        catch (std::exception &e)
        {
            QMessageBox::critical(this, "Error", e.what());
        }
    }
}

void GUI::export_to(const std::string &cmd, const std::vector<std::string> &options, const QString &default_suffix, const QStringList &filters)
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    QString save_file_name = file_save_dialog(default_suffix, filters, to_qt(fw->file_name()));
    if (!save_file_name.isEmpty())
    {
        try
        {
            std::string std_err;
            std::vector<std::string> args = options;
            args.push_back(fio::to_sys(fw->save_name()));
            args.push_back(fio::to_sys(from_qt(save_file_name)));
            int retval = run(cmd, args, std_err, NULL, NULL);
            if (retval != 0)
            {
                throw exc(std::string("<p>Export failed.</p><pre>") + std_err + "</pre>");
            }
        }
        catch (std::exception &e)
        {
            QMessageBox::critical(this, "Error", QTextCodec::codecForLocale()->toUnicode(e.what()));
        }
    }
}

void GUI::open(const std::string &file_name, const std::string &save_name, int tab_index, bool view)
{
    if (file_name.length() > 0)
    {
        QFileInfo file_info(to_qt(file_name));
        for (int i = 0; i < _files_widget->count(); i++)
        {
            FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->widget(i));
            if (fw->file_name().length() > 0)
            {
                QFileInfo existing_file_info(to_qt(fw->file_name()));
                if (existing_file_info.canonicalFilePath().length() > 0
                        && file_info.canonicalFilePath() == existing_file_info.canonicalFilePath())
                {
                    _files_widget->setCurrentWidget(fw);
                    return;
                }
            }
        }
    }
    std::vector<gta::header*> headers;
    std::vector<off_t> offsets;
    try
    {
        const std::string &name = (save_name.length() == 0 ? file_name : save_name);
        FILE *f = fio::open(name, "r");
        try {
            while (fio::has_more(f, name)) {
                gta::header *hdr = new gta::header;
                hdr->read_from(f);
                headers.push_back(hdr);
                offsets.push_back(fio::tell(f, name));
                hdr->skip_data(f);
            }
        }
        catch (...) {
            fio::close(f, name);
            for (size_t i = 0; i < headers.size(); i++)
                delete headers[i];
            headers.clear();
            throw;
        }
        fio::close(f, name);
        if (headers.size() == 0)
        {
            QMessageBox::critical(this, "Error", "File is empty");
        }
        else
        {
            FileWidget *fw = new FileWidget(file_name, save_name, headers, offsets);
            connect(fw, SIGNAL(changed(const std::string &, const std::string &)), this, SLOT(file_changed(const std::string &, const std::string &)));
            connect(fw, SIGNAL(quit()), this, SLOT(close()));
            QString fn = to_qt(fio::basename(file_name));
            QString tn = (file_name.length() == 0 ? QString("(unnamed)") : fn);
            int ti = (tab_index >= 0 ? _files_widget->insertTab(tab_index, fw, tn) : _files_widget->addTab(fw, tn));
            _files_widget->tabBar()->setTabTextColor(ti, (fw->is_saved() ? "black" : "red"));
            _files_widget->setCurrentIndex(ti);
            if (file_name.compare(name) == 0)
                _files_watcher->addPath(to_qt(file_name));
            if (view)
                static_cast<FileWidget*>(_files_widget->widget(ti))->open_view();
        }
    }
    catch (std::exception &e)
    {
        QMessageBox::critical(this, "Error", e.what());
    }
}

void GUI::file_open()
{
    QStringList file_names = file_open_dialog(QStringList("GTA files (*.gta)"));
    for (int i = 0; i < file_names.size(); i++)
    {
        open(qPrintable(file_names[i]), qPrintable(file_names[i]));
    }
}

void GUI::file_save()
{
    if (!check_have_file())
    {
        return;
    }
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    if (fw->is_saved())
    {
        return;
    }
    if (fw->file_name().length() == 0)
    {
        file_save_as();
        return;
    }
    try
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        FILE *fi = fio::open(fio::to_sys(fw->save_name()), "r");
        FILE *fo = fio::open(fio::to_sys(fw->file_name()) + ".tmp", "w");
        for (size_t i = 0; i < fw->headers().size(); i++)
        {
            gta::header dummy_header;
            dummy_header.read_from(fi);
            fw->headers()[i]->write_to(fo);
            fw->offsets()[i] = fio::tell(fo, fio::to_sys(fw->file_name()) + ".tmp");
            dummy_header.copy_data(fi, *(fw->headers()[i]), fo);
        }
        /* This is a stupid and unsafe way to switch to the new file, but it works
         * cross-platform and also over NFS etc: after this, the file exists and
         * has the expected contents. */
        fio::close(fo, fio::to_sys(fw->file_name()) + ".tmp");
        fio::close(fi, fio::to_sys(fw->file_name()));
        _files_watcher->removePath(to_qt(fw->file_name()));
        try { fio::remove(fw->file_name()); } catch (...) { }
        fio::rename(fw->file_name() + ".tmp", fw->file_name());
        _files_watcher->addPath(to_qt(fw->file_name()));
        fw->saved_to(fw->file_name());
        _files_widget->tabBar()->setTabTextColor(_files_widget->indexOf(fw), "black");
        _files_widget->tabBar()->setTabText(_files_widget->indexOf(fw), to_qt(fio::basename(fw->file_name())));
        QApplication::restoreOverrideCursor();
    }
    catch (std::exception &e)
    {
        QApplication::restoreOverrideCursor();
        QMessageBox::critical(this, "Error", (std::string("Cannot save file: ") + e.what()).c_str());
    }
}

void GUI::file_save_as()
{
    if (!check_have_file())
    {
        return;
    }
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    QString file_name = file_save_dialog();
    if (!file_name.isEmpty())
    {
        if (fw->file_name().length() > 0)
            _files_watcher->removePath(to_qt(fw->file_name()));
        fw->set_file_name(from_qt(file_name));
        file_save();
    }
}

void GUI::file_save_all()
{
    if (!check_have_file())
    {
        return;
    }
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
    if (!check_have_file())
    {
        return;
    }
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    if (!fw->is_saved())
    {
        if (QMessageBox::question(this, "Close file", "File is not saved. Close anyway?",
                    QMessageBox::Close | QMessageBox::Cancel, QMessageBox::Cancel)
                != QMessageBox::Close)
        {
            return;
        }
    }
    if (fw->file_name().length() > 0)
        _files_watcher->removePath(to_qt(fw->file_name()));
    _files_widget->removeTab(_files_widget->indexOf(fw));
    delete fw;
}

void GUI::file_close_all()
{
    for (int i = 0; i < _files_widget->count(); i++)
    {
        FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->widget(i));
        if (!fw->is_saved())
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
        FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->widget(0));
        if (fw->file_name().length() > 0)
            _files_watcher->removePath(to_qt(fw->file_name()));
        _files_widget->removeTab(0);
        delete fw;
    }
}

void GUI::file_import()
{
    import_from("from", std::vector<std::string>(), QStringList());
}

void GUI::file_import_csv()
{
    import_from("from-csv", std::vector<std::string>(), QStringList("CSV files (*.csv)"));
}

void GUI::file_import_datraw()
{
    import_from("from-datraw", std::vector<std::string>(), QStringList("Volume data files (*.dat)"));
}

void GUI::file_import_dcmtk()
{
    import_from("from-dcmtk", std::vector<std::string>(), QStringList("DICOM files (*.dcm)"));
}

void GUI::file_import_exr()
{
    import_from("from-exr", std::vector<std::string>(), QStringList("EXR files (*.exr)"));
}

void GUI::file_import_ffmpeg()
{
    import_from("from-ffmpeg", std::vector<std::string>(), QStringList("Any multimedia files (*.*)"));
}

void GUI::file_import_gdal()
{
    import_from("from-gdal", std::vector<std::string>(), QStringList("TIFF files (*.tif *.tiff)"));
}

void GUI::file_import_jpeg()
{
    import_from("from-jpeg", std::vector<std::string>(), QStringList("JPEG files (*.jpg *.jpeg)"));
}

void GUI::file_import_magick()
{
    import_from("from-magick", std::vector<std::string>(), QStringList("Typical image files (*.png *.jpg)"));
}

void GUI::file_import_mat()
{
    import_from("from-mat", std::vector<std::string>(), QStringList("MATLAB files (*.mat)"));
}

void GUI::file_import_netcdf()
{
    import_from("from-netcdf", std::vector<std::string>(), QStringList("NetCDF files (*.nc *.hdf)"));
}

void GUI::file_import_pcd()
{
    import_from("from-pcd", std::vector<std::string>(), QStringList("PCD files (*.pcd)"));
}

void GUI::file_import_pfs()
{
    import_from("from-pfs", std::vector<std::string>(), QStringList("PFS files (*.pfs)"));
}

void GUI::file_import_ply()
{
    import_from("from-ply", std::vector<std::string>(), QStringList("PLY files (*.ply)"));
}

void GUI::file_import_png()
{
    import_from("from-png", std::vector<std::string>(), QStringList("PNG files (*.png)"));
}

void GUI::file_import_pvm()
{
    import_from("from-pvm", std::vector<std::string>(), QStringList("PVM files (*.pvm)"));
}

void GUI::file_import_rat()
{
    import_from("from-rat", std::vector<std::string>(), QStringList("RAT RadarTools files (*.rat)"));
}

void GUI::file_import_raw()
{
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Import raw data");
    QGridLayout *layout = new QGridLayout;
    QLabel *comp_label = new QLabel("Array element components (comma\nseparated list of the following types:\n"
            "int{8,16,32,64,128}, uint{8,16,32,64,128}\n"
            "float{32,64,128}, cfloat{32,64,128}");
    layout->addWidget(comp_label, 0, 0, 1, 2);
    QLineEdit *comp_edit = new QLineEdit("");
    layout->addWidget(comp_edit, 1, 0, 1, 2);
    QLabel *dim_label = new QLabel("Dimensions (comma separated list):");
    layout->addWidget(dim_label, 2, 0, 1, 2);
    QLineEdit *dim_edit = new QLineEdit("");
    layout->addWidget(dim_edit, 3, 0, 1, 2);
    QRadioButton *le_button = new QRadioButton("Little endian");
    layout->addWidget(le_button, 4, 0);
    le_button->setChecked(true);
    QRadioButton *be_button = new QRadioButton("Big endian");
    layout->addWidget(be_button, 4, 1);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 5, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 5, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    std::vector<std::string> options;
    options.push_back("-c");
    options.push_back(qPrintable(comp_edit->text().simplified().replace(' ', "")));
    options.push_back("-d");
    options.push_back(qPrintable(dim_edit->text().simplified().replace(' ', "")));
    options.push_back("-e");
    options.push_back(le_button->isChecked() ? "little" : "big");
    import_from("from-raw", options, QStringList("Raw files (*.raw *.dat)"));
}

void GUI::file_import_sndfile()
{
    import_from("from-sndfile", std::vector<std::string>(), QStringList("WAV files (*.wav)"));
}

void GUI::file_import_teem()
{
    import_from("from-teem", std::vector<std::string>(), QStringList("NRRD files (*.nrrd)"));
}

void GUI::file_export()
{
    export_to("to", std::vector<std::string>(), QString(), QStringList());
}

void GUI::file_export_csv()
{
    export_to("to-csv", std::vector<std::string>(), "csv", QStringList("CSV files (*.csv)"));
}

void GUI::file_export_datraw()
{
    export_to("to-datraw", std::vector<std::string>(), "dat", QStringList("Volume data files (*.dat)"));
}

void GUI::file_export_exr()
{
    export_to("to-exr", std::vector<std::string>(), "exr", QStringList("EXR files (*.exr)"));
}

void GUI::file_export_gdal()
{
    export_to("to-gdal", std::vector<std::string>(), "tif", QStringList("TIFF files (*.tif *.tiff)"));
}

void GUI::file_export_jpeg()
{
    export_to("to-jpeg", std::vector<std::string>(), "jpg", QStringList("JPEG files (*.jpg *.jpeg)"));
}

void GUI::file_export_magick()
{
    export_to("to-magick", std::vector<std::string>(), "png", QStringList("Typical image files (*.png *.jpg)"));
}

void GUI::file_export_mat()
{
    export_to("to-mat", std::vector<std::string>(), "mat", QStringList("MATLAB files (*.mat)"));
}

void GUI::file_export_netcdf()
{
    export_to("to-netcdf", std::vector<std::string>(), "nc", QStringList("NetCDF files (*.nc *.hdf)"));
}

void GUI::file_export_pcd()
{
    export_to("to-pcd", std::vector<std::string>(), "pcd", QStringList("PCD files (*.pcd)"));
}

void GUI::file_export_pfs()
{
    export_to("to-pfs", std::vector<std::string>(), "pfs", QStringList("PFS files (*.pfs)"));
}

void GUI::file_export_ply()
{
    export_to("to-ply", std::vector<std::string>(), "ply", QStringList("PLY files (*.ply)"));
}

void GUI::file_export_png()
{
    export_to("to-png", std::vector<std::string>(), "png", QStringList("PNG files (*.png)"));
}

void GUI::file_export_pvm()
{
    export_to("to-pvm", std::vector<std::string>(), "pvm", QStringList("PVM files (*.pvm)"));
}

void GUI::file_export_rat()
{
    export_to("to-rat", std::vector<std::string>(), "rat", QStringList("RAT RadarTools files (*.rat)"));
}

void GUI::file_export_raw()
{
    if (!check_have_file())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Export raw data");
    QGridLayout *layout = new QGridLayout;
    QRadioButton *le_button = new QRadioButton("Little endian");
    layout->addWidget(le_button, 0, 0);
    le_button->setChecked(true);
    QRadioButton *be_button = new QRadioButton("Big endian");
    layout->addWidget(be_button, 0, 1);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 1, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 1, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    std::vector<std::string> options;
    options.push_back("-e");
    options.push_back(le_button->isChecked() ? "little" : "big");
    export_to("to-raw", options, "raw", QStringList("Raw files (*.raw *.dat)"));
}

void GUI::file_export_sndfile()
{
    export_to("to-sndfile", std::vector<std::string>(), "wav", QStringList("WAV files (*.wav)"));
}

void GUI::file_export_teem()
{
    export_to("to-teem", std::vector<std::string>(), "nrrd", QStringList("NRRD files (*.nrrd)"));
}

void GUI::stream_extract()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    std::vector<std::string> args;
    args.push_back(str::from(fw->array_index()));
    args.push_back(fio::to_sys(fw->save_name()));
    output_cmd("stream-extract", args, "");
}

void GUI::stream_foreach()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Run command for each array");
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(new QLabel("Enter command. %I will be replaced with the array index."), 0, 0, 1, 2);
    layout->addWidget(new QLabel("Example: gta tag --set-global=\"X-INDEX=%I\""), 1, 0, 1, 2);
    QLineEdit *edt = new QLineEdit("");
    layout->addWidget(edt, 2, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 3, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 3, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    std::vector<std::string> args;
    args.push_back(qPrintable(edt->text()));
    args.push_back(fio::to_sys(fw->save_name()));
    output_cmd("stream-foreach", args, "");
}

void GUI::stream_grep()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Select arrays from stream based on checks");
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(new QLabel("Enter command. Exit status 0 will select a GTA."), 0, 0, 1, 2);
    layout->addWidget(new QLabel("Example: gta info 2>&1 > /dev/null | grep \"dimension 0: 42\""), 1, 0, 1, 2);
    QLineEdit *edt = new QLineEdit("");
    layout->addWidget(edt, 2, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 3, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 3, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    std::vector<std::string> args;
    args.push_back(qPrintable(edt->text()));
    args.push_back(fio::to_sys(fw->save_name()));
    output_cmd("stream-grep", args, "");
}

void GUI::stream_merge()
{
    if (!check_have_file() || !check_all_files_unchanged())
    {
        return;
    }
    std::vector<std::string> args;
    for (int i = 0; i < _files_widget->count(); i++)
    {
        FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->widget(i));
        args.push_back(fio::to_sys(fw->save_name()));
    }
    output_cmd("stream-merge", args, "");
}

void GUI::stream_split()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    QMessageBox::information(this, "Split stream",
            "The arrays will be saved in files 000000000.gta,\n"
            "000000001.gta, and so on. Please choose a directory.");
    QFileDialog *file_dialog = new QFileDialog(this);
    file_dialog->setWindowTitle(tr("Split"));
    file_dialog->setAcceptMode(QFileDialog::AcceptSave);
    file_dialog->setFileMode(QFileDialog::DirectoryOnly);
    QDir last_dir = QDir(global_settings->value("general/last-dir").toString());
    if (last_dir.exists())
        file_dialog->setDirectory(last_dir);
    if (file_dialog->exec()) {
        try {
            QString dir_name = file_dialog->selectedFiles().at(0);
            global_settings->setValue("general/last-dir", file_dialog->directory().path());
            FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
            std::vector<std::string> args;
            args.push_back(fio::to_sys(std::string(qPrintable(QDir(dir_name).canonicalPath())) + "/%9N.gta"));
            args.push_back(fio::to_sys(fw->save_name()));
            std::string std_err;
            int retval = run("stream-split", args, std_err, NULL, NULL);
            if (retval != 0)
                throw exc(std::string("<p>Command failed.</p><pre>") + std_err + "</pre>");
        }
        catch (std::exception &e) {
            QMessageBox::critical(this, "Error", QTextCodec::codecForLocale()->toUnicode(e.what()));
        }
    }
}

void GUI::array_create()
{
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Create array");
    QGridLayout *layout = new QGridLayout;
    QLabel *comp_label = new QLabel("Array element components (comma\nseparated list of the following types:\n"
            "int{8,16,32,64,128}, uint{8,16,32,64,128}\n"
            "float{32,64,128}, cfloat{32,64,128}");
    layout->addWidget(comp_label, 0, 0, 1, 2);
    QLineEdit *comp_edit = new QLineEdit("");
    layout->addWidget(comp_edit, 1, 0, 1, 2);
    QLabel *dim_label = new QLabel("Dimensions (comma separated list):");
    layout->addWidget(dim_label, 2, 0, 1, 2);
    QLineEdit *dim_edit = new QLineEdit("");
    layout->addWidget(dim_edit, 3, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 4, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 4, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    std::vector<std::string> args;
    args.push_back("-c");
    args.push_back(qPrintable(comp_edit->text().simplified().replace(' ', "")));
    args.push_back("-d");
    args.push_back(qPrintable(dim_edit->text().simplified().replace(' ', "")));
    output_cmd("create", args, "");
}

void GUI::array_diff()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Compute differences between two sets of arrays");
    QGridLayout *layout = new QGridLayout;
    QCheckBox *abs_box = new QCheckBox("Compute absolute difference");
    layout->addWidget(abs_box, 0, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 1, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 1, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    std::vector<std::string> args;
    if (abs_box->isChecked())
    {
        args.push_back("-a");
    }
    if (_files_widget->count() >= 2)
    {
        for (int i = _files_widget->count() - 2; i < _files_widget->count(); i++)
        {
            FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->widget(i));
            args.push_back(fio::to_sys(fw->save_name()));
        }
    }
    else
    {
        FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->widget(0));
        args.push_back(fio::to_sys(fw->save_name()));
        args.push_back(fio::to_sys(fw->save_name()));
    }
    output_cmd("diff", args, "");
}

void GUI::array_extract()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Extract sub-arrays");
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(new QLabel("Lower indices (comma separated):"), 0, 0, 1, 2);
    QLineEdit *low_edit = new QLineEdit("");
    layout->addWidget(low_edit, 1, 0, 1, 2);
    layout->addWidget(new QLabel("Higher indices (comma separated):"), 2, 0, 1, 2);
    QLineEdit *high_edit = new QLineEdit("");
    layout->addWidget(high_edit, 3, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 4, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 4, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    std::vector<std::string> args;
    args.push_back("-l");
    args.push_back(qPrintable(low_edit->text().simplified().replace(' ', "")));
    args.push_back("-h");
    args.push_back(qPrintable(high_edit->text().simplified().replace(' ', "")));
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    args.push_back(fio::to_sys(fw->save_name()));
    output_cmd("extract", args, "");
}

void GUI::array_fill()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Fill sub-arrays");
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(new QLabel("Lower indices (comma separated):"), 0, 0, 1, 2);
    QLineEdit *low_edit = new QLineEdit("");
    layout->addWidget(low_edit, 1, 0, 1, 2);
    layout->addWidget(new QLabel("Higher indices (comma separated):"), 2, 0, 1, 2);
    QLineEdit *high_edit = new QLineEdit("");
    layout->addWidget(high_edit, 3, 0, 1, 2);
    layout->addWidget(new QLabel("Component values (comma separated):"), 4, 0, 1, 2);
    QLineEdit *val_edit = new QLineEdit("");
    layout->addWidget(val_edit, 5, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 6, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 6, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    std::vector<std::string> args;
    args.push_back("-l");
    args.push_back(qPrintable(low_edit->text().simplified().replace(' ', "")));
    args.push_back("-h");
    args.push_back(qPrintable(high_edit->text().simplified().replace(' ', "")));
    args.push_back("-v");
    args.push_back(qPrintable(val_edit->text().simplified().replace(' ', "")));
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    args.push_back(fio::to_sys(fw->save_name()));
    output_cmd("fill", args, "");
}

void GUI::array_combine()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Combine arrays");
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(new QLabel("Mode:"), 0, 0);
    QComboBox* mode_box = new QComboBox();
    layout->addWidget(mode_box, 0, 1);
    mode_box->addItem("min");
    mode_box->addItem("max");
    mode_box->addItem("add");
    mode_box->addItem("sub");
    mode_box->addItem("mul");
    mode_box->addItem("div");
    mode_box->addItem("and");
    mode_box->addItem("or");
    mode_box->addItem("xor");
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 1, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 1, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    std::vector<std::string> args;
    args.push_back("-m");
    args.push_back(qPrintable(mode_box->currentText()));
    for (int i = 0; i < _files_widget->count(); i++)
    {
        FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->widget(i));
        args.push_back(fio::to_sys(fw->save_name()));
    }
    output_cmd("combine", args, "");
}

void GUI::array_merge()
{
    if (!check_have_file() || !check_all_files_unchanged())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Merge arrays");
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(new QLabel("Dimension:"), 0, 0, 1, 2);
    QLineEdit *dim_edit = new QLineEdit("");
    layout->addWidget(dim_edit, 1, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 2, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 2, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    std::vector<std::string> args;
    args.push_back("-d");
    args.push_back(qPrintable(dim_edit->text().simplified().replace(' ', "")));
    for (int i = 0; i < _files_widget->count(); i++)
    {
        FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->widget(i));
        args.push_back(fio::to_sys(fw->save_name()));
    }
    output_cmd("merge", args, "");
}

void GUI::array_resize()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Resize arrays");
    QGridLayout *layout = new QGridLayout;
    QLabel *dim_label = new QLabel("New dimensions (comma separated list):");
    layout->addWidget(dim_label, 0, 0, 1, 2);
    QLineEdit *dim_edit = new QLineEdit("");
    layout->addWidget(dim_edit, 1, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 2, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 2, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    std::vector<std::string> args;
    args.push_back("-d");
    args.push_back(qPrintable(dim_edit->text().simplified().replace(' ', "")));
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    args.push_back(fio::to_sys(fw->save_name()));
    output_cmd("resize", args, "");
}

void GUI::array_set()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Set sub-arrays");
    QGridLayout *layout = new QGridLayout;
    QLabel *indices_label = new QLabel("Place other array at the following indices:");
    layout->addWidget(indices_label, 0, 0, 1, 2);
    QLineEdit *indices_edit = new QLineEdit("");
    layout->addWidget(indices_edit, 1, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 2, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 2, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    QStringList source_file_names = file_open_dialog(QStringList("GTA files (*.gta)"));
    if (source_file_names.size() < 1)
    {
        return;
    }
    if (source_file_names.size() > 1)
    {
        QMessageBox::critical(this, "Error", "Please choose only one array file.");
        return;
    }
    std::vector<std::string> args;
    args.push_back("-s");
    args.push_back(qPrintable(source_file_names[0]));
    args.push_back("-i");
    args.push_back(qPrintable(indices_edit->text().simplified().replace(' ', "")));
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    args.push_back(fio::to_sys(fw->save_name()));
    output_cmd("set", args, "");
}

void GUI::dimension_add()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Add dimension");
    QGridLayout *layout = new QGridLayout;
    QLabel *dim_label = new QLabel("Index of new dimension:");
    layout->addWidget(dim_label, 0, 0, 1, 2);
    QLineEdit *dim_edit = new QLineEdit("");
    layout->addWidget(dim_edit, 1, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 2, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 2, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    std::vector<std::string> args;
    args.push_back("-d");
    args.push_back(qPrintable(dim_edit->text().simplified().replace(' ', "")));
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    args.push_back(fio::to_sys(fw->save_name()));
    output_cmd("dimension-add", args, "");
}

void GUI::dimension_extract()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Extract dimension");
    QGridLayout *layout = new QGridLayout;
    QLabel *dim_label = new QLabel("Index of dimension to extract:");
    layout->addWidget(dim_label, 0, 0, 1, 2);
    QLineEdit *dim_edit = new QLineEdit("");
    layout->addWidget(dim_edit, 1, 0, 1, 2);
    QLabel *index_label = new QLabel("Index inside this dimension:");
    layout->addWidget(index_label, 2, 0, 1, 2);
    QLineEdit *index_edit = new QLineEdit("");
    layout->addWidget(index_edit, 3, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 4, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 4, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    std::vector<std::string> args;
    args.push_back("-d");
    args.push_back(qPrintable(dim_edit->text().simplified().replace(' ', "")));
    args.push_back("-i");
    args.push_back(qPrintable(index_edit->text().simplified().replace(' ', "")));
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    args.push_back(fio::to_sys(fw->save_name()));
    output_cmd("dimension-extract", args, "");
}

void GUI::dimension_flatten()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Flatten dimensions (make one-dimensional)");
    QGridLayout *layout = new QGridLayout;
    QCheckBox *p_checkbox = new QCheckBox("Prepend original coordinates to each array element");
    layout->addWidget(p_checkbox, 0, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 1, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 1, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    std::vector<std::string> args;
    if (p_checkbox->isChecked())
    {
        args.push_back("-p");
    }
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    args.push_back(fio::to_sys(fw->save_name()));
    output_cmd("dimension-flatten", args, "");
}

void GUI::dimension_merge()
{
    if (!check_have_file() || !check_all_files_unchanged())
    {
        return;
    }
    std::vector<std::string> args;
    for (int i = 0; i < _files_widget->count(); i++)
    {
        FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->widget(i));
        args.push_back(fio::to_sys(fw->save_name()));
    }
    output_cmd("dimension-merge", args, "");
}

void GUI::dimension_reorder()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Reorder dimensions");
    QGridLayout *layout = new QGridLayout;
    QLabel *indices_label = new QLabel("New order of dimensions\n(comma separated list of indices):");
    layout->addWidget(indices_label, 0, 0, 1, 2);
    QLineEdit *indices_edit = new QLineEdit("");
    layout->addWidget(indices_edit, 1, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 2, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 2, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    std::vector<std::string> args;
    args.push_back("-i");
    args.push_back(qPrintable(indices_edit->text().simplified().replace(' ', "")));
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    args.push_back(fio::to_sys(fw->save_name()));
    output_cmd("dimension-reorder", args, "");
}

void GUI::dimension_reverse()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Reverse dimensions");
    QGridLayout *layout = new QGridLayout;
    QLabel *indices_label = new QLabel("Dimensions to reverse\n(comma separated list of indices):");
    layout->addWidget(indices_label, 0, 0, 1, 2);
    QLineEdit *indices_edit = new QLineEdit("");
    layout->addWidget(indices_edit, 1, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 2, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 2, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    std::vector<std::string> args;
    args.push_back("-i");
    args.push_back(qPrintable(indices_edit->text().simplified().replace(' ', "")));
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    args.push_back(fio::to_sys(fw->save_name()));
    output_cmd("dimension-reverse", args, "");
}

void GUI::dimension_split()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Split along dimension");
    QGridLayout *layout = new QGridLayout;
    QLabel *dim_label = new QLabel("Index of dimension to split at:");
    layout->addWidget(dim_label, 0, 0, 1, 2);
    QLineEdit *dim_edit = new QLineEdit("");
    layout->addWidget(dim_edit, 1, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 2, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 2, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    std::vector<std::string> args;
    args.push_back("-d");
    args.push_back(qPrintable(dim_edit->text().simplified().replace(' ', "")));
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    args.push_back(fio::to_sys(fw->save_name()));
    output_cmd("dimension-split", args, "");
}

void GUI::component_add()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Add components");
    QGridLayout *layout = new QGridLayout;
    QLabel *comp_label = new QLabel("Component types to add (comma\nseparated list of the following types:\n"
            "int{8,16,32,64,128}, uint{8,16,32,64,128}\n"
            "float{32,64,128}, cfloat{32,64,128}");
    layout->addWidget(comp_label, 0, 0, 1, 2);
    QLineEdit *comp_edit = new QLineEdit("");
    layout->addWidget(comp_edit, 1, 0, 1, 2);
    QLabel *index_label = new QLabel("Index at which to insert the components:");
    layout->addWidget(index_label, 2, 0, 1, 2);
    QLineEdit *index_edit = new QLineEdit("");
    layout->addWidget(index_edit, 3, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 4, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 4, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    std::vector<std::string> args;
    args.push_back("-c");
    args.push_back(qPrintable(comp_edit->text().simplified().replace(' ', "")));
    args.push_back("-i");
    args.push_back(qPrintable(index_edit->text().simplified().replace(' ', "")));
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    args.push_back(fio::to_sys(fw->save_name()));
    output_cmd("component-add", args, "");
}

void GUI::component_compute()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Recompute component values");
    QGridLayout *layout = new QGridLayout;
    QLabel *expression_label = new QLabel("Expression to compute:");
    layout->addWidget(expression_label, 0, 0, 1, 2);
    QLineEdit *expression_edit = new QLineEdit("");
    layout->addWidget(expression_edit, 1, 0, 1, 2);
    QLabel *help_label = new QLabel(
            "<p>Modifiable variables:"
            "<ul><li>c0, c1, ...: Array element components<br>"
            "(For cfloat types: c0re, c0im, c1re, c1im, ...)</li></ul>"
            "Non-modifiable variables:"
            "<ul><li>c: Number of array element components</li>"
            "<li>d: Number of array dimensions</li>"
            "<li>d0, d1, ...: Array size in each dimension</li>"
            "<li>i0, i1, ...: Index of the current array element in each dimension</li></ul>"
            "Expressions are evaluated using the muParser library.<br>"
            "See <a href=\"http://muparser.sourceforge.net/mup_features.html\">"
            "http://muparser.sourceforge.net/mup_features.html</a><br>"
            "for an overview of available operators and functions.</p>"
            "<p>All computations use double precision.<br>"
            "Multiple expressions can be separated by semicolons.</p>");
    layout->addWidget(help_label, 2, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 3, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 3, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    std::vector<std::string> args;
    QStringList expressions = expression_edit->text().split(';');
    if (expressions.empty())
    {
        return;
    }
    for (int i = 0; i < expressions.size(); i++)
    {
        args.push_back("-e");
        args.push_back(qPrintable(expressions[i]));
    }
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    args.push_back(fio::to_sys(fw->save_name()));
    output_cmd("component-compute", args, "");
}

void GUI::component_convert()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Convert component types");
    QGridLayout *layout = new QGridLayout;
    QLabel *comp_label = new QLabel("New component types (comma\nseparated list of the following types:\n"
            "int{8,16,32,64,128}, uint{8,16,32,64,128}\n"
            "float{32,64,128}, cfloat{32,64,128}");
    layout->addWidget(comp_label, 0, 0, 1, 2);
    QLineEdit *comp_edit = new QLineEdit("");
    layout->addWidget(comp_edit, 1, 0, 1, 2);
    QCheckBox *n_checkbox = new QCheckBox("Normalize when converting between integers and floating point");
    layout->addWidget(n_checkbox, 2, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 3, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 3, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    std::vector<std::string> args;
    if (n_checkbox->isChecked())
    {
        args.push_back("-n");
    }
    args.push_back("-c");
    args.push_back(qPrintable(comp_edit->text().simplified().replace(' ', "")));
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    args.push_back(fio::to_sys(fw->save_name()));
    output_cmd("component-convert", args, "");
}

void GUI::component_extract()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Extract components");
    QGridLayout *layout = new QGridLayout;
    QLabel *index_label = new QLabel("Indices of components to extract:");
    layout->addWidget(index_label, 0, 0, 1, 2);
    QLineEdit *index_edit = new QLineEdit("");
    layout->addWidget(index_edit, 1, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 2, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 2, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    std::vector<std::string> args;
    args.push_back("-k");
    args.push_back(qPrintable(index_edit->text().simplified().replace(' ', "")));
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    args.push_back(fio::to_sys(fw->save_name()));
    output_cmd("component-extract", args, "");
}

void GUI::component_merge()
{
    if (!check_have_file() || !check_all_files_unchanged())
    {
        return;
    }
    std::vector<std::string> args;
    for (int i = 0; i < _files_widget->count(); i++)
    {
        FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->widget(i));
        args.push_back(fio::to_sys(fw->save_name()));
    }
    output_cmd("component-merge", args, "");
}

void GUI::component_reorder()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Reorder components");
    QGridLayout *layout = new QGridLayout;
    QLabel *indices_label = new QLabel("New order of components\n(comma separated list of indices):");
    layout->addWidget(indices_label, 0, 0, 1, 2);
    QLineEdit *indices_edit = new QLineEdit("");
    layout->addWidget(indices_edit, 1, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 2, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 2, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    std::vector<std::string> args;
    args.push_back("-i");
    args.push_back(qPrintable(indices_edit->text().simplified().replace(' ', "")));
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    args.push_back(fio::to_sys(fw->save_name()));
    output_cmd("component-reorder", args, "");
}

void GUI::component_set()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle("Set component values");
    QGridLayout *layout = new QGridLayout;
    QLabel *indices_label = new QLabel("Indices of components to set\n(comma separated list):");
    layout->addWidget(indices_label, 0, 0, 1, 2);
    QLineEdit *indices_edit = new QLineEdit("");
    layout->addWidget(indices_edit, 1, 0, 1, 2);
    QLabel *values_label = new QLabel("Values for these components\n(comma separated list):");
    layout->addWidget(values_label, 2, 0, 1, 2);
    QLineEdit *values_edit = new QLineEdit("");
    layout->addWidget(values_edit, 3, 0, 1, 2);
    QPushButton *ok_btn = new QPushButton(tr("&OK"));
    ok_btn->setDefault(true);
    connect(ok_btn, SIGNAL(clicked()), dialog, SLOT(accept()));
    layout->addWidget(ok_btn, 4, 0);
    QPushButton *cancel_btn = new QPushButton(tr("&Cancel"), dialog);
    connect(cancel_btn, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(cancel_btn, 4, 1);
    dialog->setLayout(layout);
    if (dialog->exec() == QDialog::Rejected)
    {
        return;
    }
    std::vector<std::string> args;
    args.push_back("-i");
    args.push_back(qPrintable(indices_edit->text().simplified().replace(' ', "")));
    args.push_back("-v");
    args.push_back(qPrintable(values_edit->text().simplified().replace(' ', "")));
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    args.push_back(fio::to_sys(fw->save_name()));
    output_cmd("component-set", args, "");
}

void GUI::component_split()
{
    if (!check_have_file() || !check_file_unchanged())
    {
        return;
    }
    std::vector<std::string> args;
    FileWidget *fw = reinterpret_cast<FileWidget *>(_files_widget->currentWidget());
    args.push_back(fio::to_sys(fw->save_name()));
    output_cmd("component-split", args, "");
}

void GUI::help_overview()
{
    QMessageBox::about(this, tr("Overview"), tr(
                "<p>This program manipulates Generic Tagged Arrays (GTAs).</p>"
                "<p>A GTA is an n-dimensional <i>array</i> with metadata in the form "
                "of <i>tags</i> (key-value pairs). "
                "A GTA file or <i>stream</i> contains a sequence of such arrays. "
                "Each array has n <i>dimensions</i>. For example, images have 2 dimensions, and "
                "volume data sets have 3. "
                "Each array element consists of m <i>components</i>. These components can have "
                "different types. For example, image data is commonly stored "
                "using 3 components of type <code>uint8</code>.</p>"
                "<p>The <code>%1</code> tool provides commands to manipulate GTAs. These commands are "
                "organized in the following categories:<ul>"
                "<li>Commands that operate on element component level. "
                "For example, these commands add or remove components, or change their types."
                "<li>Commands that operate on dimension level. "
                "For example, these commands add or remove dimensions, or change their sizes."
                "<li>Commands that operate on array level. "
                "For example, these commands create or compare arrays, or modify array tags."
                "<li>Commands that operate on stream level. "
                "For example, these commands add or remove arrays."
                "<li>Commands to convert from/to other file formats. "
                "These commands import and export GTAs from/to many different file formats."
                "</ul></p>"
                "<p>This user interface is a frontend for the command line tool, "
                "and provides only a limited subset of the full functionality.</p>"
                "<p>Use <code>%1 help</code> to get a list of all commands provided by this tool, "
                "and <code>%1 help &lt;cmd&gt;</code> to get a description of a specific command.</p>"
                ).arg(program_name));
}

void GUI::help_website()
{
    if (!QDesktopServices::openUrl(QUrl(PACKAGE_URL)))
    {
        QMessageBox::critical(this, "Error", "Cannot open website.");
    }
}

void GUI::help_about()
{
    QMessageBox::about(this, tr("About " PACKAGE_NAME), tr(
                "<p>This is %1 version %2, using libgta version %3.</p>"
                "<p>Copyright (C) 2014 Martin Lambers.</p>"
                "<p>See <a href=\"%4\">%4</a> for more information on this software.</p>"
                "This is <a href=\"http://www.gnu.org/philosophy/free-sw.html\">free software</a>. "
                "You may redistribute copies of it under the terms of the "
                "<a href=\"http://www.gnu.org/licenses/gpl.html\">GNU General Public License</a>. "
                "There is NO WARRANTY, to the extent permitted by law.</p>")
            .arg(PACKAGE_NAME).arg(VERSION).arg(gta::version()).arg(PACKAGE_URL));
}

extern int qInitResources_gui();
#if QT_VERSION >= 0x050000
# if W32
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
Q_IMPORT_PLUGIN(AccessibleFactory)
# endif
#endif

#if QT_VERSION < 0x050000
static void qt_msg_handler(QtMsgType type, const char *msg)
#else
static void qt_msg_handler(QtMsgType type, const QMessageLogContext&, const QString& msg)
#endif
{
#if QT_VERSION < 0x050000
    std::string s = msg;
#else
    std::string s = qPrintable(msg);
#endif
    switch (type)
    {
    case QtDebugMsg:
        msg::dbg(str::sanitize(s));
        break;
    case QtWarningMsg:
        msg::wrn(str::sanitize(s));
        break;
    case QtCriticalMsg:
        msg::err(str::sanitize(s));
        break;
    case QtFatalMsg:
    default:
        msg::err(str::sanitize(s));
        std::exit(1);
    }
}

extern "C" int gtatool_gui(int argc, char *argv[])
{
#ifdef Q_WS_X11
    // This only works with Qt4; Qt5 ignores the 'have_display' flag.
    const char *display = getenv("DISPLAY");
    bool have_display = (display && display[0] != '\0');
#else
    bool have_display = true;
#endif
#ifdef Q_OS_UNIX
    setenv("__GL_SYNC_TO_VBLANK", "0", 1);                      // for the 'view' command; works on Linux
#endif
    QCoreApplication::setAttribute(Qt::AA_X11InitThreads);      // for the 'view' command on X11
    /* Let Qt handle the command line first, so that Qt options work */
#if QT_VERSION < 0x050000
    qInstallMsgHandler(qt_msg_handler);
#else
    qInstallMessageHandler(qt_msg_handler);
#endif
    QApplication *app = new QApplication(argc, argv, have_display);
#if QT_VERSION < 0x050000
    // Make Qt4 behave like Qt5: always interpret all C strings as UTF-8.
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif
    // Set the correct encoding for the locale. Required e.g. for qPrintable() to work.
    QTextCodec::setCodecForLocale(QTextCodec::codecForName(str::localcharset().c_str()));
    QCoreApplication::setOrganizationName(PACKAGE_TARNAME);
    QCoreApplication::setApplicationName(PACKAGE_TARNAME);
    global_settings = new QSettings;
    /* Force linking of the Qt resources. Necessary if dynamic modules are disabled. */
    qInitResources_gui();
    /* Now handle our own command line options / arguments.
     * Accept and ignore some options that may be passed to Equalizer from the view command. */
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::val<std::string> eq_server("eq-server", '\0', opt::optional);
    options.push_back(&eq_server);
    opt::val<std::string> eq_config("eq-config", '\0', opt::optional);
    options.push_back(&eq_config);
    opt::val<std::string> eq_listen("eq-listen", '\0', opt::optional);
    options.push_back(&eq_listen);
    opt::val<std::string> eq_logfile("eq-logfile", '\0', opt::optional);
    options.push_back(&eq_logfile);
    opt::val<std::string> eq_render_client("eq-render-client", '\0', opt::optional);
    options.push_back(&eq_render_client);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        delete app;
        delete global_settings;
        return 1;
    }
    if (help.value())
    {
        if (::strcmp(argv[0], "view") == 0)
            cmd_run_help(cmd_find("view"));
        else
            gtatool_gui_help();
        delete app;
        delete global_settings;
        return 0;
    }
    /* Run the GUI */
    if (!have_display)
    {
        msg::err_txt("GUI failure: cannot connect to X server");
        delete app;
        delete global_settings;
        return 1;
    }
#if W32
    DWORD console_process_list[1];
    if (GetConsoleProcessList(console_process_list, 1) == 1)
    {
        // We are the only process using the console. In particular, the user did not call us
        // via the command interpreter and thus will not watch the console for output.
        // Therefore, we can get rid of it with FreeConsole().
        // But this closes our stdout, and some commands need a valid stdout (e.g.
        // stream-foreach, stream-grep), so be sure to create a new one.
        HANDLE rpl_stdout = CreateFile("NUL", GENERIC_WRITE, 0, NULL,
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        FreeConsole();
        SetStdHandle(STD_OUTPUT_HANDLE, rpl_stdout);
    }
#endif
    int retval = 0;
    try
    {
        GUI *gui = new GUI();
        gui->show();
        bool view = (::strcmp(argv[0], "view") == 0);
        for (size_t i = 0; i < arguments.size(); i++) {
            gui->open(fio::from_sys(arguments[i]), fio::from_sys(arguments[i]), -1, view);
        }
        retval = app->exec();
        delete gui;
    }
    catch (std::exception &e)
    {
        msg::err_txt("GUI failure: %s", e.what());
        retval = 1;
    }
    catch (...)
    {
        msg::err_txt("GUI failure");
        retval = 1;
    }
#if DYNAMIC_MODULES
    if (gtatool_view_create && ::strcmp(argv[0], "view") != 0)
        cmd_close(cmd_find("view"));
#endif
    delete app;
    delete global_settings;
    return retval;
}
