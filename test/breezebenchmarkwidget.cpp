//////////////////////////////////////////////////////////////////////////////
// breezebenchmarkwidget.cpp
// breeze buttons demo widget
// -------------------
//
// SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezebenchmarkwidget.h"

#include <QAbstractItemView>
#include <QIcon>

namespace Breeze
{
//_______________________________________________
BenchmarkWidget::BenchmarkWidget(QWidget *parent)
    : DemoWidget(parent)
{
    // setup ui
    ui.setupUi(this);
    ui.runButton->setIcon(QIcon::fromTheme(QStringLiteral("system-run")));
    ui.grabMouseCheckBox->setChecked(Simulator::grabMouse());
    connect(ui.grabMouseCheckBox, SIGNAL(toggled(bool)), SLOT(updateGrabMouse(bool)));
    connect(ui.runButton, SIGNAL(clicked()), SLOT(run()));
}

//_______________________________________________
void BenchmarkWidget::init(KPageDialog *dialog, QVector<KPageWidgetItem *> items)
{
    _pageDialog = dialog;

    for (auto &&item : items) {
        // get header and widget
        auto header = item->header();
        auto demoWidget(qobject_cast<DemoWidget *>(item->widget()));
        if (!demoWidget)
            continue;

        // do not add oneself to the list
        if (qobject_cast<BenchmarkWidget *>(demoWidget))
            continue;

        // add checkbox
        QCheckBox *checkbox(new QCheckBox(this));
        checkbox->setText(header);

        const bool hasBenchmark(demoWidget->metaObject()->indexOfSlot("benchmark()") >= 0);
        checkbox->setEnabled(hasBenchmark);
        checkbox->setChecked(hasBenchmark);

        if (hasBenchmark) {
            connect(this, SIGNAL(runBenchmark()), demoWidget, SLOT(benchmark()));
        }

        ui.verticalLayout->addWidget(checkbox);

        _items.append(ItemPair(checkbox, item));

        connect(checkbox, SIGNAL(toggled(bool)), SLOT(updateButtonState()));
    }
}

//_______________________________________________
void BenchmarkWidget::updateButtonState(void)
{
    bool enabled(false);
    for (auto &&item : _items) {
        if (item.first->isEnabled() && item.first->isChecked()) {
            enabled = true;
            break;
        }
    }

    ui.runButton->setEnabled(enabled);
}

//_______________________________________________
void BenchmarkWidget::run(void)
{
    // disable button and groupbox
    ui.runButton->setEnabled(false);
    Simulator::setGrabMouse(ui.grabMouseCheckBox->isChecked());
    for (int index = 0; index < _items.size(); ++index) {
        auto item(_items[index]);

        // check state
        if (!(item.first->isEnabled() && item.first->isChecked())) {
            continue;
        }

        if (simulator().aborted())
            return;
        else {
            selectPage(index);
            emit runBenchmark();
        }
    }

    // re-select last page
    selectPage(_items.size());

    // disable button and groupbox
    ui.runButton->setEnabled(true);
}

//_______________________________________________
void BenchmarkWidget::selectPage(int index) const
{
    // check dialog
    if (!_pageDialog)
        return;

    // try find item view from pageView
    auto view(_pageDialog.data()->findChild<QAbstractItemView *>());

    // select in list
    if (view) {
        simulator().selectItem(view, index);
        simulator().run();
    }

    return;
}
}
