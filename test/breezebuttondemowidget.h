#ifndef breezebuttondemowidget_h
#define breezebuttondemowidget_h

//////////////////////////////////////////////////////////////////////////////
// breezebuttondemowidget.h
// breeze buttons demo widget
// -------------------
//
// SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezedemowidget.h"
#include "ui_breezebuttondemowidget.h"

#include <QList>
#include <QToolBar>
#include <QToolButton>
#include <QWidget>

namespace Breeze
{
class ButtonDemoWidget : public DemoWidget
{
    Q_OBJECT

public:
    //* constructor
    explicit ButtonDemoWidget(QWidget * = nullptr);

public Q_SLOTS:

    void benchmark(void);

private Q_SLOTS:

    //* change text position in tool buttons
    void textPosition(int);

    //* change tool button icon size
    void iconSize(int);

    //* set buttons as flat
    void toggleFlat(bool);

private:
    void installMenu(QPushButton *);
    void installMenu(QToolButton *);

    Ui_ButtonDemoWidget ui;
    QToolBar *_toolBar = nullptr;
    QList<QPushButton *> _pushButtons;
    QList<QToolButton *> _toolButtons;
};
}

#endif
