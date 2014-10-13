#ifndef breezefactory_h
#define breezefactory_h

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
#include "breezeconfiguration.h"
#include "breezehelper.h"
#include "breezetileset.h"

#include "config-breeze.h"

#include <QObject>
#include <kdecorationfactory.h>

namespace Breeze
{

    class Client;

    #if BREEZE_USE_KDE4
    using ParentFactoryClass = KDecorationFactoryUnstable;
    #else
    using ParentFactoryClass = KDecorationFactory;
    #endif

    //* window decoration factory
    class Factory: public ParentFactoryClass
    {

        #if !BREEZE_USE_KDE4
        Q_OBJECT
        #endif

        public:

        #if BREEZE_USE_KDE4

        //* constructor
        explicit Factory( void );

        #else

        //* constructor
        explicit Factory(QObject *parent = nullptr);

        #endif
        //* destructor
        virtual ~Factory();

        //* create decoration
        virtual KDecoration *createDecoration(KDecorationBridge *b);

        //! configuration reset
        virtual bool reset(unsigned long changed);

        //* configuration capabilities
        virtual bool supports( Ability ability ) const;

        //* true if initialized
        virtual bool initialized()
        { return _initialized; }

        //* helper
        virtual Helper& helper( void )
        { return _helper; }

        //* pointer to configuration
        typedef QSharedPointer<Configuration> ConfigurationPtr;

        //* get configuration for a give client
        virtual ConfigurationPtr configuration( void );

        //* shadow tiles
        /** is public because it is also needed for mdi windows */
        TileSet shadowTiles( void );

        protected:

        //* read configuration from KConfig
        void readConfig();

        //* initialization
        void setInitialized( bool value )
        { _initialized = value; }

        private:

        //* initialization flag
        bool _initialized;

        //* config object
        KSharedConfigPtr _config;

        //* helper
        Helper _helper;

        //* tileset
        TileSet _shadowTiles;

        //* default configuration
        ConfigurationPtr _defaultConfiguration;

    };

}

#endif
