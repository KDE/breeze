#ifndef breeze_helper_h
#define breeze_helper_h

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
#include "config-breeze.h"

#include <KColorScheme>
#include <KSharedConfig>

#if BREEZE_USE_KDE4
#include <KComponentData>
#endif

#include <QBitmap>
#include <QPainterPath>
#include <QWidget>

#if BREEZE_HAVE_X11
#include <QX11Info>
#include <xcb/xcb.h>
#endif

namespace Breeze
{

    //* breeze style helper class.
    /** contains utility functions used at multiple places in both breeze style and breeze window decoration */
    class Helper
    {
        public:

        //* constructor
        explicit Helper( KSharedConfig::Ptr );

        #if BREEZE_USE_KDE4
        //* constructor
        explicit Helper( const QByteArray& );
        #endif

        //* destructor
        virtual ~Helper()
        {}

        //* load configuration
        virtual void loadConfig();

        //* pointer to shared config
        KSharedConfig::Ptr config() const;

        //*@name color utilities
        //@{

        //* add alpha channel multiplier to color
        QColor alphaColor( QColor color, qreal alpha ) const;

        //* negative text color (used for close button)
        QColor negativeTextColor( const QPalette& palette ) const
        { return _viewNegativeTextBrush.brush( palette ).color(); }

        //* shadow
        QColor shadowColor( const QPalette& palette ) const
        { return alphaColor( palette.color( QPalette::Shadow ), 0.15 ); }

        //@}

        //*@name rendering utilities
        //@{

        //* debug frame
        void renderDebugFrame( QPainter*, const QRect& ) const;

        //* returns a region matching given rect, with rounded corners, based on the multipliers
        /** setting any of the multipliers to zero will result in no corners shown on the corresponding side */
        virtual QRegion roundedMask( const QRect&, int left = 1, int right = 1, int top = 1, int bottom = 1 ) const;

        //* returns a region matching given rect, with rounded corners
        virtual QBitmap roundedMask( const QSize&, Corners corners = AllCorners, qreal radius = Metrics::Frame_FrameRadius ) const;

        //* return rounded path in a given rect, with only selected corners rounded, and for a given radius
        QPainterPath roundedPath( const QRect&, Corners = AllCorners, qreal = 4 ) const;

        //@}

        //*@name compositing utilities
        //@{

        //* true if style was compiled for and is running on X11
        static bool isX11( void );

        //* returns true if compositing is active
        bool compositingActive( void ) const;

        //@}

        //@name high dpi utility functions
        //@{

        //* return dpi-aware pixmap of given size
        virtual QPixmap highDpiPixmap( const QSize& size ) const
        { return highDpiPixmap( size.width(), size.height() ); }

        //* return dpi-aware pixmap of given size
        virtual QPixmap highDpiPixmap( int width ) const
        { return highDpiPixmap( width, width ); }

        //* return dpi-aware pixmap of given size
        virtual QPixmap highDpiPixmap( int width, int height ) const;

        //* return device pixel ratio for a given pixmap
        virtual qreal devicePixelRatio( const QPixmap& ) const;

        //@}

        //*@name X11 utilities
        //@{

        #if BREEZE_HAVE_X11

        //* get xcb connection
        static xcb_connection_t* connection( void );

        //* create xcb atom
        xcb_atom_t createAtom( const QString& ) const;

        #endif

        //@}

        protected:

        //* initialize
        void init( void );

        private:

        #if BREEZE_USE_KDE4
        //* component data
        KComponentData _componentData;
        #endif

        //* configuration
        KSharedConfig::Ptr _config;

        //*@name brushes
        //@{
        KStatefulBrush _viewNegativeTextBrush;
        //@}

        #if BREEZE_HAVE_X11

        //* atom used for compositing manager
        xcb_atom_t _compositingManagerAtom;

        #endif

    };

}

#endif
