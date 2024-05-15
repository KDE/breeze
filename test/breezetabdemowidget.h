#ifndef breezetabdemowidget_h
#define breezetabdemowidget_h

//////////////////////////////////////////////////////////////////////////////
// breezetabdemowidget.h
// breeze tabwidget demo widget
// -------------------
//
// SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include <QToolButton>
#include <QWidget>

#include "breezedemowidget.h"
#include "ui_breezetabdemowidget.h"

namespace Breeze
{
class TabDemoWidget : public DemoWidget
{
    Q_OBJECT

public:
    //* constructor
    explicit TabDemoWidget(QWidget * = nullptr);

public Q_SLOTS:

    //* benchmark
    void benchmark(void);

private Q_SLOTS:

    //* show/hide corner buttons
    void toggleCornerWidgets(bool);

    //* change document mode
    void toggleDocumentMode(bool);

    //* show tab close buttons
    void toggleTabCloseButtons(bool);

    // change tab position
    void changeTabPosition(int);

    // change tab position
    void changeTextPosition(int);

private:
    //* ui
    Ui_TabDemoWidget ui;

    //* tabbar left button
    QToolButton *_left = nullptr;

    //* tabbar right button
    QToolButton *_right = nullptr;
};
}

#endif
