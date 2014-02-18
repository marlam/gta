/*
 * Copyright (C) 2007, 2008, 2014
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

#include <cstdlib>
#include <cmath>
#include <cerrno>

#include <limits>

#include <QComboBox>
#include <QGridLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QToolBox>
#include <QGroupBox>
#include <QRadioButton>
#if QT_VERSION < 0x050000
# include <QApplication>
#else
# include <QGuiApplication>
#endif

#include "xgl/glvm.hpp"

#include "base/dbg.h"
#include "base/msg.h"
#include "base/str.h"

#include "mode_2d_widget.hpp"


/* Helper function for Qt4/Qt5 differences */

static QPalette ApplicationPalette()
{
#if QT_VERSION < 0x050000
    return QApplication::palette();
#else
    return QGuiApplication::palette();
#endif
}

/* Helper classes */

QValidator::State FloatSpinBox::validate(QString& input, int& /* pos */) const
{
    std::string s = qPrintable(input);
    errno = 0;
    char* e;
    float x = std::strtof(s.c_str(), &e);
    if (e == s || *e != '\0' || errno != 0 || !glvm::isfinite(x))
        return QValidator::Intermediate;
    else
        return QValidator::Acceptable;
}

QString FloatSpinBox::textFromValue(double value) const
{
    return str::asprintf("%g", value).c_str();
}

double FloatSpinBox::valueFromText(const QString& text) const
{
    return std::strtof(qPrintable(text), NULL);
}

RangeSelector::RangeSelector(Mode2DWidget* mode_2d_widget) : QLabel(), _mode_2d_widget(mode_2d_widget)
{
    setMouseTracking(true);
    QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sp.setHeightForWidth(false);
    setSizePolicy(sp);

    int tolerance = 4;
    _tolerance_normalized = static_cast<float>(tolerance) / static_cast<float>(width() - 1);
    _allow_change_left = false;
    _allow_change_right = false;
    _allow_drag = false;
    _change_left = false;
    _change_right = false;
    _drag = false;
    _range_left_normalized = -1.0f;
    _range_right_normalized = +2.0f;
}

QSize RangeSelector::sizeHint() const
{
    return QSize(1, 50);
}

float RangeSelector::logtransf(float x)
{
    const float base = 250.0f;
    return glvm::clamp(glvm::log(1.0f + x * (base - 1.0f)) / glvm::log(base), 0.0f, 1.0f);
}

float RangeSelector::invlogtransf(float y)
{
    const float base = 250.0f;
    return glvm::clamp((glvm::exp(y * glvm::log(base)) - 1.0f) / (base - 1.0f), 0.0f, 1.0f);
}

float RangeSelector::normalized_x_to_rangeval(float x)
{
    int component = _mode_2d_widget->_view_params.mode_2d_global.component;
    float lowerbound = _mode_2d_widget->_view_params.mode_2d_components[component].minval;
    float upperbound = _mode_2d_widget->_view_params.mode_2d_components[component].maxval;
    bool log_x = _mode_2d_widget->_range_log_x_box->isChecked();
    return (1.0f - (log_x ? logtransf(1.0f - x) : 1.0f - x))
        * (upperbound - lowerbound) + lowerbound;
}

float RangeSelector::rangeval_to_normalized_x(float x)
{
    int component = _mode_2d_widget->_view_params.mode_2d_global.component;
    float lowerbound = _mode_2d_widget->_view_params.mode_2d_components[component].minval;
    float upperbound = _mode_2d_widget->_view_params.mode_2d_components[component].maxval;
    bool log_x = _mode_2d_widget->_range_log_x_box->isChecked();
    float r = (x - lowerbound) / (upperbound - lowerbound);
    return log_x ? (1.0f - invlogtransf(1.0f - r)) : r;
}

