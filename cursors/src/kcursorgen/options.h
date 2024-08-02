/*
    SPDX-FileCopyrightText: 2024 Jin Liu <m.liu.jin@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QCommandLineOption>

namespace Options
{

// main commands

static QCommandLineOption svgThemeToXCursor()
{
    static QCommandLineOption o{QStringLiteral("svg-theme-to-xcursor"),
                                QStringLiteral("Convert a SVG cursor theme in <theme-dir>/cursors_scalable to XCursor format in <theme-dir>/cursors")};
    return o;
}

// parameters
static QCommandLineOption themeDir()
{
    static QCommandLineOption o{QStringLiteral("theme-dir"), QStringLiteral("XDG cursor theme directory."), QStringLiteral("theme-dir")};
    return o;
}

static QCommandLineOption xcursorSizes()
{
    static QCommandLineOption o{QStringLiteral("xcursor-sizes"),
                                QStringLiteral("Comma-separated list of cursor sizes to generate."),
                                QStringLiteral("xcursor-sizes")};
    return o;
}

}
