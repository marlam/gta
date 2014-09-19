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

#include <unistd.h>
#include <GL/glew.h>

#include <QMainWindow>
#include <QSettings>
#include <QImage>
#include <QGridLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QDialog>
#include <QRadioButton>
#include <QLabel>
#include <QApplication>
#include <QComboBox>
#include <QDesktopWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QCheckBox>
#include <QStandardItemModel>
#include <QGLFormat>
#include <QThread>
#include <QTextCodec>

#include "base/dbg.h"
#include "base/msg.h"
#include "base/str.h"
#include "base/fio.h"

#include "renderer.hpp"
#include "glwidget.hpp"

#include "widget.hpp"


extern int qInitResources_view();

View::View() :
    ViewWidget(), _argc(NULL), _argv(NULL), _settings(NULL),
    _mode_widget(NULL),
    _renderer_factory(NULL),
    _glwidget(NULL),
#if WITH_EQUALIZER
    _eqwidget(NULL),
#endif
    _active_glwidget(NULL),
    _renderloop_timer(NULL)
{
    // Force linking of the Qt resources. Necessary if dynamic modules are disabled.
    qInitResources_view();

    // Set application properties
    setWindowTitle(PACKAGE_NAME ": View");
    setWindowIcon(QIcon(":gui.png"));

    // Create central widget
    QWidget* widget = new QWidget;
    QGridLayout* layout = new QGridLayout();
    layout->setRowStretch(0, 1);
    widget->setLayout(layout);
    setCentralWidget(widget);

    /* Create menus */

    // View menu
    QMenu* view_menu = menuBar()->addMenu("&View");
    QAction* conf_fullscreen_act = new QAction("Configure Fullscreen...", this);
    connect(conf_fullscreen_act, SIGNAL(triggered()), this, SLOT(conf_fullscreen()));
    view_menu->addAction(conf_fullscreen_act);
    QAction* conf_stereo3d_act = new QAction("Configure Stereo 3D...", this);
    connect(conf_stereo3d_act, SIGNAL(triggered()), this, SLOT(conf_stereo3d()));
    conf_stereo3d_act->setEnabled(WITH_GLS);
    view_menu->addAction(conf_stereo3d_act);
    view_menu->addSeparator();
    /*
    QAction* conf_equalizer_act = new QAction("Configure Equalizer...", this);
    connect(conf_equalizer_act, SIGNAL(triggered()), this, SLOT(conf_equalizer()));
    conf_equalizer_act->setEnabled(WITH_EQUALIZER);
    view_menu->addAction(conf_equalizer_act);
    */
    QAction* toggle_equalizer_act = new QAction("Toggle Equalizer", this);
    connect(toggle_equalizer_act, SIGNAL(triggered()), this, SLOT(toggle_equalizer()));
    toggle_equalizer_act->setEnabled(WITH_EQUALIZER);
    view_menu->addAction(toggle_equalizer_act);
    view_menu->addSeparator();
    QAction* copy_current_view_act = new QAction("Copy current view", this);
    copy_current_view_act->setShortcut(QKeySequence::Copy);
    connect(copy_current_view_act, SIGNAL(triggered()), this, SLOT(copy_current_view()));
    view_menu->addAction(copy_current_view_act);
    QAction* save_current_view_act = new QAction("Save current view...", this);
    save_current_view_act->setShortcut(QKeySequence::Save);
    connect(save_current_view_act, SIGNAL(triggered()), this, SLOT(save_current_view()));
    view_menu->addAction(save_current_view_act);
    view_menu->addSeparator();
    QAction *close_action = new QAction(tr("&Close view"), this);
    close_action->setShortcut(QKeySequence::Close);
    connect(close_action, SIGNAL(triggered()), this, SLOT(close()));
    view_menu->addAction(close_action);
    QAction *quit_action = new QAction(tr("&Quit"), this);
    quit_action->setShortcut(QKeySequence::Quit);
    connect(quit_action, SIGNAL(triggered()), this, SLOT(request_quit()));
    view_menu->addAction(quit_action);
}

View::~View()
{
    assert(isHidden()); // this widget must have been closed()!
}