void RangeSelector::paintEvent(QPaintEvent* /* e */)
{
    QPainter p(this);
    p.fillRect(1, 1, width() - 2, height() - 2, ApplicationPalette().brush(QPalette::Base));
    p.setPen(ApplicationPalette().color(QPalette::Text));
    p.drawRect(0, 0, width() - 1, height() - 1);

    /* Get the data */
    int component = _mode_2d_widget->_view_params.mode_2d_global.component;
    const int* histogram;
    int histsize;
    int histmax;
    float hist_minval, hist_maxval;
    if (static_cast<unsigned int>(component) == _mode_2d_widget->_hdr.components()) {
        histogram = _mode_2d_widget->_view_params.mode_2d_global.lum_histogram;
        histsize = sizeof(_mode_2d_widget->_view_params.mode_2d_global.lum_histogram)
            / sizeof(_mode_2d_widget->_view_params.mode_2d_global.lum_histogram[0]);
        histmax = _mode_2d_widget->_view_params.mode_2d_global.lum_histogram_maxval;
        hist_minval = _mode_2d_widget->_view_params.mode_2d_global.lum_minval;
        hist_maxval = _mode_2d_widget->_view_params.mode_2d_global.lum_maxval;
    } else {
        histogram = _mode_2d_widget->_minmaxhist.histograms[component].data();
        histsize = _mode_2d_widget->_minmaxhist.histograms[component].size();
        histmax = _mode_2d_widget->_minmaxhist.histogram_maxvals[component];
        hist_minval = _mode_2d_widget->_minmaxhist.minvals[component];
        hist_maxval = _mode_2d_widget->_minmaxhist.maxvals[component];
    }
    float minval = _mode_2d_widget->_view_params.mode_2d_components[component].minval;
    float maxval = _mode_2d_widget->_view_params.mode_2d_components[component].maxval;
    float range_min = _mode_2d_widget->_view_params.mode_2d_components[component].range_min;
    float range_max = _mode_2d_widget->_view_params.mode_2d_components[component].range_max;
    bool log_x = _mode_2d_widget->_range_log_x_box->isChecked();
    bool log_y = _mode_2d_widget->_range_log_y_box->isChecked();

    /* Draw selected range */
    _range_left_normalized = rangeval_to_normalized_x(range_min);
    _range_right_normalized = rangeval_to_normalized_x(range_max);
    int range_left = 1 + static_cast<int>(static_cast<float>(width() - 3) * _range_left_normalized);
    int range_right = 1 + static_cast<int>(static_cast<float>(width() - 3) * _range_right_normalized);
    p.fillRect(range_left, 1, range_right - range_left + 1, height() - 2,
            ApplicationPalette().brush(QPalette::QPalette::Highlight));

    /* Draw histogram */
    p.setPen(ApplicationPalette().color(QPalette::Text));
    int last_x = 0;
    for (int bin = 0; bin < histsize; bin++) {
        float binf = bin / static_cast<float>(histsize - 1);
        if (log_x)
            binf = logtransf(binf);
        float binval = hist_minval + binf * (hist_maxval - hist_minval);
        if (binval >= minval && binval <= maxval) {
            float xf = (binval - minval) / (maxval - minval);
            int x = 1 + glvm::round(xf * static_cast<float>(width() - 3));
            float lengthf = histogram[bin] / static_cast<float>(histmax);
            int length = static_cast<int>((log_y ? logtransf(lengthf) : lengthf) * static_cast<float>(height() - 2));
            if (length > 0) {
                assert(x >= last_x);
                if (x == last_x) {
                    p.drawLine(x, height() - 2 - (length - 1), x, height() - 2);
                } else {
                    for (int xx = last_x + 1; xx <= x; xx++)
                        p.drawLine(xx, height() - 2 - (length - 1), xx, height() - 2);
                }
            }
            last_x = x;
        }
    }
}

