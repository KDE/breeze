#ifndef breezesliderdemowidget_h
#define breezesliderdemowidget_h

//////////////////////////////////////////////////////////////////////////////
// breezesliderdemowidget.h
// breeze sliders demo widget
// -------------------
//
// SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include <QCheckBox>
#include <QProgressBar>
#include <QWidget>

#include "breezedemowidget.h"
#include "ui_breezesliderdemowidget.h"

namespace Breeze
{
class ProgressBar : public QObject
{
    Q_OBJECT

public:
    //* constructor
    ProgressBar(QObject *, QProgressBar *, QCheckBox *);

    //* set value
    void setValue(int);

public Q_SLOTS:

    //* toggle invertex appearance
    void toggleInvertedAppearance(bool value)
    {
        _progressBar->setInvertedAppearance(value);
    }

private Q_SLOTS:

    //* toggle bussy state
    void toggleBusy(bool);

private:
    //* progressBar
    QProgressBar *_progressBar;

    //* checkbox
    QCheckBox *_checkBox;

    //* saved value
    int _value;
};

class SliderDemoWidget : public DemoWidget
{
    Q_OBJECT

public:
    //* constructor
    explicit SliderDemoWidget(QWidget * = nullptr);

public Q_SLOTS:

    //* benchmark
    void benchmark(void);

private Q_SLOTS:

    void updateSliders(int);

    void updateTickPosition(int);

private:
    bool _locked = false;

    Ui_SliderDemoWidget ui;

    //* progressbars
    ProgressBar *_horizontalProgressBar;
    ProgressBar *_verticalProgressBar;
};
}

#endif
