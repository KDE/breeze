//////////////////////////////////////////////////////////////////////////////
// breezeframedemowidget.cpp
// breeze frames demo widget
// -------------------
//
// SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezeframedemowidget.h"

#include <QButtonGroup>

#include <KComboBox>
#include <KMessageWidget>
#include <kwidgetsaddons_version.h>

namespace Breeze
{
//_____________________________________________________________
FrameDemoWidget::FrameDemoWidget(QWidget *parent)
    : DemoWidget(parent)
    , posMsg(nullptr)
    , infoMsg(nullptr)
    , warnMsg(nullptr)
    , errMsg(nullptr)
{
    ui.setupUi(this);
    QButtonGroup *group = new QButtonGroup(this);
    group->addButton(ui.raisedFrameRadioButton);
    group->addButton(ui.plainFrameRadioButton);
    group->addButton(ui.sunkenFrameRadioButton);

    connect(ui.raisedFrameRadioButton, SIGNAL(toggled(bool)), SLOT(toggleRaisedFrame(bool)));
    connect(ui.plainFrameRadioButton, SIGNAL(toggled(bool)), SLOT(togglePlainFrame(bool)));
    connect(ui.sunkenFrameRadioButton, SIGNAL(toggled(bool)), SLOT(toggleSunkenFrame(bool)));

    connect(ui.directionComboBox, SIGNAL(currentIndexChanged(int)), SLOT(updateLayoutDirection(int)));
    connect(ui.flatGroupBoxCheckBox, SIGNAL(toggled(bool)), SLOT(toggleFlatGroupBox(bool)));

    addMessages();
}

void FrameDemoWidget::addMessages()
{
    delete posMsg;
    delete infoMsg;
    delete warnMsg;
    delete errMsg;

    posMsg = new KMessageWidget(QStringLiteral("A positive message"), ui.msgFrame);
    posMsg->setMessageType(KMessageWidget::Positive);
    posMsg->setWordWrap(true);
    posMsg->setIcon(QIcon::fromTheme(QStringLiteral("dialog-positive")));
    ui.verticalLayout_5->addWidget(posMsg);

    infoMsg = new KMessageWidget(QStringLiteral("An information message"), ui.msgFrame);
    infoMsg->setMessageType(KMessageWidget::Information);
    infoMsg->setWordWrap(true);
    infoMsg->setIcon(QIcon::fromTheme(QStringLiteral("dialog-information")));
    ui.verticalLayout_5->addWidget(infoMsg);

    warnMsg = new KMessageWidget(QStringLiteral("A warning message"), ui.msgFrame);
    warnMsg->setMessageType(KMessageWidget::Warning);
    warnMsg->setWordWrap(true);
    warnMsg->setIcon(QIcon::fromTheme(QStringLiteral("dialog-warning")));
    ui.verticalLayout_5->addWidget(warnMsg);

    errMsg = new KMessageWidget(QStringLiteral("An error message"), ui.msgFrame);
    errMsg->setMessageType(KMessageWidget::Error);
    errMsg->setWordWrap(true);
    errMsg->setIcon(QIcon::fromTheme(QStringLiteral("dialog-error")));
    ui.verticalLayout_5->addWidget(errMsg);

    ui.verticalLayout_5->addStretch();
}

bool FrameDemoWidget::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    Q_UNUSED(event);
    return false;
}

//_____________________________________________________________
void FrameDemoWidget::updateLayoutDirection(int value)
{
    QBoxLayout::Direction direction;
    switch (value) {
    default:
    case 0:
        direction = QBoxLayout::LeftToRight;
        break;
    case 1:
        direction = QBoxLayout::RightToLeft;
        break;
    case 2:
        direction = QBoxLayout::TopToBottom;
        break;
    case 3:
        direction = QBoxLayout::BottomToTop;
        break;
    }

    if (direction != ui.frameLayout->direction()) {
        ui.frameLayout->setDirection(direction);
        ui.frameLayout->update();
    }
}

//_____________________________________________________________
void FrameDemoWidget::benchmark(void)
{
    if (!isVisible())
        return;

    if (true) {
        simulator().selectComboBoxItem(ui.directionComboBox, 1);
        simulator().selectComboBoxItem(ui.directionComboBox, 2);
        simulator().selectComboBoxItem(ui.directionComboBox, 3);
        simulator().selectComboBoxItem(ui.directionComboBox, 0);
    }

    if (true) {
        simulator().click(ui.flatGroupBoxCheckBox);
        simulator().click(ui.flatGroupBoxCheckBox);
    }

    if (true) {
        simulator().click(ui.plainFrameRadioButton);
        simulator().click(ui.sunkenFrameRadioButton);
        simulator().click(ui.raisedFrameRadioButton);
    }

    simulator().run();
}
}