void View::closeEvent(QCloseEvent* event)
{
    if (!isHidden()) { // cleanup only once
        _settings->setValue("view/windowgeometry", saveGeometry());
        _settings->setValue("view/windowstate", saveState());
        assert(_renderloop_timer);
        _renderloop_timer->stop();
#if WITH_EQUALIZER
        if (_eqwidget)
            toggle_equalizer();
#endif
        _glmanager.exit_gl();
        assert(_glwidget);
        _glmanager.remove_window(_glwidget);
        delete _glwidget;
        assert(_mode_widget);
        delete _mode_widget;
        assert(_renderloop_timer);
        delete _renderloop_timer;
        assert(_renderer_factory);
        delete _renderer_factory;
    }
    event->accept();
    emit closed();
}

void View::request_quit()
{
    emit quit();
}

void View::init(
        int* argc, char** argv, QSettings* settings,
        const std::string& file_name, const std::string& save_name,
        const std::vector<gta::header*>& headers,
        const std::vector<off_t>& offsets)
{
    assert(!_renderer_factory); // we can init() only once!

    _argc = argc;
    _argv = argv;
    _settings = settings;

    _file_name = &file_name;
    _save_name = &save_name;
    _headers = &headers;
    _offsets = &offsets;

    _all_compatible = true;
    for (size_t i = 1; _all_compatible && i < headers.size(); i++) {
        if (headers[i]->data_size() != headers[0]->data_size()
                || headers[i]->dimensions() != headers[0]->dimensions()
                || headers[i]->components() != headers[0]->components()) {
            _all_compatible = false;
        }
        for (uintmax_t j = 0; _all_compatible && j < headers[i]->dimensions(); j++) {
            if (headers[i]->dimension_size(j) != headers[0]->dimension_size(j))
                _all_compatible = false;
        }
        for (uintmax_t j = 0; _all_compatible && j < headers[i]->components(); j++) {
            if (headers[i]->component_type(j) != headers[0]->component_type(j)
                    || (headers[i]->component_type(j) == gta::blob
                        && headers[i]->component_size(j) != headers[0]->component_size(j)))
                _all_compatible = false;
        }
    }
    _view_params.resize(_all_compatible ? 1 : headers.size());
    _minmaxhists.resize(headers.size());

    restoreGeometry(_settings->value("view/windowgeometry").toByteArray());
    restoreState(_settings->value("view/windowstate").toByteArray());

    show();

    _renderer_factory = new RendererFactory();
    recreate_views();
    _renderloop_timer = new QTimer(this);
    connect(_renderloop_timer, SIGNAL(timeout()), this, SLOT(renderloop()));
    connect(this, SIGNAL(set_view_params(const ViewParameters&)),
            this, SLOT(update_renderer_view_params(const ViewParameters&)));
    QApplication::processEvents();
    _renderloop_timer->start(0);
}

void View::set_current(size_t index)
{
    msg::dbg("setting GTA array index to %lu", static_cast<unsigned long>(index));
    _index = index;
    const gta::header& hdr = *((*_headers)[_index]);
    try {
        _data.resize(hdr.data_size());
        FILE* f = fio::open(*_save_name, "r");
        fio::seek(f, (*_offsets)[_index], SEEK_SET, *_save_name);
        hdr.read_data(f, _data.ptr());
    }
    catch (std::exception& e) {
        QMessageBox::critical(this, "Error",
                QString("Cannot load GTA data: %1").arg(e.what()));
        close();
    }

    ViewParameters& view_params = _view_params[_all_compatible ? 0 : _index];
    std::string errmsg;
    ViewParameters::mode_t mode = ViewParameters::suggest_mode(hdr, &errmsg);
    if (mode == ViewParameters::mode_null) {
        QMessageBox::critical(this, "Error",
                QString("Cannot view GTA data: %1").arg(errmsg.c_str()));
    }
    bool recreate = (mode != view_params.mode);
    if (!_minmaxhists[_index].valid())
        _minmaxhists[_index].compute(hdr, _data.ptr());
    if (recreate) {
        view_params.set_mode(mode, hdr, _minmaxhists[_index]);
        recreate_views();
    }
    emit set_view_params(view_params);

    std::vector<GLRenderer*> renderers = _glmanager.get_renderers();
    for (size_t i = 0; i < renderers.size(); i++) {
        Renderer* r = dynamic_cast<Renderer*>(renderers[i]);
        r->set_gta(hdr, _data.ptr(), &_minmaxhists[_index]);
    }

    QString window_title = QTextCodec::codecForLocale()->toUnicode(fio::to_sys(fio::basename(*_file_name)).c_str());
    if (_headers->size() > 1)
        window_title += QString(":") + QString::number(_index);
    window_title += QString(" - " PACKAGE_NAME ": View");
    setWindowTitle(window_title);
}