void RangeSelector::mouseMoveEvent(QMouseEvent* e)
{
    if (_change_left) {
        int endpoint = e->x();
        int offset = endpoint - _startpoint;
        float offset_normalized = static_cast<float>(offset) / static_cast<float>(width() - 3);
        if (_range_left_normalized + offset_normalized >= _range_right_normalized - _tolerance_normalized)
            offset_normalized = _range_right_normalized - _tolerance_normalized - _range_left_normalized;
        else if (_range_left_normalized + offset_normalized < 0.0f)
            offset_normalized = - _range_left_normalized;
        float new_range_min = normalized_x_to_rangeval(_range_left_normalized + offset_normalized);
        int component = _mode_2d_widget->_view_params.mode_2d_global.component;
        _mode_2d_widget->_view_params.mode_2d_components[component].range_min = new_range_min;
        _mode_2d_widget->range_changed();
        _startpoint = endpoint;
    } else if (_change_right) {
        int endpoint = e->x();
        int offset = endpoint - _startpoint;
        float offset_normalized = static_cast<float>(offset) / static_cast<float>(width() - 3);
        if (_range_right_normalized + offset_normalized <= _range_left_normalized + _tolerance_normalized)
            offset_normalized = _range_left_normalized + _tolerance_normalized - _range_right_normalized;
        else if (_range_right_normalized + offset_normalized > 1.0f)
            offset_normalized = 1.0f - _range_right_normalized;
        float new_range_max = normalized_x_to_rangeval(_range_right_normalized + offset_normalized);
        int component = _mode_2d_widget->_view_params.mode_2d_global.component;
        _mode_2d_widget->_view_params.mode_2d_components[component].range_max = new_range_max;
        _mode_2d_widget->range_changed();
        _startpoint = endpoint;
    } else if (_drag) {
        int endpoint = e->x();
        int offset = endpoint - _startpoint;
        float offset_normalized = static_cast<float>(offset) / static_cast<float>(width() - 3);
        if (_range_left_normalized + offset_normalized < 0.0f)
            offset_normalized = - _range_left_normalized;
        else if (_range_right_normalized + offset_normalized > 1.0f)
            offset_normalized = 1.0f - _range_right_normalized;
        float new_range_min = normalized_x_to_rangeval(_range_left_normalized + offset_normalized);
        float new_range_max = normalized_x_to_rangeval(_range_right_normalized + offset_normalized);
        int component = _mode_2d_widget->_view_params.mode_2d_global.component;
        _mode_2d_widget->_view_params.mode_2d_components[component].range_min = new_range_min;
        _mode_2d_widget->_view_params.mode_2d_components[component].range_max = new_range_max;
        _mode_2d_widget->range_changed();
        _startpoint = endpoint;
    } else {
        float x_normalized = static_cast<float>(e->x()) / static_cast<float>(width() - 3);
        if (glvm::abs(x_normalized - _range_left_normalized) <= _tolerance_normalized) {
            _allow_change_left = true;
            _allow_change_right = false;
            _allow_drag = false;
            setCursor(QCursor(Qt::SplitHCursor));
        } else if (glvm::abs(x_normalized - _range_right_normalized) <= _tolerance_normalized) {
            _allow_change_left = false;
            _allow_change_right = true;
            _allow_drag = false;
            setCursor(QCursor(Qt::SplitHCursor));
        } else if (x_normalized >= _range_left_normalized && x_normalized <= _range_right_normalized) {
            _allow_change_left = false;
            _allow_change_right = false;
            _allow_drag = true;
            setCursor(QCursor(Qt::SizeHorCursor));
        } else {
            _allow_change_left = false;
            _allow_change_right = false;
            _allow_drag = false;
            unsetCursor();
        }
    }
    update();
}

void RangeSelector::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        _startpoint = e->x();
        if (_allow_change_left)
            _change_left = true;
        else if (_allow_change_right)
            _change_right = true;
        else if (_allow_drag)
            _drag = true;
    }
}

void RangeSelector::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        _change_left = false;
        _change_right = false;
        _drag = false;
    }
}

