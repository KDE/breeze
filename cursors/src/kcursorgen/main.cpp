/*
    SPDX-FileCopyrightText: 2024 Jin Liu <m.liu.jin@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcursorgen.h"
#include "options.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QString>

int main(int argc, char **argv)
{
    QCommandLineParser parser;
    QCoreApplication app(argc, argv);

    const auto description = QStringLiteral(
        "Convert SVG theme to XCursor theme.\n"
        "NOTE: This tool is intended for cursor theme creators. It's in EXPERIMENTAL stage and subject to change.\n"
        "Usage:\n"
        "    kcursorgen --svg-theme-to-xcursor --theme-dir <path> --xcursor-sizes <size1,size2,...>\n");
    const auto version = QStringLiteral("1.0");

    app.setApplicationVersion(version);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.setApplicationDescription(description);
    parser.addOptions({Options::svgThemeToXCursor(), Options::themeDir(), Options::xcursorSizes()});
    parser.process(app);

    // at least one operation should be specified
    if (argc <= 1) {
        parser.showHelp(0);
    } else if (parser.isSet(Options::svgThemeToXCursor())) {
        if (!parser.isSet(Options::themeDir())) {
            qCritical() << "Missing <theme-dir> parameter";
            return 1;
        }

        const QStringList sizeStrings = parser.value(Options::xcursorSizes()).split(QLatin1Char(','), Qt::SkipEmptyParts);

        QList<int> sizes;
        for (const QString &i : sizeStrings) {
            const int size = i.toInt();
            if (size <= 0) {
                qCritical() << "Invalid size: " << i;
                return 1;
            }
            sizes << size;
        }
        if (sizes.isEmpty()) {
            qCritical() << "No valid <xcursor-sizes> specified";
            return 1;
        }

        return KCursorGen::svgThemeToXCursor(parser.value(Options::themeDir()), sizes) ? 0 : 1;

    } else {
        qCritical() << "No command specified";
        return 1;
    }
}
