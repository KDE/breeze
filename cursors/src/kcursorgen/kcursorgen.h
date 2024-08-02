/*
    SPDX-FileCopyrightText: 2024 Jin Liu <m.liu.jin@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QCommandLineParser>
#include <QCoreApplication>

class KCursorGen : public QCoreApplication
{
    Q_OBJECT

public:
    KCursorGen(int &argc, char **argv, QCommandLineParser *parser);
    ~KCursorGen() override;

private Q_SLOTS:
    void runMain();

private:
    void buildSvgTheme();
    void renderSvgTheme();
    void svgThemeToXCursor();

    QCommandLineParser *m_parser;
};
