#ifndef breezedecorationsettingsprovider_h
#define breezedecorationsettingsprovider_h
/*
 * SPDX-FileCopyrightText: 2022 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

/*
 * breezedecorationsettingsprovider.h
 * Provides access to kdecoration exceptions to kstyle
 *
 */

#include "breeze.h"
#include "breezesettings.h"

#include <KSharedConfig>
#include <QMainWindow>
#include <QObject>

namespace Breeze
{

class SettingsProvider : public QObject
{
    Q_OBJECT

public:
    //* destructor
    ~SettingsProvider();

    //* singleton
    static SettingsProvider *self();

    //* internal settings for given QMainWindow
    InternalSettingsPtr internalSettings(const QMainWindow *mw = nullptr) const;

public Q_SLOTS:

    //* reconfigure
    void reconfigure();

private:
    //* constructor
    SettingsProvider();

    //* default configuration
    InternalSettingsPtr m_defaultSettings;

    //* exceptions
    InternalSettingsList m_exceptions;

    //* config object
    KSharedConfigPtr m_config;

    //* singleton
    static SettingsProvider *s_self;
};

}

#endif
