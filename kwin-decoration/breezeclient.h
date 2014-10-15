#ifndef breezeclient_h
#define breezeclient_h

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

#include "breezeanimation.h"
#include "breezeclientgroupitemdata.h"
#include "breezeconfiguration.h"
#include "breezefactory.h"
#include "breezehelper.h"

#include <kcommondecoration.h>

#include <QBasicTimer>
#include <QTextStream>
#include <QTimerEvent>

#include <xcb/xcb.h>

namespace Breeze
{

    //* convenience typedef for base class
    #if BREEZE_USE_KDE4
    using ParentDecorationClass = KCommonDecorationUnstable;
    #else
    using ParentDecorationClass = KCommonDecoration;
    #endif

    class SizeGrip;
    class Client : public ParentDecorationClass
    {

        Q_OBJECT

        //* declare active state opacity
        Q_PROPERTY( qreal opacity READ opacity WRITE setOpacity )

        public:

        //* constructor
        Client(KDecorationBridge*, Factory* );

        //* destructor
        virtual ~Client();

        //* decoration name
        virtual QString visibleName() const;

        //* buttons
        virtual KCommonDecorationButton *createButton(::ButtonType type);

        //*@name flags
        //@{

        //* true if decoration has iquired behavior
        virtual bool decorationBehaviour(DecorationBehaviour behaviour) const;

        //* true if window is maximized
        virtual bool isMaximized( void ) const
        { return maximizeMode()==MaximizeFull && !configuration()->drawBorderOnMaximizedWindows();  }

        //* true if animations are used
        bool animationsEnabled( void ) const
        { return _configuration->animationsEnabled(); }

        //* true if glow is animated
        bool isAnimated( void ) const
        { return _animation->isRunning(); }

        //* true if titlebar is hidden
        bool hideTitleBar( void ) const
        {
            return
                _configuration->hideTitleBar() &&
                !isShade() &&
                tabCount() == 1;
        }

        //@}

        //* window shape
        virtual void updateWindowShape();

        //* initialization
        virtual void init();

        //* return associated configuration
        Factory::ConfigurationPtr configuration( void ) const
        { return _configuration; }

        //*@name active state animation
        //@{

        void setOpacity( qreal value )
        {
            if( _opacity == value ) return;
            _opacity = value;
            widget()->update();
        }

        qreal opacity( void ) const
        { return _opacity; }

        //@}

        //* helper class
        Helper& helper( void ) const
        { return _factory->helper(); }

        //*@name metrics and color definitions
        //@{

        //* dimensions
        virtual int layoutMetric(LayoutMetric lm, bool respectWindowState = true, const KCommonDecorationButton * = 0) const;

        //* get title rect for untabbed window
        virtual QRect defaultTitleRect( bool active = true ) const;

        //* get title bounding rect
        virtual QRect titleBoundingRect( const QFont& font, const QString& caption ) const
        { return titleBoundingRect( font, titleRect(), caption ); }

        //* get title bounding rect
        virtual QRect titleBoundingRect( const QFont&, QRect, const QString& ) const;

        //@}

        //* title alignment
        inline Qt::Alignment titleAlignment( void ) const;

        //* button size
        inline int buttonSize( void ) const;

        //* frame border
        inline int frameBorder( void ) const;

        //*@name status change methods (overloaded from KCommonDecorationUnstable)
        //@{

        //* triggered when window activity is changed
        virtual void activeChange();

        //* triggered when maximize state changed
        virtual void maximizeChange();

        //* triggered when window shade is changed
        virtual void shadeChange();

        //* triggered when window shade is changed
        virtual void captionChange();

        //@}

        //*@name colors
        //@{

        QColor foregroundColor( void ) const;
        QColor backgroundColor( void ) const;
        QColor outlineColor( void ) const;

        QColor foregroundColor( bool active ) const;
        QColor backgroundColor( bool active ) const;
        QColor outlineColor( bool active ) const;

        //@}

        //* event filter
        virtual bool eventFilter( QObject*, QEvent* );

        //* resize event
        virtual void resizeEvent( QResizeEvent* );

        //* paint background to painter
        void paintBackground( QPainter& ) const;

        public Q_SLOTS:

        //* triggers widget update in titleRect only
        /** one needs to add the title top margin to avoid some clipping glitches */
        void updateTitleRect( void )
        { widget()->update( titleRect().adjusted( 0, -layoutMetric( LM_TitleEdgeTop ), 0, 1 ) ); }

        //* return region for a given defines. This allows to implement extended borders
        QRegion region( KDecorationDefines::Region );

        protected:

        //*@name event filters
        //@{

        //* paint
        virtual void paintEvent( QPaintEvent* );

        //* mouse press event
        virtual bool mousePressEvent( QMouseEvent* );

        //* mouse release event
        virtual bool mouseReleaseEvent( QMouseEvent* );

        //* mouse move event
        virtual bool mouseMoveEvent( QMouseEvent* );

        //* drag enter event
        virtual bool dragEnterEvent( QDragEnterEvent* );

