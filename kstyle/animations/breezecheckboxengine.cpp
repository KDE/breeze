
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

#include "breezecheckboxengine.h"

namespace Breeze
{

    //____________________________________________________________
    bool CheckBoxEngine::registerWidget( QWidget* widget)
    {

        if( !widget ) return false;
        if( !_state.contains( widget ) ) { _state.insert( widget, new CheckBoxData( this, widget, duration() ), enabled() ); }

        // connect destruction signal
        connect( widget, SIGNAL(destroyed(QObject*)), this, SLOT(unregisterWidget(QObject*)), Qt::UniqueConnection );

        return true;

    }

    //____________________________________________________________
    BaseEngine::WidgetList CheckBoxEngine::registeredWidgets() const
    {

        WidgetList out;

        using Value = DataMap<CheckBoxData>::Value;

        for(const Value& value: _state) {
            if( value ) out.insert( value.data()->target().data() );
        }

        return out;

    }

    //____________________________________________________________
    bool CheckBoxEngine::updateState( const QObject* object, CheckBoxState value )
    {
        DataMap<CheckBoxData>::Value data( CheckBoxEngine::data( object ) );
        return ( data && data.data()->updateState( value ) );
    }

    //____________________________________________________________
    bool CheckBoxEngine::isAnimated( const QObject* object)
    {

        DataMap<CheckBoxData>::Value data( CheckBoxEngine::data( object) );
        return ( data && data.data()->timeline && data.data()->timeline->isRunning() );

    }

    //____________________________________________________________
    DataMap<CheckBoxData>::Value CheckBoxEngine::data(const QObject* object) const
    {

        return _state.find( object ).data();

    }

    //____________________________________________________________
    DataMap<CheckBoxData>& CheckBoxEngine::dataMap()
    {

        return _state;

    }

}