void RangeSelector::wheelEvent(QWheelEvent* e)
{
    float steps = static_cast<float>(e->delta() / 120);
    float new_range_left_normalized, new_range_right_normalized;
    if (steps < 0.0f) {
        new_range_left_normalized = glvm::max(0.0f, _range_left_normalized + steps * 0.05f);
        new_range_right_normalized = glvm::min(1.0f, _range_right_normalized - steps * 0.05f);
    } else {
        new_range_left_normalized = _range_left_normalized + steps * 0.05f;
        new_range_right_normalized = _range_right_normalized - steps * 0.05f;
        if (new_range_left_normalized + _tolerance_normalized > new_range_right_normalized - _tolerance_normalized) {
            float center = (_range_right_normalized - _range_left_normalized) / 2.0f + _range_left_normalized;
            new_range_left_normalized = center - _tolerance_normalized;
            new_range_right_normalized = center + _tolerance_normalized;
        }
    }
    float new_range_min = normalized_x_to_rangeval(new_range_left_normalized);
    float new_range_max = normalized_x_to_rangeval(new_range_right_normalized);
    int component = _mode_2d_widget->_view_params.mode_2d_global.component;
    _mode_2d_widget->_view_params.mode_2d_components[component].range_min = new_range_min;
    _mode_2d_widget->_view_params.mode_2d_components[component].range_max = new_range_max;
    _mode_2d_widget->range_changed();
}

void RangeSelector::update_bounds()
{
    int component = _mode_2d_widget->_view_params.mode_2d_global.component;
    float lowerbound = _mode_2d_widget->_view_params.mode_2d_components[component].minval;
    float upperbound = _mode_2d_widget->_view_params.mode_2d_components[component].maxval;
    float range_min = _mode_2d_widget->_view_params.mode_2d_components[component].range_min;
    float range_max = _mode_2d_widget->_view_params.mode_2d_components[component].range_max;

    if (range_min < lowerbound)
        range_min = lowerbound;
    if (range_max > upperbound)
        range_max = upperbound;
    float range_left_normalized = rangeval_to_normalized_x(range_min);
    float range_right_normalized = rangeval_to_normalized_x(range_max);
    if (range_left_normalized + _tolerance_normalized > range_right_normalized - _tolerance_normalized) {
        float center = glvm::abs(range_right_normalized - range_left_normalized) / 2.0f + range_left_normalized;
        if (center < _tolerance_normalized) {
            range_left_normalized = 0.0f;
            range_right_normalized = 2.0f * _tolerance_normalized;
        } else if (center > 1.0f - _tolerance_normalized) {
            range_left_normalized = 1.0f - 2.0f * _tolerance_normalized;
            range_right_normalized = 1.0f;
        } else {
            range_left_normalized = center - _tolerance_normalized;
            range_right_normalized = center + _tolerance_normalized;
        }
    }
    float new_range_min = normalized_x_to_rangeval(range_left_normalized);
    float new_range_max = normalized_x_to_rangeval(range_right_normalized);
    _mode_2d_widget->_view_params.mode_2d_components[component].range_min = new_range_min;
    _mode_2d_widget->_view_params.mode_2d_components[component].range_max = new_range_max;
    _mode_2d_widget->range_changed();
}


/* Main class */