        //* drag move event
        virtual bool dragMoveEvent( QDragMoveEvent* );

        //* drag leave event
        virtual bool dragLeaveEvent( QDragLeaveEvent* );

        //* drop event
        virtual bool dropEvent( QDropEvent* );

        //* timer event
        virtual void timerEvent( QTimerEvent* );

        //@}

        //*@name rendering methods (called in paintEvent)
        //@{

        //* shadow
        virtual void renderShadow( QPainter*, const QRect& ) const;

        //* window background
        virtual void renderBackground( QPainter*, const QRect&, bool isShade ) const;

        //* title text
        /** second color, if valid, is for contrast pixel */
        virtual void renderTitleText( QPainter*, const QRect&, const QColor& ) const;

        //* title text
        /** second color, if valid, is for contrast pixel */
        virtual void renderTitleText( QPainter*, const QRect&, const QString&, const QColor&, bool elide = true ) const;

//         //* title text
//         virtual QPixmap renderTitleText( const QRect&, const QString&, const QColor&, bool elide = true ) const;

        //* GroupItem
        virtual void renderItem( QPainter*, int );

        //* tabbing target rect
        virtual void renderTargetRect( QPainter* );

        //@}

        //* close tab matching give button
        virtual bool closeItem( const Button* );

        //* index of item matching point
        int tabIndexAt( const QPoint& position, bool between = false ) const
        { return _itemData.itemAt( position , between ); }

        //* return pixmap corresponding to a given tab, for dragging
        QPixmap itemDragPixmap( int, QRect, bool drawShadow = false );

        //* calculate mask
        QRegion calcMask( void ) const;

        //*@name size grip
        //@{

        //* create size grip
        void createSizeGrip( void );

        //* delete size grip
        void deleteSizeGrip( void );

        // size grip
        bool hasSizeGrip( void ) const
        { return (bool)_sizeGrip; }

        //* size grip
        SizeGrip& sizeGrip( void ) const
        { return *_sizeGrip; }

        //@}

        //* remove shadow hint
        void removeShadowHint( void );

        protected Q_SLOTS:

        //* set target item to -1
        void clearTargetItem( void );

        //* title bounding rects
        /** calculate and return title bounding rects in case of tabbed window */
        void updateItemBoundingRects( bool alsoUpdate = true );

        //* bound one rect to another
        void boundRectTo( QRect&, const QRect& ) const;

        private Q_SLOTS:

        //* compositing changed
        void updateCompositing();

        //* configuration changed
        void updateConfig();

        private:

        //* factory
        Factory* _factory;

        //* size grip widget
        SizeGrip* _sizeGrip;

        //* configuration
        Factory::ConfigurationPtr _configuration;

        //* glow animation
        Animation* _animation;

        //* glow intensity
        qreal _opacity;

        //* true when initialized
        bool _initialized;

        //* mouse button
        Qt::MouseButton _mouseButton;

        //* tab bounding rects
        ClientGroupItemDataList _itemData;

        //* index of tab being dragged if any, -1 otherwise
        int _sourceItem;

        //* last index before active changed
        /**
         * this is used to detect when activity changed corresponds
         * in fact to active tab change, in which case animations are
         * disabled
         */
        int _lastTabId;

        //* drag start point
        QPoint _dragPoint;

        //* drag start timer.
        /**
        it is needed to activate animations when this was not done via either
        dragMoveEvent or dragLeaveEvent
        */
        QBasicTimer _dragStartTimer;

        //* shadow atom
        xcb_atom_t _shadowAtom;

    };

} // namespace Breeze

//____________________________________________________
Qt::Alignment Breeze::Client::titleAlignment( void ) const
{
    switch( _configuration->titleAlignment() )
    {
        case Configuration::AlignLeft: return Qt::AlignLeft;
        case Configuration::AlignRight: return Qt::AlignRight;

        default:
        case Configuration::AlignCenter:
        case Configuration::AlignCenterFullWidth:
        return Qt::AlignCenter;
    }

}

//____________________________________________________
int Breeze::Client::buttonSize( void ) const
{
    switch( _configuration->buttonSize() )
    {
        case Configuration::ButtonSmall: return 18;

        default:
        case Configuration::ButtonDefault: return 20;
        case Configuration::ButtonLarge: return 24;
        case Configuration::ButtonVeryLarge: return 32;
        case Configuration::ButtonHuge: return 48;
    }

}

//____________________________________________________
int Breeze::Client::frameBorder( void ) const
{
    switch( _configuration->frameBorder() )
    {
        case Configuration::BorderNone: return 0;
        case Configuration::BorderNoSide: return 1;

        default:
        case Configuration::BorderTiny: return 2;
        case Configuration::BorderDefault: return 4;
        case Configuration::BorderLarge: return 8;
        case Configuration::BorderVeryLarge: return 12;
        case Configuration::BorderHuge: return 18;
        case Configuration::BorderVeryHuge: return 27;
        case Configuration::BorderOversized: return 40;
    }

}

#endif