void View::recreate_views()
{
    assert(_view_params.size() >= 1);
    ViewParameters& view_params = _view_params[_all_compatible ? 0 : _index];

    /* First pass: clean up and remove old stuff. */

    std::stringstream renderer_state;
#if WITH_EQUALIZER
    bool reenable_equalizer = _eqwidget;
#endif
    if (_glwidget) {
        s11n::save(renderer_state, *_glwidget->get_shared_context()->get_renderer());
        _glmanager.exit_gl();
        _glmanager.remove_window(_glwidget);
        centralWidget()->layout()->removeWidget(_glwidget);
        delete _glwidget;
#if WITH_EQUALIZER
        if (reenable_equalizer)
            toggle_equalizer(); // switch off
#endif
    }
    _active_glwidget = NULL;
    centralWidget()->layout()->removeWidget(_mode_widget);
    delete _mode_widget;
    _mode_widget = NULL;

    if (!view_params.mode_is_valid())
        return;

    /* Second pass: create the parameter widgets, the render widget and init GL stuff */

    // Get configuration
    int fullscreen_screens = _settings->value("view/fullscreen-screens", "0").toInt();
#if WITH_GLS
    GLSmode gls_mode = static_cast<GLSmode>(_settings->value("view/stereo3d-mode", "13").toInt());
    bool gls_half = _settings->value("view/stereo3d-half", "0").toBool();
    bool gls_swap = _settings->value("view/stereo3d-swap", "0").toBool();
#endif
    QGLFormat::setDefaultFormat(XQGLWidget::get_required_format(
#if WITH_GLS
                gls_mode
#else
                0
#endif
                ));
    // Create parameter widgets and initialize them
    assert(view_params.mode == ViewParameters::mode_2d);
    _mode_widget = new Mode2DWidget(*((*_headers)[_index]), _data.ptr(), _minmaxhists[_index], view_params);
    connect(this, SIGNAL(set_view_params(const ViewParameters&)), _mode_widget, SLOT(update()));
    // Create the GL widget and initialize it
    _glwidget = new GLWidget(_renderer_factory, this);
    if (!renderer_state.str().empty())
        s11n::load(renderer_state, *_glwidget->get_shared_context()->get_renderer());
    connect(this, SIGNAL(set_view_params(const ViewParameters&)), _glwidget, SLOT(set_view_params(const ViewParameters&)));
    connect(_glwidget, SIGNAL(got_focus(XQGLWidget*)), this, SLOT(update_active_glwidget(XQGLWidget*)));
    connect(this, SIGNAL(set_fullscreen_conf(int)), _glwidget, SLOT(set_fullscreen_conf(int)));
    connect(this, SIGNAL(set_stereo3d_conf(int, bool, bool)), _glwidget, SLOT(set_stereo3d_conf(int, bool, bool)));
    connect(_mode_widget, SIGNAL(set_view_params(const ViewParameters&)), _glwidget, SLOT(set_view_params(const ViewParameters&)));
    connect(_mode_widget, SIGNAL(set_view_params(const ViewParameters&)), this, SLOT(update_renderer_view_params(const ViewParameters&)));
#if WITH_GLS
    emit set_stereo3d_conf(gls_mode, gls_half, gls_swap);
#endif
    emit set_fullscreen_conf(fullscreen_screens);
    // Add everyting to the layout
    QGridLayout* layout = static_cast<QGridLayout*>(centralWidget()->layout());
    layout->addWidget(_mode_widget, 0, 0);
    layout->addWidget(_glwidget, 0, 1);
    layout->setColumnStretch(1, 1);
    // Initialize and start
    _glwidget->setFocus(Qt::OtherFocusReason);
    _glmanager.add_window(_glwidget);
#if WITH_EQUALIZER
    if (reenable_equalizer)
        toggle_equalizer(); // switch on again
#endif
    _glmanager.init_gl();
}

