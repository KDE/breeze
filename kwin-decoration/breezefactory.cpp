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

#include "breezefactory.h"

#include "breezeclient.h"

#include <KSharedConfig>
#include <KConfigGroup>
#include <KWindowInfo>

#include <QApplication>
#include <QPainter>

#if BREEZE_USE_KDE4
KWIN_DECORATION(Breeze::Factory)
#else
KWIN_DECORATION(BreezePluginFactory, "breezeclient.json", Oxygen::Factory)
#endif

#if !BREEZE_USE_KDE4
#include "breezefactory.moc"
#endif

namespace Breeze
{

    //___________________________________________________
    #if BREEZE_USE_KDE4
    Factory::Factory( void ):

    #else
    Factory::Factory(QObject *parent):
        ParentFactoryClass(parent),
    #endif
        _initialized( false )
        ,_config( KSharedConfig::openConfig( QStringLiteral("breezerc") ) )
        ,_helper( _config )
    {
        readConfig();
        setInitialized( true );

        #if !BREEZE_USE_KDE4
        connect(options(), &KDecorationOptions::configChanged, this, [this]() {
            // read in the configuration
            setInitialized( false );
            readConfig();
            setInitialized( true );
            emit recreateDecorations();
        });

        #endif

    }

    //___________________________________________________
    Factory::~Factory()
    { setInitialized( false ); }

    //___________________________________________________
    KDecoration* Factory::createDecoration(KDecorationBridge* bridge )
    { return (new Client( bridge, this ))->decoration(); }

    //___________________________________________________
    bool Factory::reset(unsigned long)
    {

        #if BREEZE_USE_KDE4
        // read in the configuration
        setInitialized( false );
        readConfig();
        setInitialized( true );
        #endif
        return true;

    }

    //___________________________________________________
    void Factory::readConfig()
    {

        /*
         * always reload helper
         * this is needed to properly handle
         * color contrast settings changed
        */
        _config->reparseConfiguration(); // could be skipped on startup
        _helper.loadConfig();

        // reset shadow tiles
        _shadowTiles = TileSet();

        // initialize default configuration and read
        if( !_defaultConfiguration ) _defaultConfiguration = ConfigurationPtr(new Configuration());
        _defaultConfiguration->setCurrentGroup( QStringLiteral("Windeco") );

        #if BREEZE_USE_KDE4
        _defaultConfiguration->readConfig();
        #else
        _defaultConfiguration->load();
        #endif

    }

    //_________________________________________________________________
    bool Factory::supports( Ability ability ) const
    {
        switch( ability )
        {

            // announce
            case AbilityAnnounceButtons:

            // buttons
            case AbilityButtonMenu:
            case AbilityButtonApplicationMenu:
            case AbilityButtonHelp:
            case AbilityButtonMinimize:
            case AbilityButtonMaximize:
            case AbilityButtonClose:
            case AbilityButtonOnAllDesktops:
            case AbilityButtonAboveOthers:
            case AbilityButtonBelowOthers:
            case AbilityButtonSpacer:
            case AbilityButtonShade:

            // compositing
            case AbilityProvidesShadow:
            return true;

            case AbilityUsesAlphaChannel:
            case AbilityAnnounceAlphaChannel:
            return true;

            // tabs
            case AbilityTabbing:
            return true;

            // no colors supported at this time
            default:
            return false;
        };
    }



    //____________________________________________________________________
    Factory::ConfigurationPtr Factory::configuration( void )
    { return _defaultConfiguration; }

    //_______________________________________________________
    TileSet Factory::shadowTiles( void )
    {
        if( !_shadowTiles.isValid() )
        {

            const QPalette palette( QApplication::palette() );
            const QColor shadowColor( palette.color( QPalette::Shadow ) );

            // pixmap
            QPixmap pixmap = _helper.highDpiPixmap( Metrics::Shadow_Size*2 );
            pixmap.fill( Qt::transparent );

            // gradient
            auto gradientStopColor = [](QColor color, qreal alpha) {
                color.setAlphaF(alpha);
                return color;
            };

            QRadialGradient radialGradient( QPointF(0,0), Metrics::Shadow_Size);
            radialGradient.setColorAt(0.0,  gradientStopColor( shadowColor, 0.35 ) );
            radialGradient.setColorAt(0.25, gradientStopColor( shadowColor, 0.25 ) );
            radialGradient.setColorAt(0.5,  gradientStopColor( shadowColor, 0.13 ) );
            radialGradient.setColorAt(0.75, gradientStopColor( shadowColor, 0.04 ) );
            radialGradient.setColorAt(1.0,  gradientStopColor( shadowColor, 0.0 ) );

            // render
            QPainter painter( &pixmap );
            painter.setRenderHint( QPainter::Antialiasing );
            painter.setCompositionMode(QPainter::CompositionMode_Source);
            painter.setPen( Qt::NoPen );

            const QRectF rect( QPoint( 0, 0 ), pixmap.size()/_helper.devicePixelRatio( pixmap ) );
            painter.translate( rect.center() );
            painter.fillRect( rect.translated( -rect.center() ), radialGradient );

            painter.end();

            // create tiles from pixmap
            _shadowTiles = TileSet( pixmap, Metrics::Shadow_Size, Metrics::Shadow_Size, 1, 1 );

        }

        return _shadowTiles;

    }

}
