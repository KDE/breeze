#ifndef breezeHeaderHelper_h
#define breezeHeaderHelper_h

/*************************************************************************
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>    *
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


#include <QObject>
#include <QHash>
#include <QSet>
#include <QPalette>

class QToolBar;
class QMenuBar;

namespace Breeze
{

    //* forward declaration
    class Helper;

    //* handle shadow pixmaps passed to window manager via X property
    class HeaderHelper: public QObject
    {

        Q_OBJECT

        public:

            //TODO: helper for animations?
        //* constructor
        HeaderHelper( QObject* );

        //* destructor
        ~HeaderHelper() override;

        QPalette headerPalette();

        void addMenuBar( QMenuBar *menuBar );
        void removeMenuBar( QMenuBar *menuBar );

        void addToolBar( QToolBar *toolBar );
        void removeToolBar( QToolBar *toolBar );

        void notifyToolBarArea( QToolBar *toolBar, Qt::ToolBarArea area);

        bool windowHasTopToolBars( QWidget *window ) const;
        bool windowHasMenuBars( QWidget *window ) const;

        private:
            QHash<QToolBar *, Qt::ToolBarArea> _toolbarPositions;

            QHash<QWidget *, QMenuBar *> _menuBarFowWindow;
            QHash<QMenuBar *, QWidget *> _windowForMenuBar;

            QHash<QToolBar *, QWidget *> _windowForToolBar;
            QHash<QWidget *, int> _topToolBarsForWindow;

            QPalette _palette;
            bool _validPalette = false;
    };

}

#endif