void View::update_active_glwidget(XQGLWidget* view)
{
    if (_active_glwidget)
        _active_glwidget->mark_active(false);
    _active_glwidget = static_cast<GLWidget*>(view);
    _active_glwidget->mark_active(true);
}

/* Special actions */

#if QT_VERSION < 0x050000
class SleepThread : public QThread
{
public:
    static void msleep(unsigned long msecs) { QThread::msleep(msecs); }
};
void msleep(unsigned long msecs) { SleepThread::msleep(msecs); }
#else
void msleep(unsigned long msecs) { QThread::msleep(msecs); }
#endif

void View::renderloop()
{
    try {
        _glmanager.update();
        if (!_glmanager.render())
            msleep(10);
    }
    catch (std::exception& e) {
        QMessageBox::critical(this, "Error",
                tr("<p>%1</p>").arg(e.what()));
        dbg::crash();
    }
    if (_glmanager.fps() > 0.0f) {
        //msg::dbg("FPS: %f", _glmanager.fps());
    }
}

void View::update_renderer_view_params(const ViewParameters& view_params)
{
    std::vector<GLRenderer*> renderers = _glmanager.get_renderers();
    for (size_t i = 0; i < renderers.size(); i++) {
        Renderer* r = dynamic_cast<Renderer*>(renderers[i]);
        r->set_view_params(view_params);
    }
}

/* Menu actions */

void View::conf_fullscreen()
{
    int n = QApplication::desktop()->screenCount();
    QDialog *dlg = new QDialog(this);
    dlg->setModal(false);
    dlg->setWindowTitle("Fullscreen/Multiscreen Settings");
    QLabel *lbl = new QLabel("Configure fullscreen mode:");
    QRadioButton *single_btn = new QRadioButton("Single screen:");
    QComboBox *single_box = new QComboBox();
    single_box->addItem("Primary screen");
    if (n > 1) {
        for (int i = 0; i < n; i++) {
            single_box->addItem(str::asprintf("Screen %d", i + 1).c_str());
        }
    }
    QRadioButton *dual_btn = new QRadioButton("Dual screen:");
    QComboBox *dual_box0 = new QComboBox();
    QComboBox *dual_box1 = new QComboBox();
    if (n > 1) {
        for (int i = 0; i < n; i++) {
            dual_box0->addItem(str::asprintf("Screen %d", i + 1).c_str());
            dual_box1->addItem(str::asprintf("Screen %d", i + 1).c_str());
        }
    }
    QRadioButton *multi_btn = new QRadioButton("Multi screen:");
    QLineEdit *multi_edt = new QLineEdit();
    QRegExp rx("\\d{1,2}(,\\d{1,2}){0,15}");
    multi_edt->setValidator(new QRegExpValidator(rx, 0));
    QPushButton *cancel_btn = new QPushButton("Cancel");
    QPushButton *ok_btn = new QPushButton("OK");
    ok_btn->setDefault(true);
    connect(cancel_btn, SIGNAL(pressed()), dlg, SLOT(reject()));
    connect(ok_btn, SIGNAL(pressed()), dlg, SLOT(accept()));
    QGridLayout *layout0 = new QGridLayout();
    layout0->addWidget(lbl, 0, 0, 1, 3);
    layout0->addWidget(single_btn, 1, 0);
    layout0->addWidget(single_box, 1, 1, 1, 2);
    layout0->addWidget(dual_btn, 2, 0);
    layout0->addWidget(dual_box0, 2, 1);
    layout0->addWidget(dual_box1, 2, 2);
    layout0->addWidget(multi_btn, 3, 0);
    layout0->addWidget(multi_edt, 3, 1, 1, 2);
    QGridLayout *layout1 = new QGridLayout();
    layout1->addWidget(cancel_btn, 0, 0);
    layout1->addWidget(ok_btn, 0, 1);
    QGridLayout *layout = new QGridLayout();
    layout->addLayout(layout0, 0, 0);
    layout->addLayout(layout1, 1, 0);
    dlg->setLayout(layout);

    // Set initial values
    if (n < 3) {
        multi_btn->setEnabled(false);
        multi_edt->setEnabled(false);
    } else {
        multi_edt->setText("1,2,3");
    }
    if (n < 2) {
        dual_btn->setEnabled(false);
        dual_box0->setEnabled(false);
        dual_box1->setEnabled(false);
    } else {
        dual_box0->setCurrentIndex(0);
        dual_box1->setCurrentIndex(1);
    }
    int fullscreen_screens = _settings->value("view/fullscreen-screens", "0").toInt();
    std::vector<int> conf_screens;
    for (int i = 0; i < 16; i++) {
        if (fullscreen_screens & (1 << i)) {
            conf_screens.push_back(i);
        }
    }
    if (conf_screens.size() >= 3 && n >= 3) {
        QString screen_list;
        for (size_t i = 0; i < conf_screens.size(); i++) {
            screen_list += str::from(conf_screens[i] + 1).c_str();
            if (i < conf_screens.size() - 1) {
                screen_list += ',';
            }
        }
        multi_btn->setChecked(true);
        multi_edt->setText(screen_list);
    } else if (conf_screens.size() == 2 && n >= 2) {
        dual_box0->setCurrentIndex(conf_screens[0]);
        dual_box1->setCurrentIndex(conf_screens[1]);
        dual_btn->setChecked(true);
    } else {
        if (conf_screens.size() > 0 && conf_screens[0] < n) {
            single_box->setCurrentIndex(conf_screens[0] + 1);
        } else {
            single_box->setCurrentIndex(0);
        }
        single_btn->setChecked(true);
    }

    // Run dialog
    dlg->exec();
    if (dlg->result() == QDialog::Accepted) {
        if (single_btn->isChecked()) {
            if (single_box->currentIndex() == 0) {
                fullscreen_screens = 0;
            } else {
                fullscreen_screens = (1 << (single_box->currentIndex() - 1));
            }
        } else if (dual_btn->isChecked()) {
            fullscreen_screens = (1 << dual_box0->currentIndex());
            fullscreen_screens |= (1 << dual_box1->currentIndex());
        } else {
            fullscreen_screens = 0;
            QStringList screens = multi_edt->text().split(',', QString::SkipEmptyParts);
            for (int i = 0; i < screens.size(); i++) {
                int s = str::to<int>(screens[i].toUtf8().data());
                if (s >= 1 && s <= 16) {
                    fullscreen_screens |= (1 << (s - 1));
                }
            }
        }
        _settings->setValue("view/fullscreen-screens", fullscreen_screens);
        emit set_fullscreen_conf(fullscreen_screens);
    }
}