Mode2DWidget::Mode2DWidget(const gta::header& hdr, const void* data, const MinMaxHist& minmaxhist, ViewParameters& view_params) : QWidget(),
    _hdr(hdr), _data(data), _minmaxhist(minmaxhist), _view_params(view_params), _lock(false)
{
    QGroupBox* component_selector = new QGroupBox("Component");
    _component_selector_box = new QComboBox;
    connect(_component_selector_box, SIGNAL(currentIndexChanged(int)), this, SLOT(component_changed(int)));
    QGridLayout* component_selector_layout = new QGridLayout;
    component_selector_layout->addWidget(_component_selector_box, 0, 0);
    component_selector->setLayout(component_selector_layout);

    QWidget* range_selector = new QWidget;
    _range_reset_button = new QPushButton("Reset");
    connect(_range_reset_button, SIGNAL(clicked()), this, SLOT(range_bounds_reset()));
    _range_lo_spinbox = new FloatSpinBox;
    _range_lo_spinbox->setRange(-std::numeric_limits<float>::max(), +std::numeric_limits<float>::max());
    _range_lo_spinbox->setMaximumWidth(_range_reset_button->minimumSizeHint().width());
    connect(_range_lo_spinbox, SIGNAL(valueChanged(double)), this, SLOT(range_bounds_changed()));
    _range_hi_spinbox = new FloatSpinBox;
    _range_hi_spinbox->setRange(-std::numeric_limits<float>::max(), +std::numeric_limits<float>::max());
    _range_hi_spinbox->setMaximumWidth(_range_reset_button->minimumSizeHint().width());
    connect(_range_hi_spinbox, SIGNAL(valueChanged(double)), this, SLOT(range_bounds_changed()));
    _range_selector = new RangeSelector(this);
    _range_log_x_box = new QCheckBox("Logarithmic horizontal scale");
    connect(_range_log_x_box, SIGNAL(stateChanged(int)), _range_selector, SLOT(update()));
    _range_log_y_box = new QCheckBox("Logarithmic vertical scale");
    connect(_range_log_y_box, SIGNAL(stateChanged(int)), _range_selector, SLOT(update()));
    QGridLayout* range_selector_layout = new QGridLayout;
    range_selector_layout->addWidget(_range_lo_spinbox, 0, 0);
    range_selector_layout->addWidget(_range_reset_button, 0, 1);
    range_selector_layout->addWidget(_range_hi_spinbox, 0, 2);
    range_selector_layout->addWidget(_range_selector, 1, 0, 1, 3);
    range_selector_layout->addWidget(_range_log_x_box, 2, 0, 1, 3);
    range_selector_layout->addWidget(_range_log_y_box, 3, 0, 1, 3);
    range_selector_layout->setRowStretch(4, 1);
    range_selector->setLayout(range_selector_layout);

    QWidget* range_adaptor = new QWidget;
    _range_gamma_groupbox = new QGroupBox("Gamma");
    _range_gamma_groupbox->setCheckable(true);
    connect(_range_gamma_groupbox, SIGNAL(toggled(bool)), this, SLOT(range_gamma_changed()));
    _range_gamma_spinbox = new QDoubleSpinBox;
    _range_gamma_spinbox->setRange(0.01, 9.99);
    _range_gamma_spinbox->setSingleStep(0.01);
    connect(_range_gamma_spinbox, SIGNAL(valueChanged(double)), this, SLOT(range_gamma_value_changed(double)));
    _range_gamma_slider = new QSlider(Qt::Horizontal);
    _range_gamma_slider->setRange(0, 1799);
    connect(_range_gamma_slider, SIGNAL(valueChanged(int)), this, SLOT(range_gamma_slider_changed(int)));
    QGridLayout* range_gamma_layout = new QGridLayout;
    range_gamma_layout->addWidget(_range_gamma_spinbox, 0, 0);
    range_gamma_layout->addWidget(_range_gamma_slider, 0, 1);
    _range_gamma_groupbox->setLayout(range_gamma_layout);
    _range_urq_groupbox = new QGroupBox("Uniform Rational Quantization");
    _range_urq_groupbox->setCheckable(true);
    connect(_range_urq_groupbox, SIGNAL(toggled(bool)), this, SLOT(range_urq_changed()));
    _range_urq_spinbox = new QDoubleSpinBox;
    _range_urq_spinbox->setRange(1.0, 1000.0);
    _range_urq_spinbox->setSingleStep(1.0);
    connect(_range_urq_spinbox, SIGNAL(valueChanged(double)), this, SLOT(range_urq_value_changed(double)));
    _range_urq_slider = new QSlider(Qt::Horizontal);
    _range_urq_slider->setRange(1, 1000);
    connect(_range_urq_slider, SIGNAL(valueChanged(int)), this, SLOT(range_urq_slider_changed(int)));
    QGridLayout* range_urq_layout = new QGridLayout;
    range_urq_layout->addWidget(_range_urq_spinbox, 0, 0);
    range_urq_layout->addWidget(_range_urq_slider, 0, 1);
    _range_urq_groupbox->setLayout(range_urq_layout);
    QGridLayout* range_adaptor_layout = new QGridLayout;
    range_adaptor_layout->addWidget(_range_gamma_groupbox, 0, 0);
    range_adaptor_layout->addWidget(_range_urq_groupbox, 1, 0);
    range_adaptor->setLayout(range_adaptor_layout);

    QWidget* coloring_selector = new QWidget;
    _coloring_none_btn = new QRadioButton("No coloring");
    connect(_coloring_none_btn, SIGNAL(toggled(bool)), this, SLOT(coloring_changed()));
    _coloring_jet_btn = new QRadioButton("Jet color palette");
    connect(_coloring_jet_btn, SIGNAL(toggled(bool)), this, SLOT(coloring_changed()));
    _coloring_cycjet_btn = new QRadioButton("Cyclic jet color palette");
    connect(_coloring_cycjet_btn, SIGNAL(toggled(bool)), this, SLOT(coloring_changed()));
    _coloring_custom_btn = new QRadioButton("Custom palette");
    connect(_coloring_custom_btn, SIGNAL(toggled(bool)), this, SLOT(coloring_changed()));
    _coloring_inverse_box = new QCheckBox("Inverse color direction");
    connect(_coloring_inverse_box, SIGNAL(toggled(bool)), this, SLOT(coloring_changed()));
    _coloring_start_slider = new QSlider(Qt::Horizontal);
    _coloring_start_slider->setRange(0, 1000);
    connect(_coloring_start_slider, SIGNAL(valueChanged(int)), this, SLOT(coloring_changed()));
    _coloring_lightvar_slider = new QSlider(Qt::Horizontal);
    _coloring_lightvar_slider->setRange(0, 1000);
    connect(_coloring_lightvar_slider, SIGNAL(valueChanged(int)), this, SLOT(coloring_changed()));
    QGridLayout* coloring_layout = new QGridLayout;
    coloring_layout->addWidget(_coloring_none_btn, 0, 0, 1, 3);
    coloring_layout->addWidget(_coloring_jet_btn, 1, 0, 1, 3);
    coloring_layout->addWidget(_coloring_cycjet_btn, 2, 0, 1, 3);
    coloring_layout->addWidget(_coloring_custom_btn, 3, 0, 1, 3);
    coloring_layout->addWidget(_coloring_inverse_box, 4, 0, 1, 3);
    coloring_layout->addWidget(new QLabel("Start color:"), 5, 0, 1, 1);
    coloring_layout->addWidget(_coloring_start_slider, 5, 1, 1, 2);
    coloring_layout->addWidget(new QLabel("Brightness adaption:"), 6, 0, 1, 1);
    coloring_layout->addWidget(_coloring_lightvar_slider, 6, 1, 1, 2);
    coloring_selector->setLayout(coloring_layout);

    /*
    Mode2DColoringSelector* coloring_selector = new Mode2DColoringSelector;
    Mode2DIsolineSelector* isoline_selector = new Mode2DIsolineSelector;
    Mode2DSurfaceSelector* surface_selector = new Mode2DSurfaceSelector;
    Mode2DInfoView* info_view = new Mode2DInfoView;
    addItem(coloring_selector, "Coloring");
    addItem(isoline_selector, "Isolines");
    addItem(surface_selector, "Surface");
    addItem(info_view, "Element Info");
    connect(coloring_selector, SIGNAL(changed()), this, SLOT(send_set_signal()));
    connect(this, SIGNAL(update_sub_widget()), coloring_selector, SLOT(update()));
    connect(isoline_selector, SIGNAL(changed()), this, SLOT(send_set_signal()));
    connect(this, SIGNAL(update_sub_widget()), isoline_selector, SLOT(update()));
    connect(surface_selector, SIGNAL(changed()), this, SLOT(send_set_signal()));
    connect(this, SIGNAL(update_sub_widget()), surface_selector, SLOT(update()));
    connect(this, SIGNAL(update_sub_widget()), info_view, SLOT(update()));
    */

    QToolBox* toolbox = new QToolBox;
    toolbox->addItem(range_selector, "Value Range Selection");
    toolbox->addItem(range_adaptor, "Value Range Adaption");
    toolbox->addItem(coloring_selector, "Coloring");

    QGridLayout* layout = new QGridLayout;
    layout->addWidget(component_selector, 0, 0);
    layout->addWidget(toolbox, 1, 0);
    layout->setRowStretch(2, 1);
    setLayout(layout);

    setFixedWidth(toolbox->sizeHint().width() + 24);

    update();
}

