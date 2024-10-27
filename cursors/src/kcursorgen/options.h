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
static QCommandLineOption svgDir()
{
    static QCommandLineOption o{QStringLiteral("svg-dir"), QStringLiteral("SVG cursor directory."), QStringLiteral("svg-dir")};
    return o;
}

static QCommandLineOption xcursorDir()
{
    static QCommandLineOption o{QStringLiteral("xcursor-dir"), QStringLiteral("XCursor directory."), QStringLiteral("xcursor-dir")};
    return o;
}

static QCommandLineOption sizes()
{
    static QCommandLineOption o{QStringLiteral("sizes"), QStringLiteral("Comma-separated list of cursor sizes to generate."), QStringLiteral("sizes")};
    return o;
}

static QCommandLineOption scales()
{
    static QCommandLineOption o{QStringLiteral("scales"),
                                QStringLiteral("Comma-separated list of scales to apply to each size."),
                                QStringLiteral("scales"),
                                QStringLiteral("1")};
    return o;
}

}
