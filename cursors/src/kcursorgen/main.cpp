/*
    SPDX-FileCopyrightText: 2024 Jin Liu <m.liu.jin@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcursorgen.h"
#include "options.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QString>

int main(int argc, char **argv)
{
    QCommandLineParser parser;
    QCoreApplication app(argc, argv);

    const auto description = QStringLiteral(
        "Convert SVG theme to XCursor theme.\n"
        "NOTE: This tool is in EXPERIMENTAL stage and subject to change.\n"
        "Usage:\n"
        "    kcursorgen --svg-theme-to-xcursor --svg-dir <path> --xcursor-dir <path> --sizes <size1,size2,...> --scales <scale1,scale2,...>\n");
    const auto version = QStringLiteral("1.0");

    app.setApplicationVersion(version);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.setApplicationDescription(description);
    parser.addOptions({Options::svgThemeToXCursor(), Options::svgDir(), Options::xcursorDir(), Options::sizes(), Options::scales()});
    parser.process(app);

    // at least one operation should be specified
    if (argc <= 1) {
        parser.showHelp(0);
    } else if (parser.isSet(Options::svgThemeToXCursor())) {
        if (!parser.isSet(Options::svgDir())) {
            qCritical() << "Missing <svg-dir> parameter";
            return 1;
        }

        if (!parser.isSet(Options::xcursorDir())) {
            qCritical() << "Missing <xcursor-dir> parameter";
            return 1;
        }

        const QStringList sizeStrings = parser.value(Options::sizes()).split(QLatin1Char(','), Qt::SkipEmptyParts);

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
            qCritical() << "No valid <sizes> specified";
            return 1;
        }

        const QStringList scaleStrings = parser.value(Options::scales()).split(QLatin1Char(','), Qt::SkipEmptyParts);

        QList<qreal> scales;
        for (const QString &i : scaleStrings) {
            const qreal scale = i.toDouble();
            if (scale <= 0) {
                qCritical() << "Invalid scale: " << i;
                return 1;
            }
            scales << scale;
        }
        if (scales.isEmpty()) {
            qCritical() << "No valid <scales> specified";
            return 1;
        }

        return KCursorGen::svgThemeToXCursor(parser.value(Options::svgDir()), parser.value(Options::xcursorDir()), sizes, scales) ? 0 : 1;

    } else {
        qCritical() << "No command specified";
        return 1;
    }
}