void Mode2DWidget::component_changed(int i)
{
    if (_lock)
        return;
    _view_params.mode_2d_global.component = i;
    update();
    emit set_view_params(_view_params);
}

void Mode2DWidget::range_bounds_changed()
{
    if (_lock)
        return;
    unsigned int c = _view_params.mode_2d_global.component;
    _view_params.mode_2d_components[c].minval = _range_lo_spinbox->value();
    _view_params.mode_2d_components[c].maxval = _range_hi_spinbox->value();
    _range_selector->update_bounds();
}

void Mode2DWidget::range_bounds_reset()
{
    if (_lock)
        return;
    unsigned int c = _view_params.mode_2d_global.component;
    _view_params.mode_2d_components[c].minval = _view_params.mode_2d_components[c].default_minval;
    _view_params.mode_2d_components[c].maxval = _view_params.mode_2d_components[c].default_maxval;
    _range_lo_spinbox->setValue(_view_params.mode_2d_components[c].minval);
    _range_hi_spinbox->setValue(_view_params.mode_2d_components[c].maxval);
    _range_selector->update_bounds();
}

void Mode2DWidget::range_changed()
{
    if (_lock)
        return;
    _range_selector->update();
    emit set_view_params(_view_params);
}

void Mode2DWidget::range_gamma_changed()
{
    if (_lock)
        return;
    unsigned int c = _view_params.mode_2d_global.component;
    _view_params.mode_2d_components[c].gamma = _range_gamma_groupbox->isChecked();
    emit set_view_params(_view_params);
}

