/*
    SPDX-FileCopyrightText: 2024 Jin Liu <m.liu.jin@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcursorgen.h"
#include "options.h"

#include <QCollator>
#include <QDir>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QPainter>
#include <QProcess>
#include <QRegularExpression>
#include <QString>
#include <QSvgRenderer>
#include <QTemporaryDir>
#include <QTimer>
#include <QTransform>

#include <chrono>
#include <optional>

const QString XCURSOR_DIR = QStringLiteral("cursors");
const QString SVG_CURSOR_DIR = QStringLiteral("cursors_scalable");

struct SvgCursorMetaDataEntry {
    static std::optional<SvgCursorMetaDataEntry> parse(const QJsonObject &object);

    QString fileName;
    qreal nominalSize;
    QPointF hotspot;
    std::chrono::milliseconds delay;
};

std::optional<SvgCursorMetaDataEntry> SvgCursorMetaDataEntry::parse(const QJsonObject &object)
{
    const QJsonValue fileName = object.value(QLatin1String("filename"));
    if (!fileName.isString()) {
        return std::nullopt;
    }

    const QJsonValue nominalSize = object.value(QLatin1String("nominal_size"));
    if (!nominalSize.isDouble()) {
        return std::nullopt;
    }

    const QJsonValue hotspotX = object.value(QLatin1String("hotspot_x"));
    if (!hotspotX.isDouble()) {
        return std::nullopt;
    }

    const QJsonValue hotspotY = object.value(QLatin1String("hotspot_y"));
    if (!hotspotY.isDouble()) {
        return std::nullopt;
    }

    const QJsonValue delay = object.value(QLatin1String("delay"));

    return SvgCursorMetaDataEntry{
        .fileName = fileName.toString(),
        .nominalSize = nominalSize.toDouble(),
        .hotspot = QPointF(hotspotX.toDouble(), hotspotY.toDouble()),
        .delay = std::chrono::milliseconds(delay.toInt()),
    };
}

struct SvgCursorMetaData {
    static std::optional<SvgCursorMetaData> parse(const QString &filePath);

    QList<SvgCursorMetaDataEntry> entries;
};

std::optional<SvgCursorMetaData> SvgCursorMetaData::parse(const QString &filePath)
{
    QFile metaDataFile(filePath);
    if (!metaDataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return std::nullopt;
    }

    QJsonParseError jsonParseError;
    const QJsonDocument document = QJsonDocument::fromJson(metaDataFile.readAll(), &jsonParseError);
    if (jsonParseError.error) {
        return std::nullopt;
    }

    QList<SvgCursorMetaDataEntry> entries;
    if (document.isArray()) {
        const QJsonArray array = document.array();
        for (int i = 0; i < array.size(); ++i) {
            const QJsonValue element = array.at(i);
            if (!element.isObject()) {
                return std::nullopt;
            }
            if (const auto entry = SvgCursorMetaDataEntry::parse(element.toObject())) {
                entries.append(entry.value());
            } else {
                return std::nullopt;
            }
        }
    } else {
        return std::nullopt;
    }

    return SvgCursorMetaData{
        .entries = entries,
    };
}

static bool makeOrClearDir(const QString &path)
{
    QDir dir(path);
    if (!dir.removeRecursively()) {
        qCritical() << "Failed to remove dir" << path;
        return false;
    }
    if (!dir.mkpath(QStringLiteral("."))) {
        qCritical() << "Failed to create dir" << path;
        return false;
    }
    return true;
}

bool KCursorGen::svgThemeToXCursor(const QString &themeDir, const QList<int> &sizes)
{
    // Generate PNGs and config files
    const QDir srcDir(QDir(themeDir).filePath(SVG_CURSOR_DIR));

    QTemporaryDir tmpDir;
    if (!tmpDir.isValid()) {
        qCritical() << "Failed to create temporary directory";
        return false;
    }
    const QDir renderDir(tmpDir.path());

    for (const int size : sizes) {
        if (!makeOrClearDir(renderDir.filePath(QString::number(size)))) {
            return false;
        }
    }
    if (!makeOrClearDir(renderDir.filePath(QStringLiteral("config")))) {
        return false;
    }

    const QStringList shapes = srcDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name);
    for (const QString &shape : shapes) {
        const QDir shapeDir(srcDir.filePath(shape));
        qInfo() << "Rendering" << shape;

        const QString metadataFilePath = shapeDir.filePath(QStringLiteral("metadata.json"));
        const auto metadata = SvgCursorMetaData::parse(metadataFilePath);
        if (!metadata.has_value()) {
            qCritical() << "Failed to parse" << metadataFilePath;
            return false;
        }
        const bool animated = metadata->entries.size() > 1;
        QFile configFile(renderDir.filePath(QStringLiteral("config/") + shape + QStringLiteral(".config")));
        if (!configFile.open(QIODevice::WriteOnly)) {
            qCritical() << "Failed to open" << configFile.fileName() << "for writing";
            return false;
        }
        QTextStream configStream(&configFile);

        for (int desiredSize : sizes) {
            const QDir renderDirForSize(renderDir.filePath(QString::number(desiredSize)));

            for (const SvgCursorMetaDataEntry &entry : metadata->entries) {
                const QString svgPath = shapeDir.filePath(entry.fileName);
                const qreal scale = desiredSize / entry.nominalSize;

                QSvgRenderer renderer(svgPath);
                if (!renderer.isValid()) {
                    qCritical() << "Failed to render" << svgPath;
                    return false;
                }

                const QRect bounds(QPoint(0, 0), renderer.defaultSize() * scale);
                QImage image(bounds.size(), QImage::Format_ARGB32_Premultiplied);
                image.fill(Qt::transparent);

                QPainter painter(&image);
                // to suppress "QFont::setPointSizeF: Point size <= 0 (-0.720000), must be greater than 0" warnings
                painter.setFont(QFont(QStringLiteral("Sans"), 10));

                renderer.render(&painter, bounds);

                painter.end();
                QString pngName = entry.fileName;
                pngName.replace(QStringLiteral(".svg"), QStringLiteral(".png"));
                image.save(renderDirForSize.filePath(pngName));

                const QPointF hotspot = entry.hotspot * scale;

                configStream << desiredSize << QStringLiteral(" ") << int(hotspot.x()) << QStringLiteral(" ") << int(hotspot.y()) << QStringLiteral(" ")
                             << QString::number(desiredSize) << QStringLiteral("/") << pngName;
                if (animated) {
                    configStream << QStringLiteral(" ") << entry.delay.count();
                }
                configStream << QStringLiteral("\n");
            }
        }
        configFile.close();
    }

    // Generate XCursors
    const QDir xcursorDir(QDir(themeDir).filePath(XCURSOR_DIR));
    if (!makeOrClearDir(xcursorDir.path())) {
        return false;
    }

    const QDir configDir(renderDir.filePath(QStringLiteral("config")));
    const QStringList configs = configDir.entryList(QDir::Files, QDir::Name);
    for (const QString &config : configs) {
        QString cursorName = config;
        cursorName.replace(QStringLiteral(".config"), QStringLiteral(""));
        qInfo() << "Generating" << cursorName;
        QProcess xcursorgen;
        xcursorgen.setProgram(QStringLiteral("xcursorgen"));
        xcursorgen.setArguments({QStringLiteral("--prefix"), renderDir.path(), configDir.filePath(config), xcursorDir.filePath(cursorName)});
        xcursorgen.start();
        xcursorgen.waitForFinished(-1);
        if (xcursorgen.exitStatus() != QProcess::NormalExit || xcursorgen.exitCode() != 0) {
            qCritical() << "xcursorgen failed:" << xcursorgen.errorString();
            return false;
        }
    }

    // Aliases
    qInfo() << "Making aliases";
    const auto dirs = srcDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &info : dirs) {
        if (!info.isSymbolicLink()) {
            continue;
        }
        const QString aliasName = info.fileName();
        const QString targetName = info.readSymLink();
        if (!QFile::link(targetName, xcursorDir.filePath(aliasName))) {
            qCritical() << "Failed to create alias" << aliasName << "=>" << targetName;
            return false;
        }
    }

    qInfo() << "SUCCESS";
    return true;
}
