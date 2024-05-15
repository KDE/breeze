//////////////////////////////////////////////////////////////////////////////
// breezedemodialog.cpp
// breeze configuration dialog
// -------------------
//
// SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////
#include "breezedemodialog.h"

#include "breezebenchmarkwidget.h"
#include "breezebuttondemowidget.h"
#include "breezeframedemowidget.h"
#include "breezeinputdemowidget.h"
#include "breezelistdemowidget.h"
#include "breezemdidemowidget.h"
#include "breezesliderdemowidget.h"
#include "breezetabdemowidget.h"
#ifdef HAVE_SCHEME_CHOOSER
#include "breezeschemechooser.h"
#include "breezestylechooser.h"
#endif

#include <QDialogButtonBox>
#include <QShortcut>
#include <QTextStream>

#include <KStandardShortcut>
#include <QIcon>

namespace Breeze
{
//_______________________________________________________________
DemoDialog::DemoDialog(QWidget *parent)
    : KPageDialog(parent)
{
    setWindowTitle(i18n("Breeze Demo"));

    // install Quit shortcut
    const auto shortcuts = KStandardShortcut::quit();
    for (const QKeySequence &sequence : shortcuts) {
        connect(new QShortcut(sequence, this), SIGNAL(activated()), SLOT(close()));
    }

    // button box
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal);
    setButtonBox(buttonBox);

    // connection
    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), SLOT(close()));

    // customize button box
    _enableCheckBox = new QCheckBox(i18n("Enabled"));
    _enableCheckBox->setChecked(true);
    connect(_enableCheckBox, SIGNAL(toggled(bool)), SLOT(toggleEnable(bool)));
    buttonBox->addButton(_enableCheckBox, QDialogButtonBox::ResetRole);

    _rightToLeftCheckBox = new QCheckBox(i18n("Right to left layout"));
    connect(_rightToLeftCheckBox, SIGNAL(toggled(bool)), SLOT(toggleRightToLeft(bool)));
    buttonBox->addButton(_rightToLeftCheckBox, QDialogButtonBox::ResetRole);

#ifdef HAVE_SCHEME_CHOOSER
    auto styleChooser = new WidgetStyleChooser(this);
    styleChooser->createStyleSelectionMenu(i18n("Style"));
    buttonBox->addButton(styleChooser, QDialogButtonBox::ResetRole);

    auto colChooser = new ColorSchemeChooser(this);
    buttonBox->addButton(colChooser, QDialogButtonBox::ResetRole);
#endif

    // connections
    connect(this, SIGNAL(currentPageChanged(KPageWidgetItem *, KPageWidgetItem *)), SLOT(updateWindowTitle(KPageWidgetItem *)));
    connect(this, SIGNAL(currentPageChanged(KPageWidgetItem *, KPageWidgetItem *)), SLOT(updateEnableState(KPageWidgetItem *)));

    auto setPageIcon = [](KPageWidgetItem *page, const QString &iconName) {
        page->setIcon(QIcon::fromTheme(iconName));
    };

    KPageWidgetItem *page = nullptr;
    QVector<KPageWidgetItem *> items;

    // inputs
    {
        page = new KPageWidgetItem(new InputDemoWidget());
        page->setName(i18n("Input Widgets"));
        setPageIcon(page, QStringLiteral("edit-rename"));
        page->setHeader(i18n("Shows the appearance of text input widgets"));
        addPage(page);
        items.append(page);
    }

    // tab
    {
        page = new KPageWidgetItem(new TabDemoWidget());
        page->setName(i18n("Tab Widgets"));
        setPageIcon(page, QStringLiteral("tab-detach"));
        page->setHeader(i18n("Shows the appearance of tab widgets"));
        addPage(page);
        items.append(page);
    }

    // buttons
    {
        page = new KPageWidgetItem(new ButtonDemoWidget());
        page->setName(i18n("Buttons"));
        setPageIcon(page, QStringLiteral("go-jump-locationbar"));
        page->setHeader(i18n("Shows the appearance of buttons"));
        addPage(page);
        items.append(page);
    }

    // lists
    {
        page = new KPageWidgetItem(new ListDemoWidget());
        page->setName(i18n("Lists"));
        setPageIcon(page, QStringLiteral("view-list-tree"));
        page->setHeader(i18n("Shows the appearance of lists, trees and tables"));
        addPage(page);
        items.append(page);
    }

    // frames
    {
        page = new KPageWidgetItem(new FrameDemoWidget());
        page->setName(i18n("Frames"));
        setPageIcon(page, QStringLiteral("draw-rectangle"));
        page->setHeader(i18n("Shows the appearance of various framed widgets"));
        addPage(page);
        items.append(page);
    }

    // mdi
    {
        page = new KPageWidgetItem(new MdiDemoWidget());
        page->setName(i18n("MDI Windows"));
        setPageIcon(page, QStringLiteral("preferences-system-windows"));
        page->setHeader(i18n("Shows the appearance of MDI windows"));
        addPage(page);
        items.append(page);
    }

    // sliders
    {
        page = new KPageWidgetItem(new SliderDemoWidget());
        page->setName(i18n("Sliders"));
        setPageIcon(page, QStringLiteral("measure"));
        page->setHeader(i18n("Shows the appearance of sliders, progress bars and scrollbars"));
        addPage(page);
        items.append(page);
    }

    // benchmark
    {
        auto benchmarkWidget(new BenchmarkWidget());
        page = new KPageWidgetItem(benchmarkWidget);
        page->setName(i18n("Benchmark"));
        setPageIcon(page, QStringLiteral("system-run"));
        page->setHeader(i18n("Emulates user interaction with widgets for benchmarking"));
        benchmarkWidget->init(this, items);
        addPage(page);
        items.append(page);
    }

    // connections
    QShortcut *shortcut(new QShortcut(Qt::CTRL | Qt::Key_X, this));
    for (auto item : items) {
        if (item->widget()->metaObject()->indexOfSlot("benchmark()") >= 0) {
            connect(shortcut, SIGNAL(activated()), item->widget(), SLOT(benchmark()));
        }
        connect(this, SIGNAL(abortSimulations()), &static_cast<DemoWidget *>(item->widget())->simulator(), SLOT(abort()));
    }
}

//_______________________________________________________________
void DemoDialog::updateWindowTitle(KPageWidgetItem *item)
{
    QString title;
    QTextStream what(&title);
    if (item) {
        what << item->name();
        what << " - ";
    }

    what << i18n("Breeze Demo");
    setWindowTitle(title);
}

//_______________________________________________________________
void DemoDialog::updateEnableState(KPageWidgetItem *item)
{
    if (!(item && item->widget() && _enableCheckBox))
        return;
    item->widget()->setEnabled(_enableCheckBox->isChecked());
}

//_______________________________________________________________
void DemoDialog::toggleEnable(bool value)
{
    if (!(currentPage() && currentPage()->widget()))
        return;
    currentPage()->widget()->setEnabled(value);
}

//_______________________________________________________________
void DemoDialog::toggleRightToLeft(bool value)
{
    qApp->setLayoutDirection(value ? Qt::RightToLeft : Qt::LeftToRight);
}

//_______________________________________________________________
void DemoDialog::closeEvent(QCloseEvent *event)
{
    emit abortSimulations();
    KPageDialog::closeEvent(event);
}

//_______________________________________________________________
void DemoDialog::hideEvent(QHideEvent *event)
{
    emit abortSimulations();
    KPageDialog::hideEvent(event);
}
}
