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

#ifndef VIEW_H
#define VIEW_H

#include "config.h"

#include <gta/gta.hpp>

#include <GL/glew.h>

#if WITH_GLS
# include <gls/gls.h>
#else
# define GLSmode int
#endif

#include "xgl/glmanager.hpp"
#include "renderer.hpp"
#include "viewparams.hpp"
#if WITH_EQUALIZER
# include "eq/eqwindow.hpp"
#endif
#include "mode_2d_widget.hpp"

#include "base/blb.h"

#include "../gui/viewwidget.hpp"
#include "minmaxhist.hpp"

class QImage;
class QSettings;
class QTimer;
class XQGLWidget;
class GLWidget;


class View : public ViewWidget
{
Q_OBJECT

private:
    int* _argc;
    char** _argv;
    QSettings* _settings;

    // Information about all arrays in the stream:
    const std::string* _file_name;              // displayed filename of stream
    const std::string* _save_name;              // filename of stream on disk
    const std::vector<gta::header*>* _headers;  // GTA headers
    const std::vector<off_t>* _offsets;         // Offset of data inside stream
    bool _all_compatible;                       // Are all arrays of the same form?
    std::vector<ViewParameters> _view_params;   // View parameters for arrays (only one if all_compatible!)
    std::vector<MinMaxHist> _minmaxhists;       // Min, max, and histogram for all components of all arrays

    // Information about the current array within the stream:
    size_t _index;                              // Index of array within stream
    blob _data;                                 // Data of the current array

    // Parameter management
    Mode2DWidget* _mode_widget;

    // GL management
    GLManager _glmanager;
    RendererFactory* _renderer_factory;
    GLWidget* _glwidget;
#if WITH_EQUALIZER
    EQWindow* _eqwidget;
#endif
    GLWidget* _active_glwidget;
    QTimer* _renderloop_timer;

private slots:
    void update_active_glwidget(XQGLWidget* view);
    void recreate_views();

protected:
    virtual void closeEvent(QCloseEvent* event);

public:
    View();
    virtual ~View();

private:
    // Helper functions
    void save_image(QImage* img);

private slots:
    // Special actions
    void renderloop();
    void update_renderer_view_params(const ViewParameters& view_params);
    // Menu actions: View
    void conf_fullscreen();
    void conf_stereo3d();
    void conf_equalizer();
    void toggle_equalizer();
    void copy_current_view();
    void save_current_view();
    void request_quit();

signals:
    void set_fullscreen_conf(int screens);
    void set_stereo3d_conf(int mode, bool half, bool swap);
    void set_view_params(const ViewParameters& view_params);
    void quit();

public:
    virtual void init(
            int* argc, char** argv, QSettings* settings,
            const std::string& file_name, const std::string& save_name,
            const std::vector<gta::header*>& headers,
            const std::vector<off_t>& offsets);

    virtual void set_current(size_t index);
};

#endif
