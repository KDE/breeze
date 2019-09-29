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

//// //// //// //// /// /// /// // // /  /   /    /

    static constexpr const qreal qrealQNaN {std::numeric_limits<qreal>::quiet_NaN()}; // TODO: remove if unused anywhere but below
    static constexpr const QPointF invalidPointF {qrealQNaN, qrealQNaN};
    static const auto isInvalidPointF = [](const QPointF &point) { return std::isnan(point.x()) && std::isnan(point.y()); };

    class Timeline {
    public:
        struct Entry { /**************************************/
            Entry(float relStartTime, float relDuration, unsigned dataId, QVariant from, QVariant to, QEasingCurve easingCurve)
                  : relStartTime(relStartTime)
                  , state(nullptr)
                  , dataId(dataId)
                  , from(std::move(from))
                  , to(std::move(to))
                  , relDuration(relDuration)
                  , easingCurve(std::move(easingCurve))
            {}

            Entry(float relStartTime, unsigned member, QVariant to)
                  : relStartTime(relStartTime)
                  , state(nullptr)
                  , dataId(member)
                  , from(QVariant())
                  , to(std::move(to))
                  , relDuration(0)
            {}

            Entry(float relStartTime, const QVector<QVariant> *state)
                  : relStartTime(relStartTime)
                  , state(q_check_ptr(state))
                  , relDuration(0)
            {}

            float                       relStartTime;
            const QVector<QVariant> *   state;
            unsigned                    dataId;
            QVariant                    from;
            QVariant                    to;
            float                       relDuration;
            QEasingCurve                easingCurve;

            inline bool isSetter() const { return qFuzzyIsNull(relDuration) && !from.isValid(); }
            inline bool isStartingFromPreviousValue() const { return !from.isValid() && to.isValid(); }
        }; /*******************************************************/
        using EntryList = QVector<Entry>;

    public:
        Timeline(QVector<QVariant> *data, const EntryList *transitions = nullptr)
              : _data(q_check_ptr(data))
        {
            setTransitions(transitions);
        }

        void setTransitions(const EntryList *transitions) {
            _transitions = transitions;
            if(transitions != nullptr) {
                _transitionStates = QVector<TransitionState>(transitions->size());
            } else {
                _transitionStates.clear();
            }
            reset();
        }

        bool updateProgress(qreal progress) {
            if(_transitions == nullptr) {
                return false;
            }

            bool changed = false;
            for (int i = 0; i < _transitions->size(); ++i) {
                const Entry &transition = (*_transitions)[i];
                TransitionState &state = _transitionStates[i];
                if (state.processed) {
                    continue;
                }

                float relEndTime = transition.relStartTime + transition.relDuration;

                // State setter entry
                if (transition.state != nullptr) {
                    if (relEndTime <= progress) {
                        *_data = *transition.state;
                        state.processed = true;
                    }
                    continue;
                }

                // Value setter (animated or not)

                Q_ASSERT(transition.dataId < _data->size());
                QVariant &value = (*_data)[transition.dataId];

                if (relEndTime < progress) {
                    // Already ended
                    if (value != transition.to) {
                        value = transition.to;
                        changed = true;
                    }
                    state.processed = true;
                } else if (transition.relStartTime <= progress) {
                    // Is running
                    if (transition.isStartingFromPreviousValue() && !state.previousValue.isValid()) {
                        state.previousValue = value;
                    }

                    const qreal transitionProgress = (progress - transition.relStartTime) / transition.relDuration;
                    const QVariant &from = transition.isStartingFromPreviousValue() ? state.previousValue : transition.from;
                    const QVariant newValue = interpolate(from, transition.to, transition.easingCurve.valueForProgress(transitionProgress));
                    if (value != newValue) {
                        value = newValue;
                        changed = true;
                    }
                } else {
                    // Too early
                    break;
                }
            }

            return changed;
        }

        void reset() {
            for(auto &state: _transitionStates) {
                state = TransitionState();
            }
        }

    private:
        template <typename ValueType>
        static ValueType interpolateGeneric(const QVariant &from, const QVariant &to, qreal progress) {
            const auto a = from.value<ValueType>();
            const auto b = to.value<ValueType>();
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

        struct TransitionState {
            QVariant previousValue {QVariant()};
            bool processed {false};
        };

        QVector<QVariant> *_data;
        const EntryList *_transitions;
        QVector<TransitionState> _transitionStates;
    };

    class TimelineAnimation: public QAbstractAnimation {
        Q_OBJECT

    public:
        TimelineAnimation(QObject *parent, int durationMs, Timeline *timeline)
              : QAbstractAnimation(parent)
              , _durationMs(durationMs)
              , _timeline(q_check_ptr(timeline))
        {}

        void setDuration(int durationMs) { _durationMs = durationMs; }
        int duration() const override { return _durationMs; }

        inline Timeline * timeline() { return _timeline; }

    Q_SIGNALS:
        void valueChanged();

    protected:
        void updateCurrentTime(int currentTime) override {
            Q_ASSERT(_timeline);

            const bool changed = _timeline->updateProgress(qreal(currentTime)/_durationMs);
            if (changed) {
                emit valueChanged();
            }
        }

        void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState) override
        {
            Q_UNUSED(oldState);

            switch(newState) {
            case Running:
                Q_ASSERT(_timeline);

                _timeline->reset();
                break;
            default: break;
            }
        }

    private:
        int _durationMs;
        Timeline *_timeline;
    };

//    template <typename T>
//    class FixedTypeVariant: public QVariant {
//    public:
//        FixedTypeVariant(): QVariant(qMetaTypeId<T>(), nullptr) {
//            static_assert (sizeof(FixedTypeVariant<T>) == sizeof(QVariant));
//        }
//        FixedTypeVariant(const T &val): QVariant(val) {}

//        operator T() const { return value<T>(); }
//        FixedTypeVariant<T> & operator =(const T &value) { setValue(value); return *this; }
//    };

    class CheckMarkRenderer {
//        Q_OBJECT

    public:
        enum DataId: unsigned {
            Position,
            LinePointPosition_0,
            LinePointPosition_1,
            LinePointPosition_2,
            PointPosition_0,
            PointPosition_1,
            PointPosition_2,
            PointRadius_0,
            PointRadius_1,
            PointRadius_2,

            DataIdCount,

            LinePointPosition      = LinePointPosition_0,
            LinePointPosition_Last = LinePointPosition_2,

            PointPosition      = PointPosition_0,
            PointPosition_Last = PointPosition_2,

            PointRadius      = PointRadius_0,
            PointRadius_Last = PointRadius_2,
        };

        void setState(CheckBoxState newState);
        void render(QPainter &painter) {}
    };

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
            _previousState( state ),
            timeline(new Timeline(&variables)),
            anim(new TimelineAnimation(this, 250, timeline))
        {
            connect(anim, &TimelineAnimation::valueChanged, target, QOverload<>::of(&QWidget::update));
        }

        //* destructor
        ~MultiStateData() override
        {
            anim->stop();
            delete timeline;
        }

        /**
        returns true if state has changed
        and starts timer accordingly
        */
        virtual bool updateState( const QVariant &value );

        virtual QVariant state() const { return _state; }
        virtual QVariant previousState() const { return _previousState; }

        QVector<QVariant> variables;
        Timeline *timeline;
        TimelineAnimation *anim;

        private:

        bool _initialized;
        QVariant _state;
        QVariant _previousState;

    };

}

#endif
