#ifndef breezeclientgroupitemdata_h
#define breezeclientgroupitemdata_h

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

#include "breezebutton.h"
#include "breezeanimation.h"
#include "breeze.h"

#include <QList>
#include <QPointer>
#include <QRect>

namespace Breeze
{

    class Client;

    //! tab data
    class ClientGroupItemData
    {

        public:

        //! constructor
        explicit ClientGroupItemData( void )
        {}

        //! destructor
        virtual ~ClientGroupItemData( void )
        {}

        //! reset all rects to argument
        void reset( const QRect& rect )
        {
            _refBoundingRect = rect;
            _startBoundingRect = rect;
            _endBoundingRect = rect;
            _boundingRect = rect;
        }

        //! tab active rect
        QRect _activeRect;

        //! reference bounding rect
        /*! it is usually identical to activeRect unless there is only one tab in window */
        QRect _refBoundingRect;

        //! tab drawing rect
        QRect _startBoundingRect;

        //! tab drawing rect
        QRect _endBoundingRect;

        //! tab drawing rect
        QRect _boundingRect;

        //! tab button
        typedef QPointer<Button> ButtonPointer;
        ButtonPointer _closeButton;

    };

    class ClientGroupItemDataList: public QObject, public QList<ClientGroupItemData>
    {

        Q_OBJECT

        //! declare animation progress property
        Q_PROPERTY( qreal progress READ progress WRITE setProgress )

        public:

        //! invalid item index
        enum { NoItem = -1 };

        //! constructor
        explicit ClientGroupItemDataList( Client* parent );

        //! dirty state
        void setDirty( const bool& value )
        { _dirty = value; }

        //! dirty state
        bool isDirty( void ) const
        { return _dirty; }

        //! enable animations
        void setAnimationsEnabled( bool value )
        { animationsEnabled_ = value; }

        //! animations enabled
        bool animationsEnabled( void ) const
        { return animationsEnabled_; }

        //! true if being animated
        bool isAnimated( void ) const
        { return animationType_ != AnimationNone; }

        //! animation type
        AnimationTypes animationType( void ) const
        { return animationType_; }

        //! return item index matching QPoint, or -1 if none
        int itemAt( const QPoint&, bool ) const;

        //! returns true if index is target
        bool isTarget( int index ) const
        { return index == targetItem_; }

        //! start animation
        /* might need to add the side of the target here */
        void animate( AnimationTypes, int = NoItem );

        //! true if animation is in progress
        bool isAnimationRunning( void ) const
        { return animation().data()->isRunning(); }

        //! update button activity
        void updateButtonActivity( long visibleItem ) const;

        //! update buttons
        void updateButtons( bool alsoUpdate ) const;

        //! target rect
        const QRect& targetRect( void ) const
        { return targetRect_; }

        //!@name animation progress
        //@{

        //! return animation object
        virtual const Animation::Pointer& animation() const
        { return _animation; }

        void setProgress( qreal value )
        {
            if( progress_ == value ) return;
            progress_ = value;
            updateBoundingRects();
        }

        qreal progress( void ) const
        { return progress_; }

        //@}

        protected:

        //! update bounding rects
        void updateBoundingRects( bool alsoUpdate = true );

        private:

        //! client
        Client& _client;

        //! dirty flag
        /* used to trigger update at next paintEvent */
        bool _dirty;

        //! true if animations are enabled
        bool animationsEnabled_;

        //! animation
        Animation::Pointer _animation;

        //! last animation type
        AnimationTypes animationType_;

        //! animation progress
        qreal progress_;

        //! dragged item
        int draggedItem_;

        //! target item
        int targetItem_;

        //! target rect
        QRect targetRect_;

    };

}

#endif
