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

#include "breezecheckboxdata.h"

namespace Breeze
{
    const CheckBoxRenderState CheckBoxData::offState {
        /* Position          */ QPointF(0, 0),
        /* LinePointPosition */ invalidPointF, invalidPointF, invalidPointF,
        /* PointPosition     */ invalidPointF, invalidPointF, invalidPointF,
        /* PointRadius       */ 0.0f, 0.0f, 0.0f
    };
    const CheckBoxRenderState CheckBoxData::onState {
        /* Position          */ QPointF(-1, 3),
        /* LinePointPosition */ QPointF(-3, -3), QPointF(0, 0), QPointF(5, -5),
        /* PointPosition     */ invalidPointF, invalidPointF, invalidPointF,
        /* PointRadius       */ 0.0f, 0.0f, 0.0f,
    };
    const CheckBoxRenderState CheckBoxData::partialState {
        /* Position          */ QPointF(0, 0),
        /* LinePointPosition */ invalidPointF, invalidPointF, invalidPointF,
        /* PointPosition     */ QPointF(-4, 0), QPointF( 0, 0), QPointF(4,  0),
        /* PointRadius       */ 1.0f, 1.0f, 1.0f,
    };

    const TimelineAnimation::EntryList CheckBoxData::offToOnTransition {
        {0.0f,       [](void *renderState) { *static_cast<CheckBoxRenderState*>(renderState) = CheckBoxData::offState; }},
        {0.0f,       "position",            CheckBoxData::onState.position},
        {0.0f,       "linePointPosition0",  CheckBoxData::onState.linePointPosition0},
        {0.0f, 0.4f, "linePointPosition1",  CheckBoxData::onState.linePointPosition0,   CheckBoxData::onState.linePointPosition1, QEasingCurve::InOutCubic},
        {0.5f, 0.5f, "linePointPosition2",  CheckBoxData::onState.linePointPosition1,   CheckBoxData::onState.linePointPosition2, QEasingCurve::InOutCubic},
        {1.0f,       [](void *renderState) { *static_cast<CheckBoxRenderState*>(renderState) = CheckBoxData::onState; }},
    };

    const TimelineAnimation::EntryList CheckBoxData::onToOffTransition {
        {0.0f,       [](void *renderState) { *static_cast<CheckBoxRenderState*>(renderState) = CheckBoxData::onState; }},
        {0.0f, 0.5f, "linePointPosition0", CheckBoxData::onState.linePointPosition0, CheckBoxData::onState.linePointPosition1, QEasingCurve::InOutCubic},
        {0.6f, 0.4f, "linePointPosition0", CheckBoxData::onState.linePointPosition1, CheckBoxData::onState.linePointPosition2, QEasingCurve::InOutCubic},
        {0.6f, 0.4f, "linePointPosition1", CheckBoxData::onState.linePointPosition1, CheckBoxData::onState.linePointPosition2, QEasingCurve::InOutCubic},
        {1.0f,       [](void *renderState) { *static_cast<CheckBoxRenderState*>(renderState) = CheckBoxData::offState; }},
    };

    const TimelineAnimation::EntryList CheckBoxData::offToPartialTransition {
        {0.0f,       [](void *renderState) { *static_cast<CheckBoxRenderState*>(renderState) = CheckBoxData::offState; }},
        {0.0f,       "pointPosition0", CheckBoxData::partialState.pointPosition0},
        {0.0f,       "pointPosition1", CheckBoxData::partialState.pointPosition1},
        {0.0f,       "pointPosition2", CheckBoxData::partialState.pointPosition2},
        {0.0f, 0.6f, "pointRadius0",   QVariant(), CheckBoxData::partialState.pointRadius0, QEasingCurve::OutCubic},
        {0.2f, 0.6f, "pointRadius1",   QVariant(), CheckBoxData::partialState.pointRadius1, QEasingCurve::OutCubic},
        {0.4f, 0.6f, "pointRadius2",   QVariant(), CheckBoxData::partialState.pointRadius2, QEasingCurve::OutCubic},
        {1.0f,       [](void *renderState) { *static_cast<CheckBoxRenderState*>(renderState) = CheckBoxData::partialState; }},
    };
    const TimelineAnimation::EntryList CheckBoxData::partialToOffTransition {
        {0.0f,       [](void *renderState) { *static_cast<CheckBoxRenderState*>(renderState) = CheckBoxData::partialState; }},
        {0.0f, 0.6f, "pointRadius0",   CheckBoxData::partialState.pointRadius0, CheckBoxData::offState.pointRadius0, QEasingCurve::InCubic},
        {0.2f, 0.6f, "pointRadius1",   CheckBoxData::partialState.pointRadius1, CheckBoxData::offState.pointRadius1, QEasingCurve::InCubic},
        {0.4f, 0.6f, "pointRadius2",   CheckBoxData::partialState.pointRadius2, CheckBoxData::offState.pointRadius2, QEasingCurve::InCubic},
        {1.0f,       [](void *renderState) { *static_cast<CheckBoxRenderState*>(renderState) = CheckBoxData::offState; }},
    };

