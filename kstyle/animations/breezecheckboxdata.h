#ifndef breezecheckboxdata_h
#define breezecheckboxdata_h

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

#include <QMetaProperty>
#include "breezegenericdata.h"

namespace Breeze
{

#define declvaluetype(v) std::remove_reference<decltype(v)>::type

//// //// //// //// /// /// /// // // /  /   /    /

    static constexpr const qreal qrealQNaN {std::numeric_limits<qreal>::quiet_NaN()}; // TODO: remove if unused anywhere but below
    static constexpr const QPointF invalidPointF {qrealQNaN, qrealQNaN};
    static const auto isInvalidPointF = [](const QPointF &point) { return std::isnan(point.x()) && std::isnan(point.y()); };

    struct CheckBoxRenderState {
        Q_GADGET
        Q_PROPERTY(QPointF position MEMBER position)
        Q_PROPERTY(QPointF linePointPosition0 MEMBER linePointPosition0)
        Q_PROPERTY(QPointF linePointPosition1 MEMBER linePointPosition1)
        Q_PROPERTY(QPointF linePointPosition2 MEMBER linePointPosition2)
        Q_PROPERTY(QPointF pointPosition0 MEMBER pointPosition0)
        Q_PROPERTY(QPointF pointPosition1 MEMBER pointPosition1)
        Q_PROPERTY(QPointF pointPosition2 MEMBER pointPosition2)
        Q_PROPERTY(qreal pointRadius0 MEMBER pointRadius0)
        Q_PROPERTY(qreal pointRadius1 MEMBER pointRadius1)
        Q_PROPERTY(qreal pointRadius2 MEMBER pointRadius2)

    public:
        QPointF position;
        QPointF linePointPosition0, linePointPosition1, linePointPosition2;
        QPointF pointPosition0,     pointPosition1,     pointPosition2;
        qreal   pointRadius0,       pointRadius1,       pointRadius2;

        QString toString() const {
            QString str("CheckBoxRenderState {\n");
            static const auto pointFToString = [](const QPointF &p)->QString {
                if (isInvalidPointF(p)) {
                    return QStringLiteral("(InvalidPointF)");
                }
                return QStringLiteral("{%1, %2}").arg(p.x()).arg(p.y());
            };
            str.append(QStringLiteral("  position = %1,\n").arg(pointFToString(position)));
            str.append(QStringLiteral("  linePointPosition = {%1, %2, %3},\n")
                            .arg(pointFToString(linePointPosition0))
                            .arg(pointFToString(linePointPosition1))
                            .arg(pointFToString(linePointPosition2)));
            str.append(QStringLiteral("  pointPosition = {%1, %2, %3},\n")
                            .arg(pointFToString(pointPosition0))
                            .arg(pointFToString(pointPosition1))
                            .arg(pointFToString(pointPosition2)));
            str.append(QStringLiteral("  pointRadius = {%1, %2, %3},\n")
                            .arg(pointRadius0)
                            .arg(pointRadius1)
                            .arg(pointRadius2));
            str.append("}");
            return str;
        }
    };

    class TimelineAnimation: public QAbstractAnimation {
        Q_OBJECT

    public:
        struct Entry { /**************************************/
            using ActionFunc = void (*)(void *renderState);

            Entry(float relStartTime, ActionFunc action)
                  : relStartTime(relStartTime)
                  , relDuration(0)
                  , action(action)
            {}

            Entry(float relStartTime, QByteArray propertyName, QVariant to)
                  : relStartTime(relStartTime)
                  , relDuration(0)
                  , action(nullptr)
                  , propertyName(std::move(propertyName))
                  , from(QVariant())
                  , to(std::move(to))
            {}

            Entry(float relStartTime, float relDuration, QByteArray propertyName, QVariant from, QVariant to, QEasingCurve easingCurve)
                  : relStartTime(relStartTime)
                  , relDuration(relDuration)
                  , action(nullptr)
                  , propertyName(std::move(propertyName))
                  , from(std::move(from))
                  , to(std::move(to))
                  , easingCurve(std::move(easingCurve))
            {}

            float                       relStartTime;
            float                       relDuration;
            ActionFunc                  action;
            QByteArray                  propertyName;
            QVariant                    from;
            QVariant                    to;
            QEasingCurve                easingCurve;

            inline bool isSetter() const { return qFuzzyIsNull(relDuration) && !from.isValid(); }
            inline bool isStartingFromPreviousValue() const { return !from.isValid() && to.isValid(); }
        }; /*******************************************************/
        using EntryList = QList<Entry>;

        TimelineAnimation(QObject *parent, int durationMs, CheckBoxRenderState *data, const EntryList *transitions = nullptr)
              : QAbstractAnimation(parent)
              , _durationMs(durationMs)
              , _data(q_check_ptr(data))
        {
            setTransitions(transitions);
        }

