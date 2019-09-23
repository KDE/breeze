#ifndef breezemultistatedata_h
#define breezemultistatedata_h

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

#include "breezegenericdata.h"

namespace Breeze
{

//// //// //// //// /// /// /// // // /  /   /    /

class PropertyWrapperBase {
public:
    PropertyWrapperBase(QObject *object, QByteArray name): _object(object), _name(std::move(name)) {}
    PropertyWrapperBase(const PropertyWrapperBase &other) { *this = other; }
    PropertyWrapperBase & operator =(const PropertyWrapperBase &other) = default;
    virtual ~PropertyWrapperBase() = default;

    const QByteArray & name() const { return _name; }
    QObject * object() const { return _object; }

    operator QVariant() const { return _object->property(_name); }
    virtual PropertyWrapperBase & operator =(const QVariant &value)
    {
        const QVariant oldValue = _object->property(_name);
        if (oldValue.isValid() && oldValue.type() != value.type()) {

            QVariant converted = value;
            bool ok = converted.convert(oldValue.type());
            if(!ok) {
                qDebug("property \"%s\": new value type does not match previous value type (new: %s, old: %s) - trying to cast to original type.",
                       qPrintable(_name), value.typeName(), _object->property(_name).typeName());
                Q_ASSERT(ok);
            }
            _object->setProperty(_name, converted);
        } else {
            _object->setProperty(_name, value);
        }
        return *this;
    }

protected:
    QObject *_object;
    QByteArray _name;
};

template<typename T>
class PropertyWrapper: public PropertyWrapperBase {
public:
    PropertyWrapper(QObject *object_, const QByteArray &name_): PropertyWrapperBase(object_, name_)
    {
        QVariant value = object()->property(name());
        if (!value.isValid()) {
            object()->setProperty(name(), T());
        }
    }
    explicit PropertyWrapper(const PropertyWrapper<T> &other) : PropertyWrapperBase(other._object, other._name) {}
    PropertyWrapper<T> & operator =(const PropertyWrapper<T> &) = delete;
    PropertyWrapperBase & operator =(const PropertyWrapperBase &other) = delete;

    operator T() const { return _object->property(_name).template value<T>(); }
    PropertyWrapper<T> & operator =(const T &value) { _object->setProperty(_name, value); return *this; }
    PropertyWrapper<T> & operator =(const QVariant &value) override { *this = value.value<T>(); return *this; }
};

struct AbstractVariantInterpolator {
    virtual ~AbstractVariantInterpolator() = default;
    virtual QVariant interpolated(const QVariant &from, const QVariant &to, qreal progress) const = 0;
};

// QPropertyAnimation with support for passing custom interpolators as parameter and single
// PropertyWrapper instead of object + property parameters.
class CustomPropertyAnimation: public QPropertyAnimation {
public:
    CustomPropertyAnimation(const PropertyWrapperBase &property, QObject *parent = nullptr):
        QPropertyAnimation(property.object(), property.name(), parent), _interpolator(nullptr) {}

    CustomPropertyAnimation(const PropertyWrapperBase &property,
                            AbstractVariantInterpolator *interpolator,
                            QObject *parent = nullptr)
        : QPropertyAnimation(property.object(), property.name(), parent)
        , _interpolator(interpolator)
    {}

    ~CustomPropertyAnimation() override { delete _interpolator; }

protected:
    QVariant interpolated(const QVariant &from, const QVariant &to, qreal progress) const override {
        if (_interpolator != nullptr) {
            return _interpolator->interpolated(from, to, progress);
        }
        return QPropertyAnimation::interpolated(from, to, progress);
    }

private:
    AbstractVariantInterpolator *_interpolator;
};

//// //// //// //// /// /// /// // // /  /   /    /

#if 0
    class ValueAnimator: public QAbstractAnimation
    {
    public:
        ValueAnimator(QVariant *value, const QVariant &from, const QVariant &to = QVariant(),
                      qreal duration = qQNaN(), QEasingCurve curve = QEasingCurve::Linear);

        qreal normalizedDuration() const;

        int duration() const override
        {
        }

    protected:
        void updateCurrentTime(int currentTime) override
        {
        }

    private:
        QVariant *_value;
        QVariant _from;
        QVariant _to;
        qreal _duration;
        QEasingCurve _curve;
    };

#endif
    class AnimationTimeline: public QAbstractAnimation
    {
        Q_OBJECT

    public:
        struct Entry {
            Entry(qreal normalizedStartTime, const PropertyWrapperBase &property,
                  QVariant from, QVariant to, qreal normalizedDuration,
                  QEasingCurve easingCurve = QEasingCurve::Linear)
                : normalizedStartTime(normalizedStartTime)
                , property(property)
                , from(std::move(from))
                , to(std::move(to))
                , normalizedDuration(normalizedDuration)
                , easingCurve(std::move(easingCurve))
            {}

            Entry(qreal normalizedStartTime, const PropertyWrapperBase &property, QVariant value)
                : normalizedStartTime(normalizedStartTime)
                , property(property)
                , from(QVariant())
                , to(std::move(value))
                , normalizedDuration(0)
            {
            }

            inline bool isSetter() const { return qFuzzyIsNull(normalizedDuration) && !from.isValid(); }
            inline bool isStartingFromCurrentValue() const { return !qFuzzyIsNull(normalizedDuration) && !from.isValid(); }

