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


#include "breezehelper.h"
#include "breeze.h"

#include <KColorUtils>
#include <KWindowSystem>

#include <QApplication>
#include <QPainter>

#if BREEZE_HAVE_X11 && QT_VERSION < 0x050000
#include <X11/Xlib-xcb.h>
#endif

namespace Breeze
{
    //____________________________________________________________________
    Helper::Helper( KSharedConfig::Ptr config ):
        _config( config )
    { init(); }

    //____________________________________________________________________
    #if BREEZE_USE_KDE4
    Helper::Helper( const QByteArray& name ):
        _componentData( name, 0, KComponentData::SkipMainComponentRegistration ),
        _config( _componentData.config() )
    { init(); }
    #endif

    //____________________________________________________________________
    KSharedConfig::Ptr Helper::config() const
    { return _config; }

    //____________________________________________________________________
    void Helper::loadConfig()
    {
        _viewNegativeTextBrush = KStatefulBrush( KColorScheme::View, KColorScheme::NegativeText, _config );
    }

    //____________________________________________________________________
    QColor Helper::alphaColor( QColor color, qreal alpha ) const
    {
        if( alpha >= 0 && alpha < 1.0 )
        { color.setAlphaF( alpha*color.alphaF() ); }
        return color;
    }

    //______________________________________________________________________________
    void Helper::renderDebugFrame( QPainter* painter, const QRect& rect ) const
    {
        painter->save();
        painter->setRenderHints( QPainter::Antialiasing );
        painter->setBrush( Qt::NoBrush );
        painter->setPen( Qt::red );
        painter->drawRect( QRectF( rect ).adjusted( 0.5, 0.5, -0.5, -0.5 ) );
        painter->restore();
    }

    //______________________________________________________________________________
    bool Helper::isX11( void )
    {
        #if BREEZE_HAVE_X11
        #if QT_VERSION >= 0x050000
        static bool isX11 = QApplication::platformName() == QStringLiteral("xcb");
        return isX11;
        #else
        return true;
        #endif
        #endif

        return false;

    }

    //________________________________________________________________________________________________________
    bool Helper::compositingActive( void ) const
    {

        #if BREEZE_HAVE_X11
        if( isX11() )
        {
            // direct call to X
            xcb_get_selection_owner_cookie_t cookie( xcb_get_selection_owner( connection(), _compositingManagerAtom ) );
            ScopedPointer<xcb_get_selection_owner_reply_t> reply( xcb_get_selection_owner_reply( connection(), cookie, nullptr ) );
            return reply && reply->owner;

        }
        #endif

        // use KWindowSystem
        return KWindowSystem::compositingActive();

    }

    //______________________________________________________________________________________
    QPixmap Helper::highDpiPixmap( int width, int height ) const
    {
        #if QT_VERSION >= 0x050300
        const qreal dpiRatio( qApp->devicePixelRatio() );
        QPixmap pixmap( width*dpiRatio, height*dpiRatio );
        pixmap.setDevicePixelRatio( dpiRatio );
        return pixmap;
        #else
        return QPixmap( width, height );
        #endif
    }

    //______________________________________________________________________________________
    qreal Helper::devicePixelRatio( const QPixmap& pixmap ) const
    {
        #if QT_VERSION >= 0x050300
        return pixmap.devicePixelRatio();
        #else
        Q_UNUSED(pixmap);
        return 1;
        #endif
    }

    #if BREEZE_HAVE_X11

    //____________________________________________________________________
    xcb_connection_t* Helper::connection( void )
    {

        #if QT_VERSION >= 0x050000
        return QX11Info::connection();
        #else
        static xcb_connection_t* connection = nullptr;
        if( !connection )
        {
            Display* display = QX11Info::display();
            if( display ) connection = XGetXCBConnection( display );
        }
        return connection;
        #endif
    }

    //____________________________________________________________________
    xcb_atom_t Helper::createAtom( const QString& name ) const
    {
        if( isX11() )
        {

            xcb_connection_t* connection( Helper::connection() );
            xcb_intern_atom_cookie_t cookie( xcb_intern_atom( connection, false, name.size(), qPrintable( name ) ) );
            ScopedPointer<xcb_intern_atom_reply_t> reply( xcb_intern_atom_reply( connection, cookie, nullptr) );
            return reply ? reply->atom:0;

        } else return 0;

    }

    #endif

    //____________________________________________________________________
    void Helper::init( void )
    {
        #if BREEZE_HAVE_X11

        if( isX11() )
        {
            // create compositing screen
            const QString atomName( QStringLiteral( "_NET_WM_CM_S%1" ).arg( QX11Info::appScreen() ) );
            _compositingManagerAtom = createAtom( atomName );
        }

        #endif

    }

}
