#ifndef breezedemodialog_h
#define breezedemodialog_h

//////////////////////////////////////////////////////////////////////////////
// breezedemodialog.h
// breeze demo dialog
// -------------------
//
// SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezedemowidget.h"

#include <KPageDialog>

#include <QCheckBox>
#include <QList>
#include <QWidget>

namespace Breeze
{
class ButtonDemoWidget;
class FrameDemoWidget;
class InputDemoWidget;
class ListDemoWidget;
class MdiDemoWidget;
class SliderDemoWidget;
class TabDemoWidget;
class DemoDialog : public KPageDialog
{
    Q_OBJECT

public:
    //* constructor
    explicit DemoDialog(QWidget *parent = nullptr);

Q_SIGNALS:

    //* emitted when dialog is closed
    void abortSimulations(void);

protected:
    //* close event
    void closeEvent(QCloseEvent *) override;

    //* hide event
    void hideEvent(QHideEvent *) override;

private Q_SLOTS:

    //* update window title when page is changed
    void updateWindowTitle(KPageWidgetItem *);

    //* update page enability
    void updateEnableState(KPageWidgetItem *);

    //* toggle enable state
    void toggleEnable(bool);

    //* toggle RightToLeft
    void toggleRightToLeft(bool);

private:
    //* enable state checkbox
    QCheckBox *_enableCheckBox = nullptr;

    //* reverse layout checkbox
    QCheckBox *_rightToLeftCheckBox = nullptr;
};
}

#endif
