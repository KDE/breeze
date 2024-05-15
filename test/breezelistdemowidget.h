#ifndef breezelistdemowidget_h
#define breezelistdemowidget_h

//////////////////////////////////////////////////////////////////////////////
// breezelistdemowidget.h
// breeze lists (and trees) demo widget
// -------------------
//
// SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include <QWidget>

#include "breezedemowidget.h"
#include "ui_breezelistdemowidget.h"

namespace Breeze
{
class ListDemoWidget : public DemoWidget
{
    Q_OBJECT

public:
    //* constructor
    explicit ListDemoWidget(QWidget * = nullptr);

public Q_SLOTS:

    //* benchmark
    void benchmark(void);

private:
    //* ui
    Ui_ListDemoWidget ui;
};
}

#endif