        void setDuration(int durationMs) { _durationMs = durationMs; }
        int duration() const override { return _durationMs; }

        void setTransitions(const EntryList *transitions) {
            stop();
            _transitions = transitions;
            if(transitions != nullptr) {
                _transitionStates = QVector<TransitionState>(transitions->size());
            } else {
                _transitionStates.clear();
            }
        }

    Q_SIGNALS:
        void valueChanged();

    protected:
        void updateCurrentTime(int currentTime) override {
            if(_transitions == nullptr) {
                return;
            }
            bool changed = false;

            for (int i = 0; i < _transitions->size(); ++i) {
                const Entry &transition = (*_transitions)[i];
                TransitionState &state = _transitionStates[i];
                if (state.processed) {
                    continue;
                }

                int absStartTime = qRound(transition.relStartTime * _durationMs);
                int absEndTime = qRound((transition.relStartTime + transition.relDuration) * _durationMs);

                if (transition.action != nullptr) {
                    if (absEndTime <= currentTime) {
                        transition.action(_data);
                        state.processed = true;
                    }
                    continue;
                }

                int propertyIndex = _data->staticMetaObject.indexOfProperty(transition.propertyName);
                Q_ASSERT(propertyIndex >= 0);
                auto property = _data->staticMetaObject.property(propertyIndex);
                const QVariant value = property.readOnGadget(_data);

                if (absEndTime < currentTime) {
                    // Already ended
                    if (value != transition.to) {
                        property.writeOnGadget(_data, transition.to);
                        changed = true;
                    }
                    state.processed = true;
                } else if (absStartTime <= currentTime) {
                    // Is running
                    if (transition.isStartingFromPreviousValue() && !state.previousValue.isValid()) {
                        state.previousValue = value;
                    }

                    const qreal transitionProgress = (qreal(currentTime)/_durationMs - transition.relStartTime) / transition.relDuration;
                    const QVariant &from = transition.isStartingFromPreviousValue() ? state.previousValue : transition.from;
                    const QVariant newValue = interpolate(from, transition.to, transition.easingCurve.valueForProgress(transitionProgress));
                    if (value != newValue) {
                        property.writeOnGadget(_data, newValue);
                        changed = true;
                    }
                } else {
                    // Too early
                    break;
                }
            }

            if (changed) {
                emit valueChanged();
            }
        }

        void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState) override
        {
            Q_UNUSED(oldState);

            switch(newState) {
            case Running:
                for(auto &state: _transitionStates) {
                    state = TransitionState();
                }
                break;
            default: break;
            }
        }

    private:
        int _durationMs;

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

        CheckBoxRenderState *_data;
        const EntryList *_transitions;
        QVector<TransitionState> _transitionStates;
    };

    struct AbstractState {
        virtual QVariant get(unsigned id) = 0;
        virtual void set(unsigned id, const QVariant &value) = 0;
    };

    struct CheckMarkState {
        QPointF position;
        QPointF linePointPosition[3];
        QPointF pointPosition[3];
        qreal   pointRadius[3];

        enum DataId: unsigned {
            Position,
            LinePointPosition_0, LinePointPosition_1, LinePointPosition_2,
            PointPosition_0,     PointPosition_1,     PointPosition_2,
            PointRadius_0,       PointRadius_1,       PointRadius_2,
        };
    };

    class CheckBoxData: public GenericData
    {
        Q_OBJECT

    public:

        //* constructor
        CheckBoxData( QObject* parent, QWidget* target, int duration, CheckBoxState state = CheckBoxState::CheckUnknown ):
            GenericData( parent, target, duration ),
            _initialized( false ),
            _state( state ),
            _previousState( CheckBoxState::CheckUnknown ),
            timeline(new TimelineAnimation(this, 250, &renderState))
        {
            connect(timeline, &TimelineAnimation::valueChanged, target, QOverload<>::of(&QWidget::update));
        }

        //* destructor
        ~CheckBoxData() override
        {
            timeline->stop();
        }

        /**
        returns true if state has changed
        and starts timer accordingly
        */
        virtual bool updateState( CheckBoxState value );

        virtual CheckBoxState state() const { return _state; }
        virtual CheckBoxState previousState() const { return _previousState; }

        TimelineAnimation *timeline;

        static const CheckBoxRenderState offState;
        static const CheckBoxRenderState onState;
        static const CheckBoxRenderState partialState;

        static const TimelineAnimation::EntryList offToOnTransition;
        static const TimelineAnimation::EntryList onToOffTransition;
        static const TimelineAnimation::EntryList offToPartialTransition;
        static const TimelineAnimation::EntryList partialToOffTransition;
        static const TimelineAnimation::EntryList partialToOnTransition;
        static const TimelineAnimation::EntryList onToPartialTransition;

        CheckBoxRenderState renderState;

    private:

        static void initTransitions();

        bool _initialized;
        CheckBoxState _state;
        CheckBoxState _previousState;
    };

}

#endif