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
    }

    //_______________________________________________________
    HeaderHelper::~HeaderHelper()
    {
    }

    void HeaderHelper::addToolBar( QToolBar *toolBar )
    {
        _toolbarPositions[toolBar] = Qt::NoToolBarArea;
    }

    void HeaderHelper::removeToolBar( QToolBar *toolBar )
    {
        _toolbarPositions.remove(toolBar);
    }

    void HeaderHelper::notifyToolBarArea( QToolBar *toolBar, Qt::ToolBarArea area )
    {
        if (!_toolbarPositions.contains(toolBar) || _toolbarPositions[toolBar] == area) {
            return;
        }

        _toolbarPositions[toolBar] = area;
        if (area == Qt::TopToolBarArea) {
            QPalette pal = toolBar->palette();
            KColorScheme scheme(QPalette::Normal, KColorScheme::Header);
            pal.setColor(QPalette::Normal, QPalette::Window, scheme.background(KColorScheme::NormalBackground).color());
            pal.setColor(QPalette::Normal, QPalette::WindowText, scheme.foreground(KColorScheme::NormalText).color());
            toolBar->setPalette( pal );
        } else {
            toolBar->setPalette( qApp->palette() );
        }
    }
}
