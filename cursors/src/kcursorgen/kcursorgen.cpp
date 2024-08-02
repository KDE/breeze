/*
    SPDX-FileCopyrightText: 2024 Jin Liu <m.liu.jin@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcursorgen.h"
#include "options.h"

#include <QDir>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QPainter>
#include <QProcess>
#include <QRegularExpression>
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
    QPointF hotspot;
    std::chrono::milliseconds delay;
};

std::optional<SvgCursorMetaDataEntry> SvgCursorMetaDataEntry::parse(const QJsonObject &object)
{
    const QJsonValue fileName = object.value(QLatin1String("filename"));
    if (!fileName.isString()) {
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

    const QJsonValue frametime = object.value(QLatin1String("frametime"));

    return SvgCursorMetaDataEntry{
        .fileName = fileName.toString(),
        .hotspot = QPointF(hotspotX.toDouble(), hotspotY.toDouble()),
        .delay = std::chrono::milliseconds(frametime.toInt()),
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
    if (document.isObject()) {
        if (const auto entry = SvgCursorMetaDataEntry::parse(document.object())) {
            entries.append(entry.value());
        } else {
            return std::nullopt;
        }
    } else if (document.isArray()) {
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

QDir makeOrClearDir(const QString &path)
{
    QDir dir(path);
    if (dir.exists()) {
        for (const QFileInfo &info : dir.entryInfoList(QDir::NoDotAndDotDot)) {
            if (info.isDir()) {
                if (!QDir(info.filePath()).removeRecursively()) {
                    qCritical() << "Failed to remove" << info.filePath();
                    ::exit(2);
                }
            } else {
                if (!dir.remove(info.fileName())) {
                    qCritical() << "Failed to remove" << info.filePath();
                    ::exit(2);
                }
            }
        }
    }
    if (!dir.mkpath(QStringLiteral("."))) {
        qCritical() << "Failed to create dir" << path;
        ::exit(2);
    }
    return dir;
}

KCursorGen::KCursorGen(int &argc, char **argv, QCommandLineParser *parser)
    : QCoreApplication(argc, argv)
{
    m_parser = parser;
    QTimer::singleShot(0, this, &KCursorGen::runMain);
}

KCursorGen::~KCursorGen()
{
}

void KCursorGen::runMain()
{
    bool modeBuildSvgTheme = m_parser->isSet(Options::buildSvgTheme());
    bool modeRenderSvgTheme = m_parser->isSet(Options::renderSvgTheme());
    bool modeSvgThemeToXcursor = m_parser->isSet(Options::svgThemeToXCursor());

    if (modeBuildSvgTheme && !modeRenderSvgTheme && !modeSvgThemeToXcursor) {
        buildSvgTheme();
    } else if (!modeBuildSvgTheme && modeRenderSvgTheme && !modeSvgThemeToXcursor) {
        renderSvgTheme();
    } else if (!modeBuildSvgTheme && !modeRenderSvgTheme && modeSvgThemeToXcursor) {
        svgThemeToXCursor();
    } else {
        m_parser->showHelp(1);
    }

    exit(0);
}

void buildSvgCursor(QDir outputDir, QDir srcDir, QStringList frames, int frametime)
{
    QRegularExpression re(QStringLiteral("^(.+?)(-[0-9]+)?.svg$"));
    QString cursorName = re.match(frames.first()).captured(1);
    bool animated = frames.size() > 1;

    if (!outputDir.mkdir(cursorName)) {
        qCritical() << "Failed to create dir" << outputDir.filePath(cursorName);
        ::exit(2);
    }
    QDir cursorDir(outputDir.filePath(cursorName));

    if (animated) {
        qInfo() << cursorName << "\tis animated:" << frames.size() << "frames";
    }

    int frameNumber = 0;
    QJsonArray metadata;
    for (const QString &frame : frames) {
        QFile svgFile(srcDir.filePath(frame));
        QSvgRenderer renderer(svgFile.fileName());
        if (!renderer.isValid()) {
            qCritical() << "SVG file" << svgFile.fileName() << "is invalid";
            ::exit(2);
        }
        if (!renderer.elementExists("hotspot")) {
            qCritical() << "SVG file" << svgFile.fileName() << "is missing `hotspot` element";
            ::exit(2);
        }
        QPointF hotspot = renderer.transformForElement("hotspot").map(renderer.boundsOnElement("hotspot")).boundingRect().topLeft();
        QRectF viewBox = renderer.viewBoxF();

        if (animated) {
            qInfo() << "    " << frameNumber << "size:" << viewBox.width() << viewBox.height() << "hotspot:" << hotspot.x() << hotspot.y();
        } else {
            qInfo() << cursorName << "\tsize:" << viewBox.width() << viewBox.height() << "hotspot:" << hotspot.x() << hotspot.y();
        }

        svgFile.copy(cursorDir.filePath(frame));

        QJsonObject frameMetadata;
        frameMetadata.insert(QStringLiteral("filename"), frame);
        frameMetadata.insert(QStringLiteral("hotspot_x"), hotspot.x());
        frameMetadata.insert(QStringLiteral("hotspot_y"), hotspot.y());
        if (animated) {
            frameMetadata.insert(QStringLiteral("frametime"), frametime);
        }
        metadata.append(frameMetadata);

        frameNumber++;
    }

    QFile metadataFile(cursorDir.filePath(QStringLiteral("metadata.json")));
    if (!metadataFile.open(QIODevice::WriteOnly)) {
        qCritical() << "Failed to open" << metadataFile.fileName() << "for writing";
        ::exit(3);
    }
    if (animated) {
        metadataFile.write(QJsonDocument(metadata).toJson());
    } else {
        metadataFile.write(QJsonDocument(metadata.first().toObject()).toJson());
    }
}

void KCursorGen::buildSvgTheme()
{
    if (!m_parser->isSet(Options::svgSrcDir())) {
        qCritical() << "Missing <theme-dir> parameter";
        ::exit(1);
    }
    QDir themeDir(m_parser->value(Options::themeDir()));

    if (!m_parser->isSet(Options::svgSrcDir())) {
        qCritical() << "Missing <src-dir> parameter";
        ::exit(1);
    }
    QDir srcDir(m_parser->value(Options::svgSrcDir()));

    const QString aliasFile = m_parser->value(Options::aliasFile());
    const int frametime = m_parser->value(Options::frametime()).toInt();

    QDir svgCursorsDir = makeOrClearDir(themeDir.filePath(SVG_CURSOR_DIR));

    srcDir.setNameFilters({QStringLiteral("*.svg")});

    QStringList srcFiles = srcDir.entryList(QDir::Files, QDir::Name);
    QStringList frames;
    QRegularExpression re(QStringLiteral("^.+-[0-9]+.svg$"));

    for (const QString &srcFile : srcFiles) {
        if (re.match(srcFile).hasMatch()) {
            frames << srcFile;
        } else {
            if (!frames.isEmpty()) {
                buildSvgCursor(svgCursorsDir, srcDir, frames, frametime);
                frames.clear();
            }

            frames << srcFile;
            buildSvgCursor(svgCursorsDir, srcDir, frames, frametime);
            frames.clear();
        }
    }
    if (!frames.isEmpty()) {
        buildSvgCursor(svgCursorsDir, srcDir, frames, frametime);
    }

    if (aliasFile.isEmpty()) {
        return;
    }

    qInfo() << "Making aliases";
    QFile alias(aliasFile);
    if (!alias.open(QIODevice::ReadOnly)) {
        qCritical() << "Failed to open" << alias.fileName() << "for reading";
        ::exit(2);
    }
    QTextStream in(&alias);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (parts.size() != 2) {
            qCritical() << "Invalid alias line:" << line;
            ::exit(3);
        }
        QString aliasName = parts.at(0);
        QString targetName = parts.at(1);
        QFile alias(svgCursorsDir.filePath(aliasName));
        if (alias.exists()) {
            qInfo() << aliasName << "=>" << targetName << "alias already exists, skipped";
            continue;
        }
        if (!QFile::link(targetName, alias.fileName())) {
            qCritical() << "Failed to create alias" << aliasName << "for" << targetName;
            ::exit(5);
        }
        qInfo() << aliasName << "=>" << targetName;
    }
    qInfo() << "SUCCESS";
}

void doRenderSvgTheme(QDir outputDir, QDir themeDir, QString sizes, bool markHotspot)
{
    QDir svgCursorsDir(themeDir.filePath(SVG_CURSOR_DIR));

    QList<int> sizeList;
    QStringList sizeStrings = sizes.split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (const QString &sizeString : sizeStrings) {
        sizeList.push_back(sizeString.toInt());
        makeOrClearDir(outputDir.filePath(sizeString));
    }
    makeOrClearDir(outputDir.filePath(QStringLiteral("config")));

    for (const QString &subdir : svgCursorsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name)) {
        QDir cursorDir(svgCursorsDir.filePath(subdir));
        qInfo() << "Rendering" << subdir;

        const QString metadataFilePath = cursorDir.filePath(QStringLiteral("metadata.json"));
        const auto metadata = SvgCursorMetaData::parse(metadataFilePath);
        if (!metadata.has_value()) {
            qCritical() << "Failed to parse" << metadataFilePath;
            ::exit(3);
        }
        bool animated = metadata->entries.size() > 1;
        QFile configFile(outputDir.filePath(QStringLiteral("config/") + subdir + QStringLiteral(".config")));
        if (!configFile.open(QIODevice::WriteOnly)) {
            qCritical() << "Failed to open" << configFile.fileName() << "for writing";
            ::exit(3);
        }
        QTextStream configStream(&configFile);

        for (int desiredSize : sizeList) {
            QDir outputDirForSize(outputDir.filePath(QString::number(desiredSize)));
            const qreal scale = desiredSize / 24.0;

            for (const SvgCursorMetaDataEntry &entry : metadata->entries) {
                const QString svgPath = cursorDir.filePath(entry.fileName);

                QSvgRenderer renderer(svgPath);
                if (!renderer.isValid()) {
                    qCritical() << "Failed to render" << svgPath;
                    ::exit(3);
                }

                const QRect bounds(QPoint(0, 0), renderer.defaultSize() * scale);
                QImage image(bounds.size(), QImage::Format_ARGB32_Premultiplied);
                image.fill(Qt::transparent);

                QPainter painter(&image);
                renderer.render(&painter, bounds);

                if (markHotspot) {
                    painter.setPen(Qt::red);
                    painter.drawLine(entry.hotspot * scale, entry.hotspot * scale + QPointF(5, 0));
                    painter.drawLine(entry.hotspot * scale, entry.hotspot * scale + QPointF(0, 5));
                }

                painter.end();
                QString pngName = entry.fileName;
                pngName.replace(QStringLiteral(".svg"), QStringLiteral(".png"));
                image.save(outputDirForSize.filePath(pngName));

                QPointF hotspot = entry.hotspot * scale;

                configStream << desiredSize << QStringLiteral(" ") << int(hotspot.x()) << QStringLiteral(" ") << int(hotspot.y()) << QStringLiteral(" ")
                             << QString::number(desiredSize) << QStringLiteral("/") << pngName;
                if (animated) {
                    configStream << QStringLiteral(" ") << entry.delay.count();
                }
                configStream << QStringLiteral("\n");
            }
        }
    }
}

void KCursorGen::renderSvgTheme()
{
    if (!m_parser->isSet(Options::renderDir())) {
        qCritical() << "Missing <render-dir> parameter";
        ::exit(1);
    }
    if (!m_parser->isSet(Options::themeDir())) {
        qCritical() << "Missing <theme-dir> parameter";
        ::exit(1);
    }
    doRenderSvgTheme(QDir(m_parser->value(Options::renderDir())),
                     QDir(m_parser->value(Options::themeDir())),
                     m_parser->value(Options::sizes()),
                     m_parser->isSet(Options::markHotspot()));

    qInfo() << "SUCCESS";
}

void KCursorGen::svgThemeToXCursor()
{
    if (!m_parser->isSet(Options::themeDir())) {
        qCritical() << "Missing <theme-dir> parameter";
        ::exit(1);
    }

    // Generate PNGs and config files
    QDir themeDir(m_parser->value(Options::themeDir()));
    QTemporaryDir tmpDir;
    if (!tmpDir.isValid()) {
        qCritical() << "Failed to create temporary directory";
        ::exit(2);
    }
    QDir renderDir(tmpDir.path());
    doRenderSvgTheme(renderDir, themeDir, m_parser->value(Options::sizes()), m_parser->isSet(Options::markHotspot()));

    // Generate XCursors
    QDir xcursorDir = makeOrClearDir(themeDir.filePath(XCURSOR_DIR));

    QDir configDir(renderDir.filePath(QStringLiteral("config")));
    for (const QString &config : configDir.entryList(QDir::Files, QDir::Name)) {
        QString cursorName = config;
        cursorName.replace(QStringLiteral(".config"), QStringLiteral(""));
        qInfo() << "Generating" << cursorName;
        QProcess xcursorgen;
        xcursorgen.setProgram(QStringLiteral("xcursorgen"));
        xcursorgen.setArguments({QStringLiteral("--prefix"), renderDir.path(), configDir.filePath(config), xcursorDir.filePath(cursorName)});
        xcursorgen.start();
        xcursorgen.waitForFinished(-1);
    }

    // Aliases
    qInfo() << "Making aliases";
    QDir svgCursorDir(themeDir.filePath(SVG_CURSOR_DIR));
    for (const QFileInfo &info : svgCursorDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
        if (!info.isSymbolicLink()) {
            continue;
        }
        QString aliasName = info.fileName();
        QString targetName = info.readSymLink();
        if (!QFile::link(targetName, xcursorDir.filePath(aliasName))) {
            qCritical() << "Failed to create alias" << aliasName << "=>" << targetName;
            ::exit(3);
        }
    }

    qInfo() << "SUCCESS";
}

#include "moc_kcursorgen.cpp"
