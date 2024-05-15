/*
    This file was part of KDevPlatform and now of Breeze
    SPDX-FileCopyrightText: 2016 Zhigalin Alexander <alexander@zhigalin.tk>
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "breezeschemechooser.h"

#include <QActionGroup>
#include <QMenu>
#include <QModelIndex>
#include <QStringList>

#include <KActionMenu>
#include <KColorSchemeManager>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <KColorSchemeMenu>
#endif
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

namespace Breeze
{
ColorSchemeChooser::ColorSchemeChooser(QWidget *parent)
    : QPushButton(parent)
{
    auto manager = new KColorSchemeManager(parent);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto selectionMenu = manager->createSchemeSelectionMenu(this);
#else
    auto selectionMenu = KColorSchemeMenu::createMenu(manager, this);
#endif

    setMenu(selectionMenu->menu());

    setIcon(menu()->icon());
    setText(menu()->title());
}

} // namespace Breeze
