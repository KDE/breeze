/*
    SPDX-FileCopyrightText: 2024 Jin Liu <m.liu.jin@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcursorgen.h"
#include "options.h"

#include <QCommandLineParser>
#include <QString>

int main(int argc, char **argv)
{
    QCommandLineParser parser;
    KCursorGen app(argc, argv, &parser);

    const auto description = QStringLiteral("Generate SVG or XCursor cursor theme from SVG source files");
    const auto version = QStringLiteral("1.0");

    app.setApplicationVersion(version);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.setApplicationDescription(description);
    parser.addOptions({Options::buildSvgTheme(),
                       Options::renderSvgTheme(),
                       Options::svgThemeToXCursor(),
                       Options::themeDir(),
                       Options::svgSrcDir(),
                       Options::renderDir(),
                       Options::aliasFile(),
                       Options::frametime(),
                       Options::markHotspot(),
                       Options::sizes()});
    parser.process(app);

    // at least one operation should be specified
    if (argc <= 1) {
        parser.showHelp(0);
    }

    return app.exec();
}
