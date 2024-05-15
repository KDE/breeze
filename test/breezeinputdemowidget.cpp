//////////////////////////////////////////////////////////////////////////////
// breezeinputdemowidget.cpp
// breeze input widgets (e.g. text editors) demo widget
// -------------------
//
// SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezeinputdemowidget.h"

namespace Breeze
{
//________________________________________________________________
InputDemoWidget::InputDemoWidget(QWidget *parent)
    : DemoWidget(parent)
{
    ui.setupUi(this);
    ui.klineedit->setText(i18n("Example text"));
    ui.klineedit_2->setText(i18n("password"));
    ui.kcombobox->insertSeparator(2);
    ui.textedit->setPlainText(
        QStringLiteral("Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor "
                       "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud "
                       "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute "
                       "irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla "
                       "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia "
                       "deserunt mollit anim id est laborum. \n\n"
                       "Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque "
                       "laudantium, totam rem aperiam, eaque ipsa quae ab illo inventore veritatis et quasi "
                       "architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas "
                       "sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione "
                       "voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit "
                       "amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut "
                       "labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis "
                       "nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi "
                       "consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam "
                       "nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla "
                       "pariatur?"));
    ui.textedit->setLineWrapMode(QTextEdit::NoWrap);
    connect(ui.flatCheckBox, SIGNAL(toggled(bool)), SLOT(toggleFlatWidgets(bool)));
    connect(ui.buttonCheckBox, SIGNAL(toggled(bool)), SLOT(toggleButtonWidgets(bool)));
    connect(ui.wrapCheckBox, SIGNAL(toggled(bool)), SLOT(toggleWrapMode(bool)));
    ui.wrapCheckBox->setChecked(true);
}

//________________________________________________________________
void InputDemoWidget::benchmark(void)
{
    if (!isVisible())
        return;

    if (true) {
        // klineedit
        simulator().clearText(ui.klineedit);
        simulator().writeSampleText(ui.klineedit);
        simulator().clearText(ui.klineedit);

        simulator().clearText(ui.klineedit_2);
        simulator().writeSampleText(ui.klineedit_2);
        simulator().clearText(ui.klineedit_2);

        simulator().clearText(ui.kcombobox->lineEdit());
        simulator().writeSampleText(ui.kcombobox->lineEdit());
        simulator().clearText(ui.kcombobox->lineEdit());

        simulator().selectComboBoxItem(ui.kcombobox, 0);
        simulator().selectComboBoxItem(ui.kcombobox, 1);
        simulator().selectComboBoxItem(ui.kcombobox, 2);
        simulator().selectComboBoxItem(ui.kcombobox, 0);

        simulator().clearText(ui.kintspinbox);
        simulator().writeText(ui.kintspinbox, QStringLiteral("10"));
        simulator().clearText(ui.kintspinbox);
        simulator().writeText(ui.kintspinbox, QStringLiteral("0"));
    }

    if (true) {
        // toggle flat widgets
        simulator().click(ui.flatCheckBox);
        simulator().click(ui.flatCheckBox);
    }

    if (true) {
        // toggle wrap mode
        simulator().click(ui.wrapCheckBox);
        simulator().click(ui.wrapCheckBox);

        simulator().clearText(ui.textedit);
        simulator().writeSampleText(ui.textedit);
        simulator().clearText(ui.textedit);
    }

    simulator().run();
}

//________________________________________________________________
void InputDemoWidget::toggleFlatWidgets(bool value)
{
    ui.klineedit->setFrame(!value);
    ui.klineedit_2->setFrame(!value);
    ui.kcombobox->setFrame(!value);
    ui.kintspinbox->setFrame(!value);
}

void InputDemoWidget::toggleButtonWidgets(bool value)
{
    ui.toolButton->setAutoRaise(value);
    ui.toolButton_2->setAutoRaise(value);
    ui.toolButton_3->setAutoRaise(value);
    ui.toolButton_4->setAutoRaise(value);
}

//________________________________________________________________
void InputDemoWidget::toggleWrapMode(bool value)
{
    if (value)
        ui.textedit->setLineWrapMode(QTextEdit::WidgetWidth);
    else
        ui.textedit->setLineWrapMode(QTextEdit::NoWrap);
}
}
