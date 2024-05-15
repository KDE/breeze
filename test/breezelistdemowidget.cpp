//////////////////////////////////////////////////////////////////////////////
// breezelistdemowidget.cpp
// breeze lists (and trees) demo widget
// -------------------
//
// SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezelistdemowidget.h"

namespace Breeze
{
//______________________________________________________________
ListDemoWidget::ListDemoWidget(QWidget *parent)
    : DemoWidget(parent)
{
    ui.setupUi(this);
    ui.treeWidget->sortByColumn(0, Qt::AscendingOrder);
}

//______________________________________________________________
void ListDemoWidget::benchmark(void)
{
    if (!isVisible())
        return;

    if (true) {
        simulator().selectItem(ui.listWidget, 0);
        simulator().selectItem(ui.listWidget, 1);
        simulator().selectItem(ui.listWidget, 2);
    }

    if (true) {
        simulator().selectItem(ui.treeWidget, 0, 0);
        simulator().selectItem(ui.treeWidget, 1, 0);
        simulator().selectItem(ui.treeWidget, 2, 0);
    }

    if (true) {
        simulator().selectItem(ui.tableWidget, 0, 0);
        simulator().selectItem(ui.tableWidget, 0, 1);
        simulator().selectItem(ui.tableWidget, 0, 2);

        simulator().selectItem(ui.tableWidget, 1, 0);
        simulator().selectItem(ui.tableWidget, 1, 1);
        simulator().selectItem(ui.tableWidget, 1, 2);

        simulator().selectItem(ui.tableWidget, 2, 0);
        simulator().selectItem(ui.tableWidget, 2, 1);
        simulator().selectItem(ui.tableWidget, 2, 2);
    }

    if (true) {
        QSplitterHandle *handle(ui.splitter->handle(1));
        simulator().slide(handle, QPoint(0, -20));
        simulator().slide(handle, QPoint(0, 20));

        handle = ui.splitter->handle(2);
        simulator().slide(handle, QPoint(0, 20));
        simulator().slide(handle, QPoint(0, -20));
    }

    simulator().run();
}
}
