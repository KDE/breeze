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

//______________________________________________________________________________
void Helper::renderCheckBoxBackground( QPainter* painter, const QRect& rect, const QColor& color, bool sunken ) const
{

    // setup painter
    painter->setRenderHint( QPainter::Antialiasing, true );

    // copy rect and radius
    QRectF frameRect( rect );
    frameRect.adjust( 3, 3, -3, -3 );

    if( sunken ) frameRect.translate(1, 1);

    painter->setPen( Qt::NoPen );
    painter->setBrush( color );
    painter->drawRect( frameRect );

}

//______________________________________________________________________________
void Helper::renderCheckBox(
    QPainter* painter, const QRect& rect,
    const QColor& color, const QColor& shadow,
    bool sunken, CheckBoxState state, qreal animation ) const
{
    // FIXME (mglb): use for drawing radio button control

    // setup painter
    painter->setRenderHint( QPainter::Antialiasing, true );

    // copy rect and radius
    QRectF frameRect( rect );
    frameRect.adjust( 2, 2, -2, -2 );
    qreal radius( frameRadius() );

    // shadow
    if( sunken )
    {

        frameRect.translate(1, 1);

    } else {

        painter->setPen( QPen( shadow, 1 ) );
        painter->setBrush( Qt::NoBrush );

        const qreal shadowRadius( radius + 0.5 );
        painter->drawRoundedRect( shadowRect( frameRect ).adjusted( -0.5, -0.5, 0.5, 0.5 ), shadowRadius, shadowRadius );

    }

    // content
    {

        painter->setPen( QPen( color, 1 ) );
        painter->setBrush( Qt::NoBrush );

        radius = qMax( radius-1, qreal( 0.0 ) );
        const QRectF contentRect( frameRect.adjusted( 0.5, 0.5, -0.5, -0.5 ) );
        painter->drawRoundedRect( contentRect, radius, radius );

    }

    // mark
    if( state == CheckOn )
    {

        painter->setBrush( color );
        painter->setPen( Qt::NoPen );

        const QRectF markerRect( frameRect.adjusted( 3, 3, -3, -3 ) );
        painter->drawRect( markerRect );

    } else if( state == CheckPartial ) {

        QPen pen( color, 2 );
        pen.setJoinStyle( Qt::MiterJoin );
        painter->setPen( pen );

        const QRectF markerRect( frameRect.adjusted( 4, 4, -4, -4 ) );
        painter->drawRect( markerRect );

        painter->setPen( Qt::NoPen );
        painter->setBrush( color );
        painter->setRenderHint( QPainter::Antialiasing, false );

        QPainterPath path;
        path.moveTo( markerRect.topLeft() );
        path.lineTo( markerRect.right() - 1, markerRect.top() );
        path.lineTo( markerRect.left(), markerRect.bottom()-1 );
        painter->drawPath( path );

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
    const bool mouseOver( enabled && ( state & State_MouseOver ) );
    const bool hasFocus( enabled && ( state & State_HasFocus ) );

    // focus takes precedence over mouse over
    _animations->widgetStateEngine().updateState( widget, AnimationFocus, hasFocus );
    _animations->widgetStateEngine().updateState( widget, AnimationHover, mouseOver );

    // retrieve animation mode and opacity
    const AnimationMode mode( _animations->widgetStateEngine().frameAnimationMode( widget ) );
    const qreal opacity( _animations->widgetStateEngine().frameOpacity( widget ) );

    // Render background and frame

    // Foreground and background color

    const auto &normalBackground = palette.color(QPalette::Base);
    const auto &normalForeground = palette.color(QPalette::Text);
    const auto &focusBackground  = palette.color(QPalette::Highlight);
    const auto &focusForeground  = palette.color(QPalette::HighlightedText);

    const auto focusOpacityOrInvalid = _animations->widgetStateEngine().opacity(widget, AnimationFocus);
    const auto focusOpacity = focusOpacityOrInvalid != AnimationData::OpacityInvalid
                              ? focusOpacityOrInvalid
                              : 1.0 * int(hasFocus);

    const auto background = KColorUtils::mix(normalBackground, focusBackground, focusOpacity);
    const auto foreground = hasFocus ? focusForeground : normalForeground;

    // Frame color - hover priority

    QColor outline = _helper->frameOutlineColor(palette, mouseOver, hasFocus, opacity, mode);

    _helper->renderFrame( painter, rect.adjusted(0, 0, -0, -0), background, outline , isRadioButton);

    // Render mark

    static const auto inQuadEasingCurve = [](qreal v) { return v*v; };
    static const auto outQuadEasingCurve = [](qreal v) { return 1.0-inQuadEasingCurve(1.0-v); };

    painter->setRenderHint( QPainter::Antialiasing, true );

    if(isRadioButton) {
        RadioButtonState radioButtonState = state & State_On ? RadioOn : RadioOff;

        _animations->widgetStateEngine().updateState( widget, AnimationPressed, radioButtonState != RadioOff );
        if( _animations->widgetStateEngine().isAnimated( widget, AnimationPressed ) ) {
           radioButtonState = radioButtonState == RadioOn ? RadioOffToOn : RadioOnToOff;
        }
        const qreal animation = _animations->widgetStateEngine().opacity( widget, AnimationPressed );

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
                radius = outQuadEasingCurve(animation) * fullRadius;
                color.setAlphaF(outQuadEasingCurve(animation));
                painter->setBrush(color);
                painter->setPen( Qt::NoPen );
            } else {
                qreal penWidth = fullRadius * inQuadEasingCurve(animation);
                radius = fullRadius / 2.0 + ((rect.width() - fullRadius) / 2 - 2) * outQuadEasingCurve(1.0-animation);
                color.setAlphaF(inQuadEasingCurve(animation));
                painter->setBrush(Qt::NoBrush);
                painter->setPen(QPen(color, penWidth));
            }
            painter->drawEllipse(center, radius, radius);
        }
    } else {
        const CheckBoxState checkBoxState = state & State_NoChange ? CheckPartial
                                          : state & State_On       ? CheckOn
                                                                   : CheckOff;
        bool startAnim = (checkBoxState != _animations->multiStateEngine().state(widget).value<CheckBoxState>());
        _animations->multiStateEngine().updateState(widget, checkBoxState);

        const QVariant lastStateVariant = _animations->multiStateEngine().previousState(widget);
        const CheckBoxState previousCheckBoxState = lastStateVariant.isValid()
                                                        ? lastStateVariant.value<CheckBoxState>()
                                                        : CheckBoxState::CheckUnknown;

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
