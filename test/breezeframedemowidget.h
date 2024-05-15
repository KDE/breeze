#ifndef breezeframedemowidget_h
#define breezeframedemowidget_h

//////////////////////////////////////////////////////////////////////////////
// breezeframedemowidget.h
// breeze frames demo widget
// -------------------
//
// SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include <QBoxLayout>
#include <QFrame>
#include <QWidget>

#include "breezedemowidget.h"
#include "ui_breezeframedemowidget.h"

class KMessageWidget;

namespace Breeze
{
class FrameDemoWidget : public DemoWidget
{
    Q_OBJECT

public:
    //* constructor
    explicit FrameDemoWidget(QWidget * = nullptr);

    void addMessages();

    bool eventFilter(QObject *obj, QEvent *event) override;

public Q_SLOTS:

    //* benchmarking
    void benchmark(void);

private Q_SLOTS:

    //* groupbox
    void toggleFlatGroupBox(bool value)
    {
        ui.groupBox->setFlat(value);
    }

    //* frame style
    void toggleRaisedFrame(bool value)
    {
        if (value)
            ui.frame->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    }

    void togglePlainFrame(bool value)
    {
        if (value)
            ui.frame->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    }

    void toggleSunkenFrame(bool value)
    {
        if (value)
            ui.frame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    }

    //* layout direction
    void updateLayoutDirection(int);

private:
    Ui_FrameDemoWidget ui;
    KMessageWidget *posMsg, *infoMsg, *warnMsg, *errMsg;
};
}

#endif
