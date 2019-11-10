#include "breezestyle.h"

#include "breeze.h"
#include "breezeanimations.h"
#include "breezecheckboxdata.h"

#include <KColorUtils>
#include <QPainter>

#include <QEasingCurve>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPauseAnimation>

#include <cmath>
#include <QDebug>

namespace Breeze
{

static void renderCheckMark(QPainter *painter, const QPoint &position, const QColor &color,
                            const CheckBoxRenderState &s)
{
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    QPainterPath pp;

    const QPointF linePointPosition[] = {s.linePointPosition0, s.linePointPosition1, s.linePointPosition2};
    const QPointF pointPosition[] = {s.pointPosition0, s.pointPosition1, s.pointPosition2};
    const qreal pointRadius[] = {s.pointRadius0, s.pointRadius1, s.pointRadius2};

    int i = 0;
    for(; i < 3; ++i) {
        if(!isInvalidPointF(linePointPosition[i])) {
            pp.moveTo(linePointPosition[i]);
            break;
        }
    }
    for(; i < 3; ++i) {
        if(!isInvalidPointF(linePointPosition[i])) {
            pp.lineTo(linePointPosition[i]);
        }
    }
    pp.translate(s.position + position);
    painter->drawPath(pp);

    painter->setPen(Qt::NoPen);
    for (int i = 0; i < 3; ++i) {
        if (isInvalidPointF(pointPosition[i]) || qFuzzyIsNull(pointRadius[i])) {
            continue;
        }
        painter->setBrush(color);
        painter->drawEllipse(pointPosition[i] + position, pointRadius[i], pointRadius[i]);
    }
}

//___________________________________________________________________________________
void Style::drawChoicePrimitive(const QStyleOption *option, QPainter *painter, const QWidget* widget, bool isRadioButton) const
{
    // copy rect and palette
    const auto& rect( option->rect );
    const auto& palette( option->palette );

    // copy state
    const State& state( option->state );
    const bool enabled( state & State_Enabled );
    // State_Selected can be active in list and menu items
    const bool mouseOver( enabled && ( state & (State_MouseOver | State_Selected) ) );
    const bool hasFocus( enabled && ( state & (State_HasFocus | State_Selected) ) );
    const bool isChecked( state & State_On );

    // focus takes precedence over mouse over
    _animations->widgetStateEngine().updateState( widget, AnimationFocus, hasFocus );
    _animations->widgetStateEngine().updateState( widget, AnimationHover, mouseOver );
    _animations->widgetStateEngine().updateState( widget, AnimationPressed, isChecked );

    // Render background and frame

    // Foreground and background color

    const auto &normalBackground = palette.color(QPalette::Base);
    const auto &normalForeground = palette.color(QPalette::Text);
    const auto &checkedBackground  = palette.color(QPalette::Highlight);
    const auto &checkedForeground  = palette.color(QPalette::HighlightedText);

    const qreal stateOpacityOrInvalid = _animations->widgetStateEngine().opacity( widget, AnimationPressed );
    const qreal stateOpacity = stateOpacityOrInvalid != AnimationData::OpacityInvalid
                               ? stateOpacityOrInvalid
                               : 1.0 * int(isChecked);

    const auto background = KColorUtils::mix(normalBackground, checkedBackground, stateOpacity);
    const auto foreground = KColorUtils::mix(normalForeground, checkedForeground, stateOpacity);

    // Frame color - hover priority

    const AnimationMode mode( _animations->widgetStateEngine().frameAnimationMode( widget ) );
    const qreal opacity( _animations->widgetStateEngine().frameOpacity( widget ) );

    QColor outline = _helper->frameOutlineColor(palette, mouseOver, hasFocus, opacity, mode, background);

    _helper->renderFrame( painter, rect.adjusted(0, 0, -0, -0), background, outline , isRadioButton);

    // Render mark

    static const auto inQuadEasingCurve = [](qreal v) { return v*v; };
    static const auto outQuadEasingCurve = [](qreal v) { return 1.0-inQuadEasingCurve(1.0-v); };

    painter->setRenderHint( QPainter::Antialiasing, true );

    if(isRadioButton) {
        RadioButtonState radioButtonState = state & State_On ? RadioOn : RadioOff;

        if( _animations->widgetStateEngine().isAnimated( widget, AnimationPressed ) ) {
           radioButtonState = radioButtonState == RadioOn ? RadioOffToOn : RadioOnToOff;
        }

        if(radioButtonState == RadioOff) {
            return;
        }

        /*
        painter->setBrush( Qt::NoBrush );
        painter->setPen( QPen(foreground, 4, Qt::SolidLine, Qt::RoundCap) );
        */

        const QPointF center = {rect.x() + rect.width() / 2.0, rect.y() + rect.height() / 2.0};
        const qreal fullRadius = 4.0;

        if(radioButtonState == RadioOn) {
            painter->setBrush( foreground );
            painter->setPen( Qt::NoPen );
            painter->drawEllipse(center, fullRadius, fullRadius);
        } else {
            qreal radius;
            QColor color = foreground;
            if(radioButtonState == RadioOffToOn) {
                radius = outQuadEasingCurve(stateOpacity) * fullRadius;
                color.setAlphaF(outQuadEasingCurve(stateOpacity));
                painter->setBrush(color);
                painter->setPen( Qt::NoPen );
            } else {
                qreal penWidth = fullRadius * inQuadEasingCurve(stateOpacity);
                radius = fullRadius / 2.0 + ((rect.width() - fullRadius) / 2 - 2) * outQuadEasingCurve(1.0-stateOpacity);
                color.setAlphaF(inQuadEasingCurve(stateOpacity));
                painter->setBrush(Qt::NoBrush);
                painter->setPen(QPen(color, penWidth));
            }
            painter->drawEllipse(center, radius, radius);
        }
    } else {
        const CheckBoxState checkBoxState = state & State_NoChange ? CheckPartial
                                          : state & State_On       ? CheckOn
                                                                   : CheckOff;
        bool startAnim = (checkBoxState != _animations->multiStateEngine().state(widget));
        _animations->multiStateEngine().updateState(widget, checkBoxState);

        const CheckBoxState previousCheckBoxState = _animations->multiStateEngine().previousState(widget);

        qreal progress = _animations->multiStateEngine().progress(widget);
        if(!_animations->multiStateEngine().isAnimated(widget)) {
            progress = 1.0;
        }

        const QPoint centerOffset = {rect.width()/2 + rect.x(), rect.height()/2 + rect.y()};

        DataMap<CheckBoxData>::Value dataPtr = _animations->multiStateEngine().data(widget);

        static const auto stateToData = [](CheckBoxState state) -> const CheckBoxRenderState * {
            switch(state) {
            case CheckUnknown:
            case CheckOff:      return &CheckBoxData::offState;
            case CheckOn:       return &CheckBoxData::onState;
            case CheckPartial:  return &CheckBoxData::partialState;
            };
            return nullptr;
        };

        const CheckBoxRenderState *state = nullptr;
        if (dataPtr.isNull()) {
            state = stateToData(checkBoxState);
            Q_CHECK_PTR(state);
        } else {
            CheckBoxData *data = dataPtr.data();
            state = &data->renderState;
            if(previousCheckBoxState == CheckBoxState::CheckUnknown) {
                // First rendering. Don't animate, it is initial state.
                data->renderState = *q_check_ptr(stateToData(checkBoxState));
            } else {
                if (startAnim) {
                    data->timeline->stop();
                    if (previousCheckBoxState == CheckOff       && checkBoxState == CheckOn)        { data->timeline->setTransitions(&CheckBoxData::offToOnTransition); }
                    if (previousCheckBoxState == CheckOn        && checkBoxState == CheckOff)       { data->timeline->setTransitions(&CheckBoxData::onToOffTransition); }
                    if (previousCheckBoxState == CheckOff       && checkBoxState == CheckPartial)   { data->timeline->setTransitions(&CheckBoxData::offToPartialTransition); }
                    if (previousCheckBoxState == CheckPartial   && checkBoxState == CheckOff)       { data->timeline->setTransitions(&CheckBoxData::partialToOffTransition); }
                    if (previousCheckBoxState == CheckPartial   && checkBoxState == CheckOn)        { data->timeline->setTransitions(&CheckBoxData::partialToOnTransition); }
                    if (previousCheckBoxState == CheckOn        && checkBoxState == CheckPartial)   { data->timeline->setTransitions(&CheckBoxData::onToPartialTransition); }
                    data->timeline->start();
                }
            }
        }
        renderCheckMark(painter, centerOffset, foreground, *state);
    }
}

}