void View::conf_stereo3d()
{
#if WITH_GLS
    GLSmode gls_mode = static_cast<GLSmode>(_settings->value("view/stereo3d-mode", "13").toInt());
    bool gls_half = _settings->value("view/stereo3d-half", "0").toBool();
    bool gls_swap = _settings->value("view/stereo3d-swap", "0").toBool();

    QDialog* dlg = new QDialog(this);
    dlg->setModal(false);
    dlg->setWindowTitle("Stereo 3D Settings");
    QLabel* mode_lbl = new QLabel("Mode:");
    QComboBox* mode_box = new QComboBox();
    mode_box->addItem(QIcon(":icons-local/output-type-mono-left.png"), ("Left view only"));
    mode_box->addItem(QIcon(":icons-local/output-type-mono-right.png"), ("Right view only"));
    mode_box->addItem(QIcon(":icons-local/output-type-stereo.png"), ("OpenGL quad-buffered stereo"));
    mode_box->addItem(QIcon(":icons-local/output-type-alternating.png"), ("Left/right alternating"));
    mode_box->addItem(QIcon(":icons-local/output-type-top-bottom.png"), ("Top/bottom"));
    mode_box->addItem(QIcon(":icons-local/output-type-top-bottom-half.png"), ("Top/bottom, half height"));
    mode_box->addItem(QIcon(":icons-local/output-type-left-right.png"), ("Left/right"));
    mode_box->addItem(QIcon(":icons-local/output-type-left-right-half.png"), ("Left/right, half width"));
    mode_box->addItem(QIcon(":icons-local/output-type-even-odd-rows.png"), ("Even/odd rows"));
    mode_box->addItem(QIcon(":icons-local/output-type-even-odd-columns.png"), ("Even/odd columns"));
    mode_box->addItem(QIcon(":icons-local/output-type-checkerboard.png"), ("Checkerboard pattern"));
    mode_box->addItem(QIcon(":icons-local/output-type-hdmi-frame-pack.png"), ("HDMI frame packing mode"));
    mode_box->addItem(QIcon(":icons-local/output-type-red-cyan.png"), ("Red/cyan glasses, monochrome"));
    mode_box->addItem(QIcon(":icons-local/output-type-red-cyan.png"), ("Red/cyan glasses, half color"));
    mode_box->addItem(QIcon(":icons-local/output-type-red-cyan.png"), ("Red/cyan glasses, full color"));
    mode_box->addItem(QIcon(":icons-local/output-type-red-cyan.png"), ("Red/cyan glasses, Dubois"));
    mode_box->addItem(QIcon(":icons-local/output-type-green-magenta.png"), ("Green/magenta glasses, monochrome"));
    mode_box->addItem(QIcon(":icons-local/output-type-green-magenta.png"), ("Green/magenta glasses, half color"));
    mode_box->addItem(QIcon(":icons-local/output-type-green-magenta.png"), ("Green/magenta glasses, full color"));
    mode_box->addItem(QIcon(":icons-local/output-type-green-magenta.png"), ("Green/magenta glasses, Dubois"));
    mode_box->addItem(QIcon(":icons-local/output-type-amber-blue.png"), ("Amber/blue glasses, monochrome"));
    mode_box->addItem(QIcon(":icons-local/output-type-amber-blue.png"), ("Amber/blue glasses, half color"));
    mode_box->addItem(QIcon(":icons-local/output-type-amber-blue.png"), ("Amber/blue glasses, full color"));
    mode_box->addItem(QIcon(":icons-local/output-type-amber-blue.png"), ("Amber/blue glasses, Dubois"));
    mode_box->addItem(QIcon(":icons-local/output-type-red-green.png"), ("Red/green glasses, monochrome"));
    mode_box->addItem(QIcon(":icons-local/output-type-red-blue.png"), ("Red/blue glasses, monochrome"));
    switch (gls_mode) {
    case GLS_MODE_QUAD_BUFFER_STEREO:
        mode_box->setCurrentIndex(2);
        break;
    case GLS_MODE_ALTERNATING:
        mode_box->setCurrentIndex(3);
        break;
    case GLS_MODE_MONO_LEFT:
        mode_box->setCurrentIndex(0);
        break;
    case GLS_MODE_MONO_RIGHT:
        mode_box->setCurrentIndex(1);
        break;
    case GLS_MODE_LEFT_RIGHT:
        mode_box->setCurrentIndex(gls_half ? 7 : 6);
        break;
    case GLS_MODE_TOP_BOTTOM:
        mode_box->setCurrentIndex(gls_half ? 5 : 4);
        break;
    case GLS_MODE_HDMI_FRAME_PACK:
        mode_box->setCurrentIndex(11);
        break;
    case GLS_MODE_EVEN_ODD_ROWS:
        mode_box->setCurrentIndex(8);
        break;
    case GLS_MODE_EVEN_ODD_COLUMNS:
        mode_box->setCurrentIndex(9);
        break;
    case GLS_MODE_CHECKERBOARD:
        mode_box->setCurrentIndex(10);
        break;
    case GLS_MODE_RED_CYAN_MONOCHROME:
        mode_box->setCurrentIndex(12);
        break;
    case GLS_MODE_RED_CYAN_HALF_COLOR:
        mode_box->setCurrentIndex(13);
        break;
    case GLS_MODE_RED_CYAN_FULL_COLOR:
        mode_box->setCurrentIndex(14);
        break;
    case GLS_MODE_RED_CYAN_DUBOIS:
        mode_box->setCurrentIndex(15);
        break;
    case GLS_MODE_GREEN_MAGENTA_MONOCHROME:
        mode_box->setCurrentIndex(16);
        break;
    case GLS_MODE_GREEN_MAGENTA_HALF_COLOR:
        mode_box->setCurrentIndex(17);
        break;
    case GLS_MODE_GREEN_MAGENTA_FULL_COLOR:
        mode_box->setCurrentIndex(18);
        break;
    case GLS_MODE_GREEN_MAGENTA_DUBOIS:
        mode_box->setCurrentIndex(19);
        break;
    case GLS_MODE_AMBER_BLUE_MONOCHROME:
        mode_box->setCurrentIndex(20);
        break;
    case GLS_MODE_AMBER_BLUE_HALF_COLOR:
        mode_box->setCurrentIndex(21);
        break;
    case GLS_MODE_AMBER_BLUE_FULL_COLOR:
        mode_box->setCurrentIndex(22);
        break;
    case GLS_MODE_AMBER_BLUE_DUBOIS:
        mode_box->setCurrentIndex(23);
        break;
    case GLS_MODE_RED_GREEN_MONOCHROME:
        mode_box->setCurrentIndex(24);
        break;
    case GLS_MODE_RED_BLUE_MONOCHROME:
        mode_box->setCurrentIndex(25);
        break;
    }
    {
        // Check if we have quad buffer stereo support
        QGLFormat fmt(QGLFormat::defaultFormat());
        fmt.setStereo(true);
        QGLWidget* tmpwidget = new QGLWidget(fmt);
        bool have_stereo = tmpwidget->format().stereo();
        delete tmpwidget;
        if (!have_stereo) {
            qobject_cast<QStandardItemModel*>(mode_box->model())->item(2)->setEnabled(false);
        }
    }
    QCheckBox* swap_box = new QCheckBox("Swap eyes");
    swap_box->setChecked(gls_swap);
    QPushButton *cancel_btn = new QPushButton("Cancel");
    QPushButton *ok_btn = new QPushButton("OK");
    ok_btn->setDefault(true);
    connect(cancel_btn, SIGNAL(pressed()), dlg, SLOT(reject()));
    connect(ok_btn, SIGNAL(pressed()), dlg, SLOT(accept()));
    QGridLayout* layout0 = new QGridLayout();
    layout0->addWidget(mode_lbl, 0, 0);
    layout0->addWidget(mode_box, 0, 1);
    QGridLayout* layout1 = new QGridLayout();
    layout1->addWidget(swap_box, 0, 0);
    QGridLayout* layout2 = new QGridLayout();
    layout2->addWidget(cancel_btn, 0, 0);
    layout2->addWidget(ok_btn, 0, 1);
    QGridLayout* layout = new QGridLayout();
    layout->addLayout(layout0, 0, 0);
    layout->addLayout(layout1, 1, 0);
    layout->addLayout(layout2, 2, 0);
    dlg->setLayout(layout);

    dlg->exec();
    if (dlg->result() == QDialog::Accepted) {
        switch (mode_box->currentIndex()) {
        case 0:
            gls_mode = GLS_MODE_MONO_LEFT; gls_half = false;
            break;
        case 1:
            gls_mode = GLS_MODE_MONO_RIGHT; gls_half = false;
            break;
        case 2:
            gls_mode = GLS_MODE_QUAD_BUFFER_STEREO; gls_half = false;
            break;
        case 3:
            gls_mode = GLS_MODE_ALTERNATING; gls_half = false;
            break;
        case 4:
            gls_mode = GLS_MODE_TOP_BOTTOM; gls_half = false;
            break;
        case 5:
            gls_mode = GLS_MODE_TOP_BOTTOM; gls_half = true;
            break;
        case 6:
            gls_mode = GLS_MODE_LEFT_RIGHT; gls_half = false;
            break;
        case 7:
            gls_mode = GLS_MODE_LEFT_RIGHT; gls_half = true;
            break;
        case 8:
            gls_mode = GLS_MODE_EVEN_ODD_ROWS; gls_half = false;
            break;
        case 9:
            gls_mode = GLS_MODE_EVEN_ODD_COLUMNS; gls_half = false;
            break;
        case 10:
            gls_mode = GLS_MODE_CHECKERBOARD; gls_half = false;
            break;
        case 11:
            gls_mode = GLS_MODE_HDMI_FRAME_PACK; gls_half = false;
            break;
        case 12:
            gls_mode = GLS_MODE_RED_CYAN_MONOCHROME; gls_half = false;
            break;
        case 13:
            gls_mode = GLS_MODE_RED_CYAN_HALF_COLOR; gls_half = false;
            break;
        case 14:
            gls_mode = GLS_MODE_RED_CYAN_FULL_COLOR; gls_half = false;
            break;
        case 15:
            gls_mode = GLS_MODE_RED_CYAN_DUBOIS; gls_half = false;
            break;
        case 16:
            gls_mode = GLS_MODE_GREEN_MAGENTA_MONOCHROME; gls_half = false;
            break;
        case 17:
            gls_mode = GLS_MODE_GREEN_MAGENTA_HALF_COLOR; gls_half = false;
            break;
        case 18:
            gls_mode = GLS_MODE_GREEN_MAGENTA_FULL_COLOR; gls_half = false;
            break;
        case 19:
            gls_mode = GLS_MODE_GREEN_MAGENTA_DUBOIS; gls_half = false;
            break;
        case 20:
            gls_mode = GLS_MODE_AMBER_BLUE_MONOCHROME; gls_half = false;
            break;
        case 21:
            gls_mode = GLS_MODE_AMBER_BLUE_HALF_COLOR; gls_half = false;
            break;
        case 22:
            gls_mode = GLS_MODE_AMBER_BLUE_FULL_COLOR; gls_half = false;
            break;
        case 23:
            gls_mode = GLS_MODE_AMBER_BLUE_DUBOIS; gls_half = false;
            break;
        case 24:
            gls_mode = GLS_MODE_RED_GREEN_MONOCHROME; gls_half = false;
            break;
        case 25:
            gls_mode = GLS_MODE_RED_BLUE_MONOCHROME; gls_half = false;
            break;
        }
        gls_swap = swap_box->isChecked();
        _settings->setValue("view/stereo3d-mode", static_cast<int>(gls_mode));
        _settings->setValue("view/stereo3d-half", gls_half);
        _settings->setValue("view/stereo3d-swap", gls_swap);
        if (XQGLWidget::get_required_format(gls_mode) != QGLFormat::defaultFormat())
            recreate_views();
        else
            emit set_stereo3d_conf(gls_mode, gls_half, gls_swap);
    }
#endif
}