void Mode2DWidget::range_gamma_value_changed(double g)
{
    if (_lock)
        return;
    unsigned int c = _view_params.mode_2d_global.component;
    _view_params.mode_2d_components[c].gamma_value = g;
    _lock = true;
    _range_gamma_slider->setValue(range_gamma_to_slider(g));
    _lock = false;
    emit set_view_params(_view_params);
}

void Mode2DWidget::range_gamma_slider_changed(int g)
{
    if (_lock)
        return;
    _range_gamma_spinbox->setValue(g >= 900 ? (g - 800) / 100.0 : g / 899.0);
}

int Mode2DWidget::range_gamma_to_slider(double g)
{
    return (g < 1.0 ? glvm::round(g * 899.0) : 800.0 + glvm::round(g * 100.0));
}

void Mode2DWidget::range_urq_changed()
{
    if (_lock)
        return;
    unsigned int c = _view_params.mode_2d_global.component;
    _view_params.mode_2d_components[c].urq = _range_urq_groupbox->isChecked();
    emit set_view_params(_view_params);
}

void Mode2DWidget::range_urq_value_changed(double g)
{
    if (_lock)
        return;
    unsigned int c = _view_params.mode_2d_global.component;
    _view_params.mode_2d_components[c].urq_value = g;
    _lock = true;
    _range_urq_slider->setValue(g);
    _lock = false;
    emit set_view_params(_view_params);
}

void Mode2DWidget::range_urq_slider_changed(int g)
{
    if (_lock)
        return;
    _range_urq_spinbox->setValue(g);
}

