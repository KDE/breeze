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

                for (auto *menuBar : _menuBarFowWindow.values()) {
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

    QColor HeaderHelper::transitionHeaderColor( const QWidget *widget, qreal progress ) const
    {
        QColor color1 = !widget || widget->isEnabled() ? _palette.color( QPalette::Inactive, QPalette::Window ) : _palette.color( QPalette::Disabled, QPalette::Window );
        QColor color2 = _palette.color( QPalette::Active, QPalette::Window );

        return QColor(255 * (color1.redF() * (1-progress) + color2.redF() * progress),
                      255 * (color1.greenF() * (1-progress) + color2.greenF() * progress),
                      255 * (color1.blueF() * (1-progress) + color2.blueF() * progress) );
    }

    void HeaderHelper::addMenuBar( QMenuBar *menuBar )
    {
        if (!menuBar->window()) {
            return;
        }
        menuBar->setPalette( headerPalette() );

        _menuBarFowWindow.insert(menuBar->window(), menuBar);
        _windowForMenuBar.insert(menuBar, menuBar->window());
    }

    void HeaderHelper::removeMenuBar( QMenuBar *menuBar )
    {
        if ( !_menuBarFowWindow.contains( menuBar ) ) {
            return;
        }

        QWidget *window = _windowForMenuBar.take( menuBar );
        _menuBarFowWindow.remove( window );
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

        QWidget *window = toolBar->window();
        if ( !window ) {
            return;
        }

        if ( _toolbarPositions[toolBar] == Qt::TopToolBarArea ) {
            _topToolBarsForWindow[window] = qMin( 0, _topToolBarsForWindow[window] - 1 );
        }
        toolBar->setPalette( QPalette() );

        _toolbarPositions.remove( toolBar );
        _windowForToolBar.remove( toolBar );
    }

    void HeaderHelper::notifyToolBarArea( QToolBar *toolBar, Qt::ToolBarArea area )
    {
        QWidget *window = toolBar->window();
        QWidget *oldWindow = _windowForToolBar.value( toolBar );

        const bool windowChanged = window != oldWindow;
        const bool areaChanged = _toolbarPositions[toolBar] != area;

        if ( !_toolbarPositions.contains(toolBar) || (!windowChanged && !areaChanged)) {
            return;
        }

        if (window && area == Qt::TopToolBarArea) {
            toolBar->setPalette( headerPalette() );
            _topToolBarsForWindow[window]++;

            if ( _topToolBarsForWindow[window] == 1 ) {
                auto *menuBar = _menuBarFowWindow.value(window);
                if ( menuBar ) {
                    menuBar->update();
                }
            }
        }

        auto decrementTopToolBarsForWindow = [this] (QWidget *window) {
            if ( !window ) {
                return;
            }
            _topToolBarsForWindow[window] = qMin( 0, _topToolBarsForWindow[window] - 1 );

            if ( _topToolBarsForWindow[window] == 0 ) {
                auto *menuBar = _menuBarFowWindow.value(window);
                if ( menuBar ) {
                    menuBar->update();
                }
            }
        };

        if ( area != Qt::TopToolBarArea ) {
            toolBar->setPalette( QPalette() );
            decrementTopToolBarsForWindow( window );
        }

        if ( windowChanged ) {
            _windowForToolBar[ toolBar ] = window;
            decrementTopToolBarsForWindow( oldWindow );
        }
    }

    bool HeaderHelper::windowHasTopToolBars( QWidget *window ) const
    {
        return _topToolBarsForWindow.value( window ) > 0;
    }
    
    bool HeaderHelper::windowHasMenuBars( QWidget *window ) const
    {
        return _menuBarFowWindow.contains( window );
    }
}
