//////////////////////////////////////////////////////////////////////////////
// breezetabdemowidget.cpp
// breeze tabwidget demo widget
// -------------------
//
// SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezetabdemowidget.h"

#include <QIcon>

namespace Breeze
{
//______________________________________________________________
TabDemoWidget::TabDemoWidget(QWidget *parent)
    : DemoWidget(parent)
    , _left(new QToolButton(nullptr))
    , _right(new QToolButton(nullptr))
{
    ui.setupUi(this);
    connect(ui.tabPositionComboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeTabPosition(int)));
    connect(ui.textPositionComboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeTextPosition(int)));
    connect(ui.documentModeCheckBox, SIGNAL(toggled(bool)), SLOT(toggleDocumentMode(bool)));
    connect(ui.cornerWidgetsCheckBox, SIGNAL(toggled(bool)), SLOT(toggleCornerWidgets(bool)));
    connect(ui.tabCloseButtonsCheckBox, SIGNAL(toggled(bool)), SLOT(toggleTabCloseButtons(bool)));
    connect(ui.tabBarVisibilityCheckBox, SIGNAL(toggled(bool)), ui.tabWidget, SLOT(toggleTabBarVisibility(bool)));
    ui.textPositionComboBox->setCurrentIndex(1);

    _left->setIcon(QIcon::fromTheme(QStringLiteral("tab-new")));
    _left->setVisible(false);

    _right->setIcon(QIcon::fromTheme(QStringLiteral("tab-close")));
    _right->setText(tr("Right Corner Button"));
    _right->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    _right->setVisible(false);
}

//______________________________________________________________
void TabDemoWidget::toggleCornerWidgets(bool value)
{
    if (value) {
        ui.tabWidget->setCornerWidget(_left, Qt::TopLeftCorner);
        ui.tabWidget->setCornerWidget(_right, Qt::TopRightCorner);
    } else {
        ui.tabWidget->setCornerWidget(nullptr, Qt::TopLeftCorner);
        ui.tabWidget->setCornerWidget(nullptr, Qt::TopRightCorner);
    }

    _left->setVisible(value);
    _right->setVisible(value);
    ui.tabWidget->adjustSize();
}

//______________________________________________________________
void TabDemoWidget::toggleDocumentMode(bool value)
{
    ui.tabWidget->setDocumentMode(value);
}

//______________________________________________________________
void TabDemoWidget::toggleTabCloseButtons(bool value)
{
    ui.tabWidget->setTabsClosable(value);
}

//______________________________________________________________
void TabDemoWidget::changeTabPosition(int index)
{
    switch (index) {
    case 1:
        ui.tabWidget->setTabPosition(QTabWidget::South);
        break;

    case 2:
        ui.tabWidget->setTabPosition(QTabWidget::West);
        break;

    case 3:
        ui.tabWidget->setTabPosition(QTabWidget::East);
        break;

    default:
    case 0:
        ui.tabWidget->setTabPosition(QTabWidget::North);
        break;
    }
}

//______________________________________________________________
void TabDemoWidget::changeTextPosition(int index)
{
    switch (index) {
    case 0:
        ui.tabWidget->hideText();
        ui.tabWidget->showIcons();
        break;

    case 1:
        ui.tabWidget->showText();
        ui.tabWidget->hideIcons();
        break;

    default:
    case 2:
        ui.tabWidget->showText();
        ui.tabWidget->showIcons();
        break;
    }
}

//______________________________________________________________
void TabDemoWidget::benchmark(void)
{
    if (!isVisible())
        return;

    if (true) {
        simulator().selectComboBoxItem(ui.tabPositionComboBox, 1);
        simulator().selectComboBoxItem(ui.tabPositionComboBox, 2);
        simulator().selectComboBoxItem(ui.tabPositionComboBox, 3);
        simulator().selectComboBoxItem(ui.tabPositionComboBox, 0);

        simulator().selectComboBoxItem(ui.textPositionComboBox, 0);
        simulator().selectComboBoxItem(ui.textPositionComboBox, 2);
        simulator().selectComboBoxItem(ui.textPositionComboBox, 1);
    }

    if (true) {
        simulator().click(ui.documentModeCheckBox);
        simulator().click(ui.documentModeCheckBox);

        simulator().click(ui.cornerWidgetsCheckBox);
        simulator().click(ui.cornerWidgetsCheckBox);

        simulator().click(ui.tabCloseButtonsCheckBox);
        simulator().click(ui.tabCloseButtonsCheckBox);

        simulator().click(ui.tabBarVisibilityCheckBox);
        simulator().click(ui.tabBarVisibilityCheckBox);
    }

    if (true) {
        simulator().selectTab(ui.tabWidget, 1);
        simulator().selectTab(ui.tabWidget, 2);
        simulator().selectTab(ui.tabWidget, 3);
        simulator().selectTab(ui.tabWidget, 0);
    }

    // run
    simulator().run();
}
}
