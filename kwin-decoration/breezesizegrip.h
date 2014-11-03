#ifndef breezesizegrip_h
#define breezesizegrip_h

/*************************************************************************
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>    *
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

#include "breeze.h"
#include "breezeclient.h"

#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>

namespace Breeze
{

    //* implements size grip for all widgets
    class SizeGrip: public QWidget
    {

        public:

        //* constructor
        explicit SizeGrip( Client* );

        //* constructor
        virtual ~SizeGrip( void );

        //* event filter
        virtual bool eventFilter( QObject*, QEvent* );

        public Q_SLOTS:

        //* update background color
        void activeChange( void );

        protected Q_SLOTS:

        //* embed into parent widget
        void embed( void );

        protected:

        //*@name event handlers
        //@{

        //* paint
        virtual void paintEvent( QPaintEvent* );

        //* mouse press
        virtual void mousePressEvent( QMouseEvent* );

        //@}

        //* update position
        void updatePosition( void );

        private:

        //* grip size
        enum {
            Offset = 0,
            GripSize = 14
        };

        // breeze client
        WeakPointer<Client> _client;

    };


}

#endif
