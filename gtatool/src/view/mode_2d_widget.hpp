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

#ifndef MODE_2D_WIDGET_H
#define MODE_2D_WIDGET_H

#include "config.h"

#include <QWidget>
#include <QDoubleSpinBox>
#include <QLabel>

#include "minmaxhist.hpp"
#include "viewparams.hpp"


class QComboBox;
class QPushButton;
class QCheckBox;
class QGroupBox;
class QDoubleSpinBox;
class QSlider;
class QRadioButton;

class Mode2DWidget;


// Helper classes

class FloatSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:
    QValidator::State validate(QString& input, int& pos) const;
    QString textFromValue(double value) const;
    double valueFromText(const QString& text) const;
};

class RangeSelector : public QLabel
{
    Q_OBJECT

private:
    Mode2DWidget* _mode_2d_widget;
    float _tolerance_normalized;
    bool _allow_change_left;
    bool _allow_change_right;
    bool _allow_drag;
    bool _change_left;
    bool _change_right;
    bool _drag;
    int _startpoint;
    float _range_left_normalized;
    float _range_right_normalized;
    float logtransf(float x);
    float invlogtransf(float x);
    float normalized_x_to_rangeval(float x);
    float rangeval_to_normalized_x(float y);

public:
    RangeSelector(Mode2DWidget* mode_2d_widget);
    QSize sizeHint() const;
    void update_bounds();

protected:
    void paintEvent(QPaintEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void wheelEvent(QWheelEvent*);
};

// The main class

class Mode2DWidget : public QWidget
{
Q_OBJECT

private:
    const gta::header& _hdr;
    const void* _data;
    const MinMaxHist& _minmaxhist;
    ViewParameters& _view_params;
    bool _lock;

    QComboBox* _component_selector_box;
    FloatSpinBox* _range_lo_spinbox;
    FloatSpinBox* _range_hi_spinbox;
    QPushButton* _range_reset_button;
    RangeSelector* _range_selector;
    QCheckBox* _range_log_x_box;
    QCheckBox* _range_log_y_box;
    QGroupBox* _range_gamma_groupbox;
    QDoubleSpinBox* _range_gamma_spinbox;
    QSlider* _range_gamma_slider;
    QGroupBox* _range_urq_groupbox;
    QDoubleSpinBox* _range_urq_spinbox;
    QSlider* _range_urq_slider;
    QRadioButton* _coloring_none_btn;
    QRadioButton* _coloring_jet_btn;
    QRadioButton* _coloring_cycjet_btn;
    QRadioButton* _coloring_custom_btn;
    QCheckBox* _coloring_inverse_box;
    QSlider* _coloring_start_slider;
    QSlider* _coloring_lightvar_slider;

private slots:
    void component_changed(int);
    void range_bounds_changed();
    void range_bounds_reset();
    void range_changed();
    void range_gamma_changed();
    void range_gamma_value_changed(double);
    void range_gamma_slider_changed(int);
    int range_gamma_to_slider(double);
    void range_urq_changed();
    void range_urq_value_changed(double);
    void range_urq_slider_changed(int);
    void coloring_changed();

public:
    Mode2DWidget(const gta::header& hdr, const void* data, const MinMaxHist& minmaxhist, ViewParameters& view_params);

public slots:
    void update();

signals:
    void set_view_params(const ViewParameters&);

    friend class RangeSelector;
};

#endif
