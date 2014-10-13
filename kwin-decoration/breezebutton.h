#ifndef breezebutton_h
#define breezebutton_h

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
#include "breezeanimation.h"
#include "breezehelper.h"

#include <kcommondecoration.h>

namespace Breeze
{
    class Client;

    class Button : public KCommonDecorationButton
    {

        Q_OBJECT

        //* declare animation progress property
        Q_PROPERTY( qreal opacity READ opacity WRITE setOpacity )

        public:

        //* constructor
        explicit Button(Client&, const QString& = QString(), ButtonType = ButtonHelp );

        //* destructor
        ~Button();

        //* destructor
        QSize sizeHint() const;

        //* button type
        ButtonType type( void ) const
        { return _type; }

        //* set force inactive
        /*! returns true if value was actually changed */
        void setForceInactive( const bool& value )
        { _forceInactive = value; }

        //* configuration reset
        virtual void reset( unsigned long );

        //*@name glow animation
        //@{
        void setOpacity( qreal value )
        {
            if( _opacity == value ) return;
            _opacity = value;
            update();
        }

        qreal opacity( void ) const
        { return _opacity; }

        //@}

        protected:

        //* press event
        void mousePressEvent( QMouseEvent* );

        //* release event
        void mouseReleaseEvent( QMouseEvent* );

        //* enter event
        void enterEvent( QEvent* );

        //* leave event
        void leaveEvent( QEvent* );

        //* paint
        void paintEvent( QPaintEvent* );

        //* draw icon
        void drawIcon( QPainter*, QColor foreground, QColor background );

        //* true if animation is in progress
        bool isAnimated( void ) const
        { return _animation->isRunning(); }

        //* true if buttons hover are animated
        bool animationsEnabled( void ) const;

        //*@name button properties
        //@{

        //* true if button if of menu type
        bool isMenuButton( void ) const
        { return _type == ButtonMenu || _type == ButtonItemMenu; }

        //* true if button is of toggle type
        bool isToggleButton( void ) const
        { return _type == ButtonSticky || _type == ButtonAbove || _type == ButtonBelow; }

        //* true if button if of close type
        bool isCloseButton( void ) const
        { return _type == ButtonClose || _type == ButtonItemClose; }

        //* true if button has decoration
        bool hasDecoration( void ) const
        { return !isMenuButton() && _type != ButtonItemClose; }

        //@}

        private Q_SLOTS:
            void slotAppMenuHidden();

        private:

        //* parent client
        Client &_client;

        //! helper
        Helper &_helper;

        //* button type
        ButtonType _type;

        //* button status
        unsigned int _status;

        //* true if button should be forced inactive
        bool _forceInactive;

        //* glow animation
        //Animation::Pointer _animation;
        Animation* _animation;

        //* glow intensity
        qreal _opacity;


    };

} //namespace Breeze

#endif
