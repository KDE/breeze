//////////////////////////////////////////////////////////////////////////////
// breezesliderdemowidget.cpp
// breeze sliders demo widget
// -------------------
//
// SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezesliderdemowidget.h"

#include <QMenu>
#include <QStyleOptionSlider>

namespace Breeze
{
//_____________________________________________________________
ProgressBar::ProgressBar(QObject *parent, QProgressBar *progressBar, QCheckBox *checkBox)
    : QObject(parent)
    , _progressBar(progressBar)
    , _checkBox(checkBox)
    , _value(0)
{
    connect(_checkBox, SIGNAL(toggled(bool)), SLOT(toggleBusy(bool)));
}

//_____________________________________________________________
void ProgressBar::toggleBusy(bool value)
{
    if (value) {
        _value = _progressBar->value();
        _progressBar->setMinimum(0);
        _progressBar->setMaximum(0);

    } else {
        _progressBar->setMinimum(0);
        _progressBar->setMaximum(100);
        _progressBar->setValue(_value);
    }

    _progressBar->update();
}

//_____________________________________________________________
void ProgressBar::setValue(int value)
{
    if (!_checkBox->isChecked()) {
        _progressBar->setValue(value);
    }
}

//_____________________________________________________________
SliderDemoWidget::SliderDemoWidget(QWidget *parent)
    : DemoWidget(parent)
    , _locked(false)
{
    ui.setupUi(this);

    _horizontalProgressBar = new ProgressBar(this, ui.horizontalProgressBar, ui.animateProgressBarCheckBox);
    _verticalProgressBar = new ProgressBar(this, ui.verticalProgressBar, ui.animateProgressBarCheckBox);

    connect(ui.invertProgressBarCheckBox, SIGNAL(toggled(bool)), _horizontalProgressBar, SLOT(toggleInvertedAppearance(bool)));
    connect(ui.invertProgressBarCheckBox, SIGNAL(toggled(bool)), _verticalProgressBar, SLOT(toggleInvertedAppearance(bool)));
    connect(ui.tickPositionComboBox, SIGNAL(currentIndexChanged(int)), SLOT(updateTickPosition(int)));

    connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), SLOT(updateSliders(int)));
    connect(ui.horizontalScrollBar, SIGNAL(valueChanged(int)), SLOT(updateSliders(int)));
    connect(ui.verticalSlider, SIGNAL(valueChanged(int)), SLOT(updateSliders(int)));
    connect(ui.verticalScrollBar, SIGNAL(valueChanged(int)), SLOT(updateSliders(int)));
    connect(ui.dial, SIGNAL(valueChanged(int)), SLOT(updateSliders(int)));
}

//_____________________________________________________________
void SliderDemoWidget::benchmark(void)
{
    if (!isVisible())
        return;

    // horizontal
    simulator().slide(ui.horizontalSlider, QPoint(50, 0));
    simulator().slide(ui.horizontalSlider, QPoint(-50, 0));

    simulator().slide(ui.horizontalScrollBar, QPoint(50, 0));
    simulator().slide(ui.horizontalScrollBar, QPoint(-50, 0));

    // vertical
    simulator().slide(ui.verticalScrollBar, QPoint(0, 50));
    simulator().slide(ui.verticalScrollBar, QPoint(0, -50));

    simulator().slide(ui.verticalSlider, QPoint(0, 50));
    simulator().slide(ui.verticalSlider, QPoint(0, -50));

    // dial button
    // nothing for now.

    simulator().run();
}

//_____________________________________________________________
void SliderDemoWidget::updateSliders(int value)
{
    if (_locked)
        return;

    _locked = true;
    _horizontalProgressBar->setValue(value);
    _verticalProgressBar->setValue(value);

    ui.horizontalSlider->setValue(value);
    ui.verticalSlider->setValue(value);
    ui.horizontalScrollBar->setValue(value);
    ui.verticalScrollBar->setValue(value);
    ui.dial->setValue(value);

    _locked = false;
}

//_____________________________________________________________
void SliderDemoWidget::updateTickPosition(int value)
{
    ui.horizontalSlider->setTickPosition(QSlider::TickPosition(value));
    ui.verticalSlider->setTickPosition(QSlider::TickPosition(value));
    ui.dial->setNotchesVisible(value > 0);
}
}
