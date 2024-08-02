/*
    SPDX-FileCopyrightText: 2024 Jin Liu <m.liu.jin@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QCommandLineOption>

namespace Options
{
// main commands
static QCommandLineOption buildSvgTheme()
{
    static QCommandLineOption o{QStringLiteral("build-svg-theme"),
                                QStringLiteral("Build a SVG cursor theme in <theme-dir>/cursors_scalable from SVG source files in <svg-src-dir>")};
    return o;
}
static QCommandLineOption renderSvgTheme()
{
    static QCommandLineOption o{QStringLiteral("render-svg-theme"),
                                QStringLiteral("Render a SVG cursor theme in <theme-dir>/cursors_scalable to PNG images in <render-dir>")};
    return o;
}
static QCommandLineOption svgThemeToXCursor()
{
    static QCommandLineOption o{QStringLiteral("svg-theme-to-xcursor"),
                                QStringLiteral("Convert a SVG cursor theme in <theme-dir>/cursors_scalable to XCursor format in <theme-dir>/cursors")};
    return o;
}

// common required parameters
static QCommandLineOption themeDir()
{
    static QCommandLineOption o{QStringLiteral("theme-dir"), QStringLiteral("XDG cursor theme directory."), QStringLiteral("theme-dir")};
    return o;
}

// required by build-svg-theme
static QCommandLineOption svgSrcDir()
{
    static QCommandLineOption o{QStringLiteral("svg-src-dir"), QStringLiteral("SVG source file directory."), QStringLiteral("svg-src-dir")};
    return o;
}

// required by render-svg-theme
static QCommandLineOption renderDir()
{
    static QCommandLineOption o{QStringLiteral("render-dir"), QStringLiteral("render-svg-theme output directory."), QStringLiteral("render-dir")};
    return o;
}

// optional parameters

// build_svg_theme
static QCommandLineOption aliasFile()
{
    static QCommandLineOption o{QStringLiteral("alias-file"), QStringLiteral("Source file containing alias definitions."), QStringLiteral("alias-file")};
    return o;
}

static QCommandLineOption frametime()
{
    static QCommandLineOption o{QStringLiteral("frametime"),
                                QStringLiteral("Frame time in milliseconds for animated cursors. (default:30)"),
                                QStringLiteral("frametime"),
                                QStringLiteral("30")};
    return o;
}

// render_svg_theme
static QCommandLineOption markHotspot()
{
    static QCommandLineOption o{QStringLiteral("mark-hotspot"), QStringLiteral("Draw lines to mark the hotspot.")};
    return o;
}

// render_svg_theme and svg_theme_to_xcursor
static QCommandLineOption sizes()
{
    static QCommandLineOption o{QStringLiteral("sizes"),
                                QStringLiteral("Comma-separated list of cursor sizes to generate. (default: 12,18,24,30,36,42,48,54,60,66,72)"),
                                QStringLiteral("sizes"),
                                QStringLiteral("12,18,24,30,36,42,48,54,60,66,72")};
    return o;
}

}
