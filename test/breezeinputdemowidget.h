#ifndef breezeinputdemowidget_h
#define breezeinputdemowidget_h

//////////////////////////////////////////////////////////////////////////////
// breezeinputdemowidget.h
// breeze input widgets (e.g. text editors) demo widget
// -------------------
//
// SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include <QToolButton>
#include <QWidget>

#include "breezedemowidget.h"
#include "ui_breezeinputdemowidget.h"

namespace Breeze
{
class InputDemoWidget : public DemoWidget
{
    Q_OBJECT

public:
    //* constructor
    explicit InputDemoWidget(QWidget * = nullptr);

public Q_SLOTS:

    //* run benchmark
    void benchmark(void);

private Q_SLOTS:

    //* flat widgets
    void toggleFlatWidgets(bool);

    //* flat button widgets
    void toggleButtonWidgets(bool);

    //* wrap mode
    void toggleWrapMode(bool);

private:
    Ui_InputDemoWidget ui;
};
}

#endif
