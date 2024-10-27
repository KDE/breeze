/*
    SPDX-FileCopyrightText: 2024 Jin Liu <m.liu.jin@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QList>
#include <QString>

class KCursorGen
{
public:
    static bool svgThemeToXCursor(const QString &svgDir, const QString &xcursorDir, const QList<int> &sizes, const QList<qreal> &scales);
};
