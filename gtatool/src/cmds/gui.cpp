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
#include <QTableWidget>
#include <QHeaderView>
#include <QTabWidget>
#include <QGridLayout>
#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include <QCloseEvent>

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
    : QWidget(parent), _header(header), _type(type), _index(index)
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
    update();
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(_tablewidget, 0, 0);
    layout->setRowStretch(0, 1);
    layout->setColumnStretch(0, 1);        
    setLayout(layout);
}

TaglistWidget::~TaglistWidget()
{
}

void TaglistWidget::update()
{
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
        _tablewidget->setItem(row, 0, new QTableWidgetItem(QString(taglist.name(i))));
        _tablewidget->setItem(row, 1, new QTableWidgetItem(QString(taglist.value(i))));
        _tablewidget->setRowHeight(row, row_height);
    }
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
    _taglists_widget = new QTabWidget;
    layout->addWidget(_taglists_widget, 3, 0, 1, 4);
    update();
    layout->setRowStretch(6, 1);
    layout->setColumnStretch(3, 1);
    setLayout(layout);
}

ArrayWidget::~ArrayWidget()
{
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
    _taglists_widget->addTab(new TaglistWidget(_header, TaglistWidget::global), QString("Global"));
    for (uintmax_t i = 0; i < _header->dimensions(); i++)
    {
        _taglists_widget->addTab(new TaglistWidget(_header, TaglistWidget::dimension, i),
                QString((std::string("Dim ") + str::from(i)
                        /* + " (" + str::from(_header->dimension_size(i)) + ")" */).c_str()));
    }
    for (uintmax_t i = 0; i < _header->components(); i++)
    {
        _taglists_widget->addTab(new TaglistWidget(_header, TaglistWidget::component, i),
                QString((std::string("Comp ") + str::from(i)
                        /* + " (" + type_to_string(_header->component_type(i), _header->component_size(i)) + ")" */).c_str()));
    }
}

FileWidget::FileWidget(const std::string &name, const std::string &temp_name,
        const std::vector<gta::header *> &headers,
        const std::vector<off_t> &offsets,
        QWidget *parent)
    : QWidget(parent),
    _name(name), _temp_name(temp_name), _is_changed(name.length() != 0),
    _headers(headers), _offsets(offsets)
{

    _arrays_widget = new QTabWidget;
    for (size_t i = 0; i < headers.size(); i++)
    {
        _arrays_widget->addTab(new ArrayWidget(headers[i]), QString((std::string("Array ") + str::from(i)).c_str()));
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

GUI::GUI()
{
    setWindowTitle(PACKAGE_NAME);
    setWindowIcon(QIcon(":gui.png"));
    QWidget *widget = new QWidget;
    QGridLayout *layout = new QGridLayout;
    _tabwidget = new QTabWidget;
    layout->addWidget(_tabwidget, 0, 0);
    layout->setRowStretch(0, 1);
    layout->setColumnStretch(0, 1);
    widget->setLayout(layout);
    setCentralWidget(widget);

    QMenu *file_menu = menuBar()->addMenu(tr("&File"));
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
    event->accept();
}

void GUI::open(const std::string &filename)
{
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
            QMessageBox::critical(this, "Error", "File is empty");
        }
        else
        {
            _tabwidget->addTab(new FileWidget(filename, "", headers, offsets), QString(cio::to_sys(cio::basename(filename)).c_str()));
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

void GUI::help_about()
{
    QMessageBox::about(this, tr("About " PACKAGE_NAME), tr(
                "<p>This is %1 version %2, using libgta version %3.</p>"
                "<p>Copyright (C) 2010  Martin Lambers and others.<br>"
                "This is free software. You may redistribute copies of it under the terms of "
                "the <a href=\"http://www.gnu.org/licenses/gpl.html\">"
                "GNU General Public License</a>.<br>"
                "There is NO WARRANTY, to the extent permitted by law.</p>"
                "See <a href=\"%4\">%5</a> for more information on this software.</p>")
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
    catch (exc &e)
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
