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


#include "breezesizegrip.h"

#include "breeze.h"
#include "breezebutton.h"
#include "breezeclient.h"

#include <cassert>
#include <QPainter>
#include <QPolygon>
#include <QTimer>

#include <xcb/xcb.h>

namespace Breeze
{

    //_____________________________________________
    SizeGrip::SizeGrip( Client* client ):
        QWidget(0),
        _client( client )
    {

        setAttribute(Qt::WA_NoSystemBackground );
        setAutoFillBackground( false );

        // cursor
        setCursor( Qt::SizeFDiagCursor );

        // size
        setFixedSize( QSize( GripSize, GripSize ) );

        // mask
        QPolygon p;
        p << QPoint( 0, GripSize )
            << QPoint( GripSize, 0 )
            << QPoint( GripSize, GripSize )
            << QPoint( 0, GripSize );

        setMask( QRegion( p ) );

        // embed
        embed();
        updatePosition();

        // event filter
        client->widget()->installEventFilter( this );

        // show
        show();

    }

    //_____________________________________________
    SizeGrip::~SizeGrip( void )
    {}

    //_____________________________________________
    void SizeGrip::activeChange( void )
    {
        static const quint32 value = XCB_STACK_MODE_ABOVE;
        xcb_configure_window( _client->helper().connection(), winId(), XCB_CONFIG_WINDOW_STACK_MODE, &value );
    }

    //_____________________________________________
    void SizeGrip::embed( void )
    {
        xcb_window_t windowId = _client->windowId();
        if( _client->isPreview() ) {

            setParent( _client->widget() );

        } else if( windowId ) {

            // find client's parent
            xcb_window_t current = windowId;
            xcb_connection_t* connection = _client->helper().connection();
            #if BREEZE_USE_KDE4
            while( true )
            {

                xcb_query_tree_cookie_t cookie = xcb_query_tree_unchecked( connection, current );
                ScopedPointer<xcb_query_tree_reply_t> tree(xcb_query_tree_reply( connection, cookie, nullptr ) );
                if( !tree.isNull() && tree->parent && tree->parent != tree->root && tree->parent != current ) current = tree->parent;
                else break;

            }

            #else
            xcb_query_tree_cookie_t cookie = xcb_query_tree_unchecked( connection, current );
            ScopedPointer<xcb_query_tree_reply_t> tree(xcb_query_tree_reply( connection, cookie, nullptr ) );
            if( !tree.isNull() && tree->parent ) current = tree->parent;
            #endif

            // reparent
            xcb_reparent_window( connection, winId(), current, 0, 0 );
            setWindowTitle( "Breeze::SizeGrip" );

        } else {

            hide();

        }
    }

    //_____________________________________________
    bool SizeGrip::eventFilter( QObject*, QEvent* event )
    {

        if ( event->type() == QEvent::Resize) updatePosition();
        return false;

    }

    //_____________________________________________
    void SizeGrip::paintEvent( QPaintEvent* )
    {

        // get relevant colors
        QColor base( _client->backgroundColor( this, palette(), _client->isActive() ) );

        // create and configure painter
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing );

        painter.setPen( Qt::NoPen );
        painter.setBrush( base );

        // polygon
        QPolygon p;
        p << QPoint( 0, GripSize )
            << QPoint( GripSize, 0 )
            << QPoint( GripSize, GripSize )
            << QPoint( 0, GripSize );
        painter.drawPolygon( p );

    }

    //_____________________________________________
    void SizeGrip::mousePressEvent( QMouseEvent* event )
    {

        switch (event->button())
        {

            case Qt::RightButton:
            {
                hide();
                QTimer::singleShot(5000, this, SLOT(show()));
                break;
            }

            case Qt::MidButton:
            {
                hide();
                break;
            }

            case Qt::LeftButton:
            if( rect().contains( event->pos() ) )
            {

                // check client window id
                if( !_client->windowId() ) break;
                _client->widget()->setFocus();
                if( _client->decoration() )
                { _client->decoration()->performWindowOperation( KDecorationDefines::ResizeOp ); }

            }
            break;

            default: break;

        }

        return;

    }

    //_______________________________________________________________________________
    void SizeGrip::updatePosition( void )
    {

        QPoint position(
            _client->width() - GripSize - Offset,
            _client->height() - GripSize - Offset );

        if( _client->isPreview() )
        {

            position -= QPoint(
                _client->layoutMetric( Client::LM_BorderRight )+
                _client->layoutMetric( Client::LM_OuterPaddingRight ),
                _client->layoutMetric( Client::LM_OuterPaddingBottom )+
                _client->layoutMetric( Client::LM_BorderBottom )
                );

        } else {

            #if BREEZE_USE_KDE4
            position -= QPoint(
                _client->layoutMetric( Client::LM_BorderRight ),
                _client->layoutMetric( Client::LM_BorderBottom ) );
            #else
            position.ry() -= 1 + 2*( _client->titleRect().height() + _client->layoutMetric( Client::LM_TitleEdgeTop ) );
            #endif

        }

        move( position );

    }

}