void View::conf_equalizer()
{
#if WITH_EQUALIZER
#endif
}

void View::toggle_equalizer()
{
#if WITH_EQUALIZER
    try {
        if (_eqwidget) {
            _glmanager.remove_window(_eqwidget);
            delete _eqwidget;
            _eqwidget = NULL;
        } else {
            _eqwidget = new EQWindow(_renderer_factory, _glwidget, 0, _argc, _argv);
            _glmanager.add_window(_eqwidget);
            std::stringstream renderer_state;
            s11n::save(renderer_state, *_glwidget->get_shared_context()->get_renderer());
            s11n::load(renderer_state, *_eqwidget->get_shared_context()->get_renderer());
        }
    }
    catch (std::exception& e) {
        QMessageBox::critical(this, "Error", e.what());
        dbg::crash();
    }
#endif
}

void View::copy_current_view()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QImage* img = _active_glwidget->get_current_image();
    QApplication::clipboard()->setImage(*img);
    delete img;
    QApplication::restoreOverrideCursor();
}

void View::save_image(QImage* img)
{
    QFileDialog* file_dialog = new QFileDialog(this);
    QDir last_dir = QDir(_settings->value("general/last-dir").toString());
    if (last_dir.exists())
        file_dialog->setDirectory(last_dir);
    file_dialog->setWindowTitle(tr("Save image"));
    file_dialog->setAcceptMode(QFileDialog::AcceptSave);
    file_dialog->setFileMode(QFileDialog::AnyFile);
    file_dialog->setDefaultSuffix("png");
    QStringList filters;
    filters << tr("PNG images (*.png)") << tr("All files (*)");
    file_dialog->setNameFilters(filters);
    if (!file_dialog->exec() || file_dialog->selectedFiles().empty())
        return;

    QString file_name = file_dialog->selectedFiles().at(0);
    _settings->setValue("general/last-dir", file_dialog->directory().path());

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    bool success = img->save(file_name, "png");
    QApplication::restoreOverrideCursor();
    if (!success)
        QMessageBox::critical(this, tr("Error"), QString(tr("Saving %1 failed.")).arg(file_name));
}

void View::save_current_view()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QImage* img = _active_glwidget->get_current_image();
    QApplication::restoreOverrideCursor();
    if (!img->isNull())
        save_image(img);
    delete img;
}
