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
    QByteArray name() const { return _name; }
    QObject * object() const { return _object; }

protected:
    QObject *_object;
    const QByteArray _name;
};

template<typename T>
class PropertyWrapper: public PropertyWrapperBase {
public:
    using PropertyWrapperBase::PropertyWrapperBase;
    PropertyWrapper(QObject *_object, QByteArray _name): PropertyWrapperBase(_object, std::move(_name))
    {
        QVariant value = object()->property(name());
        if (!value.isValid()) {
            object()->setProperty(name(), T());
        }
    }

    explicit PropertyWrapper(const PropertyWrapper<T> &other) : PropertyWrapperBase(other._object, other._name) {}

    operator T() const { return _object->property(_name).template value<T>(); }
    PropertyWrapper<T> & operator =(const T &value) { _object->setProperty(_name, value); return *this; }

private:
    PropertyWrapper<T> & operator =(const PropertyWrapper &) { return *this; }
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

#if 0
    class AnimationTimeline: public QAbstractAnimation
    {
        Q_OBJECT

    public:
        struct Transition {
            Transition(QByteArray property, QVariant from, QVariant to = QVariant(),
                       qreal normalizedDuration = 0.0,
                       QEasingCurve easingCurve = QEasingCurve::Linear)
                : property(std::move(property))
                , from(std::move(from))
                , to(std::move(to))
                , normalizedDuration(normalizedDuration)
                , easingCurve(std::move(easingCurve))
            {}

            QByteArray property;
            QVariant from;
            QVariant to;
            qreal normalizedDuration;
            QEasingCurve easingCurve;
        };

        struct Entry {
            qreal normalizedStartTime;
            Transition transition;
        };

        AnimationTimeline(QObject *parent, QList<Entry> entries = {})
            : QAbstractAnimation(parent)
            , _entries(std::move(entries))
            , _firstNotStartedIndex(0)
            , _durationMs(0)
        {
            static const auto compareEntry = [](const Entry &a, const Entry &b) {
                return a.normalizedStartTime < b.normalizedStartTime;
            };
            std::sort(_entries.begin(), _entries.end(), compareEntry);
        }

        void setDuration(int durationMs) { _durationMs = durationMs; /* TODO: recalculate animations */ }
        int duration() const override { return _durationMs; }
        // auto *onAnimation = new Timeline(data, {
        //     {.0, {dots[0], 0.0, 1.0, 0.4, QEasingCurve::OutBack}},
        //     {.2, {dots[1], 0.0, 1.0, 0.4, QEasingCurve::OutBack}},
        //     {.4, {dots[2], 0.0, 1.0, 0.4, QEasingCurve::OutBack}},
        //     {.6, {dots[3], 0.0, 1.0, 0.4, QEasingCurve::OutBack}},
        // });

    protected:
        void updateCurrentTime(int currentTime) override {
            for(int i = _firstNotStartedIndex; i < _entries.length(); ++i) {
                int startTime = _durationMs * _entries[i].normalizedStartTime;
                int duration = _durationMs * _entries[i].animator.normalizedDuration();
                int endTime = startTime + duration;

                int startDelay = currentTime - startTime; // < 0 ? OK : started, set current time to diff
                int endDiff = currentTime - endTime; // < 0 ? OK : already ended!!

                if (startDelay < 0) {
                    break;
                }

                // Should be started

                // _entries[i].animator.setTime(startDelay);
                // _entries[i].animator.start();
                // _firstNotStartedIndex = i+1;

                if (endDiff < 0) {
                    continue;
                }

                // Already ended

                // _entries[i].animator.applyFinalState();
            }
        }

    private:
        QList<Entry> _entries;
        int _firstNotStartedIndex;

        int _durationMs;
    };
#endif

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
