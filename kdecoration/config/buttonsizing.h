#ifndef BUTTONSIZING_H
#define BUTTONSIZING_H

//////////////////////////////////////////////////////////////////////////////
// buttonsizing.h
// -------------------
//
// SPDX-FileCopyrightText: 2022 Paul A McAuley <kde@paulmcauley.com>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breeze.h"
#include "breezesettings.h"
#include "ui_buttonsizing.h"
#include <QDialog>

namespace Breeze
{

class ButtonSizing : public QDialog
{
    Q_OBJECT

    friend class ConfigWidget;

public:
    explicit ButtonSizing(QWidget *parent = nullptr);
    ~ButtonSizing();

    void load();
    void save(const bool reloadKwinConfig = true);
    void defaults();

private slots:
    void accept() override;
    void updateChanged();
    void fullHeightButtonWidthMarginLeftChanged();
    void fullHeightButtonWidthMarginRightChanged();
    void buttonSpacingLeftChanged();
    void buttonSpacingRightChanged();
    void fullHeightButtonSpacingLeftChanged();
    void fullHeightButtonSpacingRightChanged();
    void saveAndReloadKWinConfig()
    {
        save(true);
    }
    void setApplyButtonState(const bool on);

signals:
    void changed(bool);

private:
    void setChanged(bool value);

    Ui_ButtonSizing m_ui;

    InternalSettingsPtr m_internalSettings;

    //* changed state
    bool m_changed;

    //* defaults clicked
    bool m_defaultsPressed = false;

    bool m_loading = false;
    bool m_processingDefaults = false;
};

}

#endif // BUTTONSIZING_H
