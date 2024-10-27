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
#include <numeric>
#include <optional>

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

bool KCursorGen::svgThemeToXCursor(const QString &svgDir, const QString &xcursorDir, const QList<int> &sizes, const QList<qreal> &scales)
{
    // STEP 1: Calculate all desired sizes
    // map<nominalSize -> alignment>
    // Some toolkits (e.g. GTK3) require cursor image size to be a multiple of the scale factor.
    // So we might need to add paddings to satisfy this requirement.
    QMap<int, int> desiredSizes;

    for (qreal scale : scales) {
        int alignment = round(scale);

        // Fractional scales doesn't need alignment. If the toolkit doesn't support fractional scaling
        // for cursors, it will use the nearest integer scale. If it does, it will use the Wayland
        // Viewporter protocol, which doesn't require alignment.
        if (alignment != scale) {
            alignment = 1;
        }

        for (int size : sizes) {
            const int desiredSize = round(size * scale);
            if (!desiredSizes.contains(desiredSize)) {
                desiredSizes.insert(desiredSize, alignment);
            } else {
                desiredSizes[desiredSize] = std::lcm(desiredSizes[desiredSize], alignment);
            }
        }
    }

    qInfo() << "Desired sizes:";
    for (auto [size, alignment] : desiredSizes.asKeyValueRange()) {
        qInfo() << '\t' << size << "alignment" << alignment;
    }

    // STEP 2: Generate PNGs and config files

    const QDir srcDir(svgDir);

    QTemporaryDir tmpDir;
    if (!tmpDir.isValid()) {
        qCritical() << "Failed to create temporary directory";
        return false;
    }
    const QDir renderDir(tmpDir.path());

    for (const int size : desiredSizes.keys()) {
        if (!renderDir.mkdir(QString::number(size))) {
            qCritical() << "Failed to create dir " << QString::number(size);
            return false;
        }
    }
    if (!renderDir.mkdir(QStringLiteral("config"))) {
        qCritical() << "Failed to create dir" << QStringLiteral("config");
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

        for (auto [nominalSize, alignment] : desiredSizes.asKeyValueRange()) {
            const QDir renderDirForSize(renderDir.filePath(QString::number(nominalSize)));

            for (const SvgCursorMetaDataEntry &entry : metadata->entries) {
                const QString svgPath = shapeDir.filePath(entry.fileName);
                QSvgRenderer renderer(svgPath);
                if (!renderer.isValid()) {
                    qCritical() << "Failed to render" << svgPath;
                    return false;
                }

                const qreal scale = nominalSize / entry.nominalSize;
                QSize imageSize = renderer.defaultSize() * scale;
                QSize alignedImageSize = imageSize;
                alignedImageSize.rwidth() += (alignment - (alignedImageSize.width() % alignment)) % alignment;
                alignedImageSize.rheight() += (alignment - (alignedImageSize.height() % alignment)) % alignment;

                QImage image(alignedImageSize, QImage::Format_ARGB32_Premultiplied);
                image.fill(Qt::transparent);
                QPainter painter(&image);
                // to suppress "QFont::setPointSizeF: Point size <= 0 (-0.720000), must be greater than 0" warnings
                painter.setFont(QFont(QStringLiteral("Sans"), 10));
                renderer.render(&painter, QRect(QPoint(0, 0), imageSize));
                painter.end();

                QString pngName = entry.fileName;
                pngName.replace(QStringLiteral(".svg"), QStringLiteral(".png"));
                image.save(renderDirForSize.filePath(pngName));

                const QPointF hotspot = entry.hotspot * scale;
                configStream << nominalSize << QStringLiteral(" ") << int(hotspot.x()) << QStringLiteral(" ") << int(hotspot.y()) << QStringLiteral(" ")
                             << QString::number(nominalSize) << QStringLiteral("/") << pngName;
                if (animated) {
                    configStream << QStringLiteral(" ") << entry.delay.count();
                }
                configStream << QStringLiteral("\n");
            }
        }
        configFile.close();
    }

    // STEP 3: Generate XCursors
    QDir outputDir(xcursorDir);
    if (!outputDir.mkpath(QStringLiteral("."))) {
        qCritical() << "Failed to create" << outputDir.path();
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
        xcursorgen.setArguments({QStringLiteral("--prefix"), renderDir.path(), configDir.filePath(config), outputDir.filePath(cursorName)});
        xcursorgen.start();
        xcursorgen.waitForFinished(-1);
        if (xcursorgen.exitStatus() != QProcess::NormalExit || xcursorgen.exitCode() != 0) {
            qCritical() << "xcursorgen failed:" << xcursorgen.errorString();
            return false;
        }
    }

    // STEP 4: Aliases
    qInfo() << "Making aliases";
    const auto dirs = srcDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &info : dirs) {
        if (!info.isSymbolicLink()) {
            continue;
        }
        const QString aliasName = info.fileName();
        const QString targetName = info.readSymLink();
        QFile f(outputDir.filePath(aliasName));
        if (f.exists()) {
            f.remove();
        }
        if (!QFile::link(targetName, outputDir.filePath(aliasName))) {
            qCritical() << "Failed to create alias" << aliasName << "=>" << targetName;
            return false;
        }
    }

    qInfo() << "SUCCESS";
    return true;
}