    static const float partialPointRadiusSqrt2 = CheckBoxData::partialState.pointRadius0 * sqrtf(2);
    static const float partialPointRadiusSqrt3 = CheckBoxData::partialState.pointRadius0 * sqrtf(3);
    static const QPointF onStateAbsLinePointPosition2 = CheckBoxData::onState.linePointPosition2 + CheckBoxData::onState.position;

    const TimelineAnimation::EntryList CheckBoxData::partialToOnTransition {
        {0.0f,       [](void *renderState) { *static_cast<CheckBoxRenderState*>(renderState) = CheckBoxData::partialState; }},
        {0.0f, 0.5f, "pointPosition0",      CheckBoxData::partialState.pointPosition0,  CheckBoxData::partialState.pointPosition1,  QEasingCurve::InOutCubic},
        {0.0f, 0.5f, "pointPosition2",      CheckBoxData::partialState.pointPosition2,  CheckBoxData::partialState.pointPosition1,  QEasingCurve::InOutCubic},
        {0.0f, 0.5f, "pointRadius0",        CheckBoxData::partialState.pointRadius0,    partialPointRadiusSqrt3,                    QEasingCurve::InOutCubic},
        {0.0f, 0.5f, "pointRadius1",        CheckBoxData::partialState.pointRadius1,    partialPointRadiusSqrt3,                    QEasingCurve::InOutCubic},
        {0.0f, 0.5f, "pointRadius2",        CheckBoxData::partialState.pointRadius2,    partialPointRadiusSqrt3,                    QEasingCurve::InOutCubic},
        {0.5f,       "pointPosition0",      invalidPointF},
        {0.5f,       "pointPosition2",      invalidPointF},

        {0.6f, 0.4f, "position",            CheckBoxData::partialState.position,        CheckBoxData::onState.position,             QEasingCurve::InOutCubic},
        {0.6f, 0.4f, "pointPosition1",      CheckBoxData::partialState.pointPosition1,  CheckBoxData::onState.position,             QEasingCurve::InOutCubic},
        {0.6f, 0.4f, "pointRadius1",        partialPointRadiusSqrt3,                    CheckBoxData::onState.pointRadius1,         QEasingCurve::InOutCubic},
        {0.6f,       "linePointPosition1",  CheckBoxData::onState.linePointPosition1},
        {0.6f, 0.4f, "linePointPosition0",  CheckBoxData::onState.linePointPosition1,   CheckBoxData::onState.linePointPosition0,   QEasingCurve::InOutCubic},
        {0.6f, 0.4f, "linePointPosition2",  CheckBoxData::onState.linePointPosition1,   CheckBoxData::onState.linePointPosition2,   QEasingCurve::InOutCubic},
        {1.0f,       [](void *renderState) { *static_cast<CheckBoxRenderState*>(renderState) = CheckBoxData::onState; }},
    };
    const TimelineAnimation::EntryList CheckBoxData::onToPartialTransition {
        {0.0f,       [](void *renderState) { *static_cast<CheckBoxRenderState*>(renderState) = CheckBoxData::onState; }},
        {0.0f, 0.4f, "position",            QVariant(),                                 CheckBoxData::partialState.position,        QEasingCurve::InOutCubic},
        {0.0f, 0.4f, "linePointPosition0",  QVariant(),                                 CheckBoxData::onState.linePointPosition1,   QEasingCurve::InOutCubic},
        {0.0f, 0.4f, "linePointPosition2",  QVariant(),                                 CheckBoxData::onState.linePointPosition1,   QEasingCurve::InOutCubic},
        {0.0f, 0.4f, "pointPosition1",      CheckBoxData::onState.position,             CheckBoxData::partialState.pointPosition1,  QEasingCurve::InOutCubic},
        {0.0f, 0.4f, "pointRadius1",        QVariant(),                                 partialPointRadiusSqrt3,                    QEasingCurve::InOutCubic},

        {0.5f, 0.5f, "pointPosition0",      CheckBoxData::partialState.pointPosition1,  CheckBoxData::partialState.pointPosition0,  QEasingCurve::InOutCubic},
        {0.5f, 0.5f, "pointPosition2",      CheckBoxData::partialState.pointPosition1,  CheckBoxData::partialState.pointPosition2,  QEasingCurve::InOutCubic},
        {0.5f, 0.5f, "pointRadius0",        partialPointRadiusSqrt3,                    CheckBoxData::partialState.pointRadius0,    QEasingCurve::InOutCubic},
        {0.5f, 0.5f, "pointRadius1",        partialPointRadiusSqrt3,                    CheckBoxData::partialState.pointRadius1,    QEasingCurve::InOutCubic},
        {0.5f, 0.5f, "pointRadius2",        partialPointRadiusSqrt3,                    CheckBoxData::partialState.pointRadius2,    QEasingCurve::InOutCubic},
        {1.0f,       [](void *renderState) { *static_cast<CheckBoxRenderState*>(renderState) = CheckBoxData::partialState; }},
    };

    //______________________________________________
    bool CheckBoxData::updateState( CheckBoxState value )
    {
        if( !_initialized )
        {

            _state = value;
            _initialized = true;
            return false;

        } else if( _state == value ) {

            return false;

        } else {

            _previousState = _state;
            _state = value;
            animation().data()->setDirection(Animation::Forward);
            if( !animation().data()->isRunning() ) animation().data()->start();
            return true;

        }

    }

}
