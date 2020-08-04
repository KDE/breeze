/*************************************************************************
 * Copyright (C) 2020 by Marco MArtin <mart@kde.org>                     *
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program; if not, write to the                         *
 * Free Software Foundation, Inc.,                                       *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 *************************************************************************/

#include "breezeheaderhelper.h"

#include <QToolBar>
#include <QMenuBar>
#include <QMainWindow>
#include <QGuiApplication>
#include <QDebug>

#include <KColorScheme>

namespace Breeze
{

    //_____________________________________________________
    HeaderHelper::HeaderHelper( QObject* parent):
        QObject( parent )
    {
        if (qApp) {
            connect( qApp, &QGuiApplication::paletteChanged, this, [=]() {
                _validPalette = false;
                for (auto i = _toolbarPositions.constBegin(); i != _toolbarPositions.constEnd(); ++i) {
                    if (i.value() == Qt::TopToolBarArea) {
                        i.key()->setPalette(headerPalette());
                    }
                }

                for (auto *menuBar : _menuBars) {
                    menuBar->setPalette(headerPalette());
                }
            } );
        }
    }

    //_______________________________________________________
    HeaderHelper::~HeaderHelper()
    {
    }

    QPalette HeaderHelper::headerPalette()
    {
        if (_validPalette) {
            return _palette;
        }

        _palette = QPalette();

        KSharedConfigPtr config;

        QString path;
        if (qApp && qApp->property("KDE_COLOR_SCHEME_PATH").isValid()) {
            path = qApp->property("KDE_COLOR_SCHEME_PATH").toString();
            config = KSharedConfig::openConfig(path);
        }

        KColorScheme schemeWindow(QPalette::Active, KColorScheme::Header, config);

        _palette.setBrush(QPalette::Active, QPalette::WindowText, schemeWindow.foreground());
        _palette.setBrush(QPalette::Active, QPalette::Window, schemeWindow.background());

        _palette.setColor(QPalette::Active, QPalette::Light, schemeWindow.shade(KColorScheme::LightShade));
        _palette.setColor(QPalette::Active, QPalette::Midlight, schemeWindow.shade(KColorScheme::MidlightShade));
        _palette.setColor(QPalette::Active, QPalette::Mid, schemeWindow.shade(KColorScheme::MidShade));
        _palette.setColor(QPalette::Active, QPalette::Dark, schemeWindow.shade(KColorScheme::DarkShade));
        _palette.setColor(QPalette::Active, QPalette::Shadow, schemeWindow.shade(KColorScheme::ShadowShade));

        _validPalette = true;

        return _palette;
    }

    void HeaderHelper::addMenuBar( QMenuBar *menuBar )
    {
        menuBar->setPalette( headerPalette() );
        _menuBars.insert(menuBar);
    }

    void HeaderHelper::removeMenuBar( QMenuBar *menuBar )
    {
        if ( !_menuBars.contains( menuBar) ) {
            return;
        }

        menuBar->setPalette( QPalette() );
        _menuBars.remove( menuBar );
    }

    void HeaderHelper::addToolBar( QToolBar *toolBar )
    {
        _toolbarPositions[toolBar] = Qt::NoToolBarArea;
    }

    void HeaderHelper::removeToolBar( QToolBar *toolBar )
    {
        if ( !_toolbarPositions.contains(toolBar) ) {
            return;
        }

        toolBar->setPalette( QPalette() );
        _toolbarPositions.remove( toolBar );
    }

    void HeaderHelper::notifyToolBarArea( QToolBar *toolBar, Qt::ToolBarArea area )
    {
        if ( !_toolbarPositions.contains(toolBar) || _toolbarPositions[toolBar] == area ) {
            return;
        }

        _toolbarPositions[toolBar] = area;
        if (area == Qt::TopToolBarArea) {
            toolBar->setPalette( headerPalette() );
        } else {
            toolBar->setPalette( QPalette() );
        }
    }
}
