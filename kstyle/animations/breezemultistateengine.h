#ifndef breezemultistateengine_h
#define breezemultistateengine_h

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
#include "breezemultistatedata.h"

namespace Breeze
{

    //* used for simple widgets
    class MultiStateEngine: public BaseEngine
    {

        Q_OBJECT

    public:

        //* constructor
        explicit MultiStateEngine( QObject* parent ):
            BaseEngine( parent )
        {}

        //* destructor
        virtual ~MultiStateEngine()
        {}

        //* register widget
        virtual bool registerWidget( QWidget* );

        //* returns registered widgets
        virtual WidgetList registeredWidgets() const;

        using BaseEngine::registeredWidgets;

        //* true if widget hover state is changed
        virtual bool updateState( const QObject*, const QVariant & );

        virtual QVariant state( const QObject *widget) const
        {
            DataMap<MultiStateData>::Value dataPtr = data(widget);
            if(!dataPtr.isNull()) {
                return dataPtr.data()->state();
            }
            return QVariant();
        }

        virtual QVariant previousState( const QObject *widget) const
        {
            DataMap<MultiStateData>::Value dataPtr = data(widget);
            if(!dataPtr.isNull()) {
                return dataPtr.data()->previousState();
            }
            return QVariant();
        }

        //* true if widget is animated
        virtual bool isAnimated( const QObject* );

        //* animation opacity
        virtual qreal opacity( const QObject* object)
        { return isAnimated( object) ? data( object).data()->opacity(): AnimationData::OpacityInvalid; }

        //* duration
        virtual void setEnabled( bool value )
        {
            BaseEngine::setEnabled( value );
            _state.setEnabled( value );
        }

        //* duration
        virtual void setDuration( int value )
        {
            BaseEngine::setDuration( value );
            _state.setDuration( value );
        }

        public Q_SLOTS:

        //* remove widget from map
        virtual bool unregisterWidget( QObject* object )
        {
            if( !object ) return false;
            return _state.unregisterWidget( object );
        }

        //* returns data associated to widget
        DataMap<MultiStateData>::Value data( const QObject*) const;

        protected:

        DataMap<MultiStateData> &dataMap();

    private:

        DataMap<MultiStateData> _state;
    };

}

#endif