void Mode2DWidget::coloring_changed()
{
    if (_lock)
        return;
    unsigned int c = _view_params.mode_2d_global.component;
    if (_coloring_none_btn->isChecked()) {
        _view_params.mode_2d_components[c].jetcolor = false;
        _view_params.mode_2d_components[c].jetcolor_cyclic = false;
        _view_params.mode_2d_components[c].gradient = false;
    } else if (_coloring_jet_btn->isChecked()) {
        _view_params.mode_2d_components[c].jetcolor = true;
        _view_params.mode_2d_components[c].jetcolor_cyclic = false;
        _view_params.mode_2d_components[c].gradient = false;
    } else if (_coloring_cycjet_btn->isChecked()) {
        _view_params.mode_2d_components[c].jetcolor = true;
        _view_params.mode_2d_components[c].jetcolor_cyclic = true;
        _view_params.mode_2d_components[c].gradient = false;
    } else {
        _view_params.mode_2d_components[c].jetcolor = false;
        _view_params.mode_2d_components[c].jetcolor_cyclic = false;
        _view_params.mode_2d_components[c].gradient = true;
    }
    _view_params.mode_2d_components[c].coloring_inverse = _coloring_inverse_box->isChecked();
    _view_params.mode_2d_components[c].coloring_start = _coloring_start_slider->value() / 1000.0f;
    _view_params.mode_2d_components[c].coloring_lightvar = _coloring_lightvar_slider->value() / 1000.0f;
    emit set_view_params(_view_params);
}

void Mode2DWidget::update()
{
    _lock = true;
    unsigned int c = _view_params.mode_2d_global.component;

    _component_selector_box->clear();
    for (unsigned int i = 0; i < _hdr.components(); i++) {
        QString name = QString::number(i);
        const char* tagval = _hdr.component_taglist(i).get("INTERPRETATION");
        if (tagval)
            name += QString(": ") + QString(tagval);
        _component_selector_box->addItem(name);
    }
    if (_view_params.mode_2d_global.colorspace != ViewParameters::colorspace_null) {
        QString name = QString::number(_view_params.mode_2d_global.color_components[0])
            + QString("+") + QString::number(_view_params.mode_2d_global.color_components[1])
            + QString("+") + QString::number(_view_params.mode_2d_global.color_components[2])
            + QString(": Color");
        _component_selector_box->addItem(name);
    }
    _component_selector_box->setCurrentIndex(c);

    _range_lo_spinbox->setValue(_view_params.mode_2d_components[c].minval);
    _range_hi_spinbox->setValue(_view_params.mode_2d_components[c].maxval);
    _range_selector->update_bounds();
    _range_selector->update();

    _range_gamma_groupbox->setChecked(_view_params.mode_2d_components[c].gamma);
    _range_gamma_spinbox->setValue(_view_params.mode_2d_components[c].gamma_value);
    _range_gamma_slider->setValue(range_gamma_to_slider(_range_gamma_spinbox->value()));
    _range_urq_groupbox->setChecked(_view_params.mode_2d_components[c].urq);
    _range_urq_spinbox->setValue(_view_params.mode_2d_components[c].urq_value);
    _range_urq_slider->setValue(_range_urq_spinbox->value());

    if (_view_params.mode_2d_components[c].jetcolor) {
        if (_view_params.mode_2d_components[c].jetcolor_cyclic)
            _coloring_cycjet_btn->setChecked(true);
        else
            _coloring_jet_btn->setChecked(true);
    } else if (_view_params.mode_2d_components[c].gradient) {
        _coloring_custom_btn->setChecked(true);
    } else {
        _coloring_none_btn->setChecked(true);
    }
    _coloring_inverse_box->setChecked(_view_params.mode_2d_components[c].coloring_inverse);
    _coloring_start_slider->setValue(_view_params.mode_2d_components[c].coloring_start * 1000.0f);
    _coloring_lightvar_slider->setValue(_view_params.mode_2d_components[c].coloring_lightvar * 1000.0f);

    _lock = false;
}
