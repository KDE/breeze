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
        always reload helper
        this is needed to properly handle
        color contrast settings changed
        */
        _config->reparseConfiguration(); // could be skipped on startup
        _helper.loadConfig();

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

}