            qreal normalizedStartTime;
            PropertyWrapperBase property;
            QVariant from;
            QVariant to;
            qreal normalizedDuration;
            QEasingCurve easingCurve;
        };
        using EntryList = QList<Entry>;

        AnimationTimeline(QObject *parent, int duration = 0, EntryList entries = {})
              : QAbstractAnimation(parent)
              , _entries(std::move(entries))
              , _states(QVector<EntryState>(_entries.length()))
              , _durationMs(duration)
        {}

        void setDuration(int durationMs) { _durationMs = durationMs; }
        int duration() const override { return _durationMs; }

        // XXX: remove
        bool isfirst() {
            static void *_loggedobj = parent();
            return _loggedobj == parent();
        }

    Q_SIGNALS:
        void valueChanged();

    protected:
        #define dbg(...) if(isfirst()) { qDebug(__VA_ARGS__); } else {}
        void updateCurrentTime(int currentTime) override {
            bool changed = false;

            for(int i = 0; i < _entries.length(); ++i) {
                auto &entry = _entries[i];
                auto &state = _states[i];

                if(state.processed) {
                    continue;
                }

                int startTime = _durationMs * entry.normalizedStartTime;
                int duration = _durationMs * entry.normalizedDuration;
                int endTime = startTime + duration;

                if (currentTime < startTime) {
                    // Too early for this and all following entries
                    break;
                }


                if (currentTime < endTime) {
                    if (entry.isStartingFromCurrentValue() && !state.from.isValid()) {
                        state.from = entry.property;
                    }

                    // We're here only when endTime > startTime => duration > 0
                    Q_ASSERT(duration > 0);
                    qreal progress = qreal(currentTime - startTime) / duration;

                    const QVariant &from = entry.isStartingFromCurrentValue() ? state.from : entry.from;
                    entry.property = interpolate(from, entry.to, entry.easingCurve.valueForProgress(progress));

                    dbg("t=%4d; i=%2d; start=%4d; duration=%4d; end=%4d; progress=% 4f", currentTime, i, startTime, duration, endTime, progress);
                } else {
                    entry.property = entry.to;
                    state.processed = true;
                    dbg("t=%4d; i=%2d; start=%4d; duration=%4d; end=%4d", currentTime, i, startTime, duration, endTime);
                }
                changed = true;
            }

            if (changed) {
                emit valueChanged();
            }
        }

        void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState) override
        {
            Q_UNUSED(oldState);
            dbg("updateState: %d", newState);

            switch(newState) {
            case Running:
                dbg("Timings for %dms:", _durationMs);
                for (int i = 0; i < _states.length(); ++i) {
                    auto &entry = _entries[i];
                    auto &state = _states[i];

                    state.processed = false;
                    if (entry.isStartingFromCurrentValue()) {
                        state.from = QVariant();
                    }

                    int startTime = _durationMs * entry.normalizedStartTime;
                    int duration = _durationMs * entry.normalizedDuration;
                    int endTime = startTime + duration;
                    dbg("  * % 4d → % 4d; % 4d [%s]", startTime, endTime, duration, qPrintable(entry.property.name()));
                }
            default: break;
            }
        }

    private:
        template <typename T>
        static T interpolateGeneric(const QVariant &from, const QVariant &to, qreal progress) {
            const T a = from.value<T>();
            const T b = to.value<T>();
            return a * (1.0 - progress) + b * progress;
        }

        static QVariant interpolate(const QVariant &from, const QVariant &to, qreal progress)
        {
            switch(QMetaType::Type(from.type())) {
            case QMetaType::Int:
                return interpolateGeneric<int>(from, to, progress);
            case QMetaType::UInt:
                return interpolateGeneric<uint>(from, to, progress);
            case QMetaType::LongLong:
                return interpolateGeneric<long long>(from, to, progress);
            case QMetaType::ULongLong:
                return interpolateGeneric<unsigned long long>(from, to, progress);
            case QMetaType::Float:
                return interpolateGeneric<float>(from, to, progress);
            case QMetaType::Double:
                return interpolateGeneric<double>(from, to, progress);
            case QMetaType::QPoint:
                return interpolateGeneric<QPoint>(from, to, progress);
            case QMetaType::QPointF:
                return interpolateGeneric<QPointF>(from, to, progress);
            default:
                qWarning("Interpolation not supported for type %s", from.typeName());
                return to;
            }
        }

        struct EntryState {
            QVariant from {QVariant()};
            bool processed {false};
        };

        EntryList _entries;
        QVector<EntryState> _states;

        int _durationMs;
    };


////////////////////////////////////////////////////////////////////////////////////////////////////


    //* Tracks arbitrary states (e.g. tri-state checkbox check state)
    class MultiStateData: public GenericData
    {

        Q_OBJECT

        public:

        //* constructor
        MultiStateData( QObject* parent, QWidget* target, int duration, QVariant state = QVariant() ):
            GenericData( parent, target, duration ),
            _initialized( false ),
            _state( state ),
            _previousState( state )
        {}

        //* destructor
        virtual ~MultiStateData()
        {}

        /**
        returns true if state has changed
        and starts timer accordingly
        */
        virtual bool updateState( const QVariant &value );

        virtual QVariant state() const { return _state; }
        virtual QVariant previousState() const { return _previousState; }

        QMap<unsigned, QAbstractAnimation *> anims;

        private:

        bool _initialized;
        QVariant _state;
        QVariant _previousState;

    };

}

#endif
