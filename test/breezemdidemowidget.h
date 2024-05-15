#ifndef breezemdidemowidget_h
#define breezemdidemowidget_h

//////////////////////////////////////////////////////////////////////////////
// breezemdidemowidget.h
// breeze mdi windows demo widget
// -------------------
//
// SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include <QToolButton>
#include <QWidget>

#include "breezedemowidget.h"
#include "ui_breezemdidemowidget.h"

namespace Breeze
{
class MdiDemoWidget : public DemoWidget
{
    Q_OBJECT

public:
    //* constructor
    explicit MdiDemoWidget(QWidget * = nullptr);

public Q_SLOTS:

    void setLayoutTiled(void);
    void setLayoutCascade(void);
    void setLayoutTabbed(void);

    void benchmark(void);

private:
    Ui_MdiDemoWidget ui;
};
}

#endif
