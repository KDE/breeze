#ifndef breezecheckboxengine_h
#define breezecheckboxengine_h

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
#include "breezebaseengine.h"
#include "breezedatamap.h"
#include "breezecheckboxdata.h"

namespace Breeze
{

    class CheckBoxEngine: public BaseEngine
    {

        Q_OBJECT

    public:

        //* constructor
        explicit CheckBoxEngine( QObject* parent ):
            BaseEngine( parent )
        {}

        //* destructor
        virtual ~CheckBoxEngine()
        {}

        //* register widget
        virtual bool registerWidget( QWidget* );

        //* returns registered widgets
        virtual WidgetList registeredWidgets() const;

        using BaseEngine::registeredWidgets;

        //* true if widget hover state is changed
        virtual bool updateState(const QObject*, CheckBoxState );

        virtual CheckBoxState state( const QObject *widget) const
        {
            DataMap<CheckBoxData>::Value dataPtr = data(widget);
            if(!dataPtr.isNull()) {
                return dataPtr.data()->state();
            }
            return CheckUnknown;
        }

        virtual CheckBoxState previousState( const QObject *widget) const
        {
            DataMap<CheckBoxData>::Value dataPtr = data(widget);
            if(!dataPtr.isNull()) {
                return dataPtr.data()->previousState();
            }
            return CheckUnknown;
        }

        //* true if widget is animated
        bool isAnimated( const QObject* );

        //* duration
        void setEnabled( bool value ) override
        {
            BaseEngine::setEnabled( value );
            _state.setEnabled( value );
        }

        //* duration
        void setDuration( int value ) override
        {
            BaseEngine::setDuration( value );
            _state.setDuration( value );
        }

        public Q_SLOTS:

        //* remove widget from map
        bool unregisterWidget( QObject* object ) override
        {
            if( !object ) return false;
            return _state.unregisterWidget( object );
        }

        //* returns data associated to widget
        DataMap<CheckBoxData>::Value data(const QObject*) const;

        protected:

        DataMap<CheckBoxData> &dataMap();

    private:

        DataMap<CheckBoxData> _state;
    };

}

#endif
