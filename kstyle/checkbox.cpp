#include "breezestyle.h"

#include "breeze.h"
#include "breezeanimations.h"
#include "breezemultistatedata.h"

#include <KColorUtils>
#include <QPainter>

#include <QEasingCurve>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPauseAnimation>

#include <cmath>

namespace Breeze
{

    using Id = CheckMarkRenderer::DataId;

    namespace {
    const QVector<QVariant> offStateData {
        /* Position          */ QPointF(0, 0),
        /* LinePointPosition */ invalidPointF, invalidPointF, invalidPointF,
        /* PointPosition     */ invalidPointF, invalidPointF, invalidPointF,
        /* PointRadius       */ 0.0f, 0.0f, 0.0f
    };
    const QVector<QVariant> onStateData = {
        /* Position          */ QPointF(-1, 3),
        /* LinePointPosition */ QPointF(-3, -3), QPointF(0, 0), QPointF(5, -5),
        /* PointPosition     */ invalidPointF, invalidPointF, invalidPointF,
        /* PointRadius       */ 0.0f, 0.0f, 0.0f,
    };
    const QVector<QVariant> partialStateData = {
        /* Position          */ QPointF(0, 0),
        /* LinePointPosition */ invalidPointF, invalidPointF, invalidPointF,
        /* PointPosition     */ QPointF(-4, 0), QPointF( 0, 0), QPointF(4,  0),
        /* PointRadius       */ 1.0f, 1.0f, 1.0f,
    };

    const TimelineAnimation::EntryList offToOnTransition {
        {0.0f,       &offStateData},
        {0.0f,       Id::Position,            onStateData[Id::Position]},
        {0.0f,       Id::LinePointPosition_0, onStateData[Id::LinePointPosition_0]},
        {0.0f, 0.4f, Id::LinePointPosition_1, onStateData[Id::LinePointPosition_0], onStateData[Id::LinePointPosition_1], QEasingCurve::InOutCubic},
        {0.5f, 0.5f, Id::LinePointPosition_2, onStateData[Id::LinePointPosition_1], onStateData[Id::LinePointPosition_2], QEasingCurve::InOutCubic},
        {1.0f,       &onStateData},
    };
    const TimelineAnimation::EntryList onToOffTransition {
        {0.0f,       &onStateData},
        {0.0f, 0.5f, Id::LinePointPosition_0, onStateData[Id::LinePointPosition_0], onStateData[Id::LinePointPosition_1], QEasingCurve::InOutCubic},
        {0.6f, 0.4f, Id::LinePointPosition_0, onStateData[Id::LinePointPosition_1], onStateData[Id::LinePointPosition_2], QEasingCurve::InOutCubic},
        {0.6f, 0.4f, Id::LinePointPosition_1, onStateData[Id::LinePointPosition_1], onStateData[Id::LinePointPosition_2], QEasingCurve::InOutCubic},
        {1.0f,       &offStateData},
    };

    const TimelineAnimation::EntryList offToPartialTransition {
        {0.0f,       &offStateData},
        {0.0f,       Id::PointPosition_0, partialStateData[Id::PointPosition_0]},
        {0.0f,       Id::PointPosition_1, partialStateData[Id::PointPosition_1]},
        {0.0f,       Id::PointPosition_2, partialStateData[Id::PointPosition_2]},
        {0.0f, 0.6f, Id::PointRadius_0,   QVariant(), partialStateData[Id::PointRadius_0], QEasingCurve::OutCubic},
        {0.2f, 0.6f, Id::PointRadius_1,   QVariant(), partialStateData[Id::PointRadius_1], QEasingCurve::OutCubic},
        {0.4f, 0.6f, Id::PointRadius_2,   QVariant(), partialStateData[Id::PointRadius_2], QEasingCurve::OutCubic},
        {1.0f,       &partialStateData},
    };
    const TimelineAnimation::EntryList partialToOffTransition {
        {0.0f,       &partialStateData},
        {0.0f, 0.6f, Id::PointRadius_0,   partialStateData[Id::PointRadius_0], offStateData[Id::PointRadius_0], QEasingCurve::InCubic},
        {0.2f, 0.6f, Id::PointRadius_1,   partialStateData[Id::PointRadius_1], offStateData[Id::PointRadius_1], QEasingCurve::InCubic},
        {0.4f, 0.6f, Id::PointRadius_2,   partialStateData[Id::PointRadius_2], offStateData[Id::PointRadius_2], QEasingCurve::InCubic},
        {1.0f,       &offStateData},
    };

    const float partialPointRadiusSqrt2 = partialStateData[Id::PointRadius_0].toFloat() * sqrtf(2);
    const float partialPointRadiusSqrt3 = partialStateData[Id::PointRadius_0].toFloat() * sqrtf(3);
    const QPointF onAbsLinePointPosition_2 = onStateData[Id::LinePointPosition_2].toPointF() + onStateData[Id::Position].toPointF();

    const TimelineAnimation::EntryList partialToOnTransition {
        {0.0f,       &partialStateData},
        {0.0f,       Id::Position,               onStateData[Id::Position]},
        {0.0f,       Id::LinePointPosition_0,    onStateData[Id::LinePointPosition_0]},

        {0.0f, 0.4f, Id::LinePointPosition_1,    onStateData[Id::LinePointPosition_0],  onStateData[Id::LinePointPosition_1],   QEasingCurve::InOutCubic},
        {0.0f, 0.4f, Id::PointRadius_0,          QVariant(),                            onStateData[Id::PointRadius_0],         QEasingCurve::InOutCubic},
        {0.0f, 0.5f, Id::PointPosition_1,        QVariant(),                            onStateData[Id::Position],              QEasingCurve::InOutCubic},
        {0.0f, 0.5f, Id::PointPosition_2,        QVariant(),                            onStateData[Id::Position],              QEasingCurve::InOutCubic},
        {0.0f, 0.5f, Id::PointRadius_1,          QVariant(),                            partialPointRadiusSqrt2,                QEasingCurve::InOutCubic},
        {0.0f, 0.5f, Id::PointRadius_2,          QVariant(),                            partialPointRadiusSqrt2,                QEasingCurve::InOutCubic},

        {0.5f, 0.5f, Id::LinePointPosition_2,    onStateData[Id::LinePointPosition_1],  onStateData[Id::LinePointPosition_2],   QEasingCurve::InOutCubic},
        {0.5f, 0.5f, Id::PointPosition_2,        QVariant(),                            onAbsLinePointPosition_2,               QEasingCurve::InOutCubic},
        {0.5f, 0.5f, Id::PointRadius_1,          QVariant(),                            onStateData[Id::PointRadius_1],         QEasingCurve::InOutCubic},
        {0.5f, 0.5f, Id::PointRadius_2,          QVariant(),                            onStateData[Id::PointRadius_2],         QEasingCurve::InOutCubic},
        {1.0f,       &onStateData},
    };
    const TimelineAnimation::EntryList onToPartialTransition {
        {0.0f,       &onStateData},
        {0.0f, 0.4f, Id::Position,               QVariant(),                            partialStateData[Id::Position],         QEasingCurve::InOutCubic},
        {0.0f, 0.4f, Id::LinePointPosition_0,    QVariant(),                            onStateData[Id::LinePointPosition_1],   QEasingCurve::InOutCubic},
        {0.0f, 0.4f, Id::LinePointPosition_2,    QVariant(),                            onStateData[Id::LinePointPosition_1],   QEasingCurve::InOutCubic},
        {0.0f, 0.4f, Id::PointPosition_1,        onStateData[Id::Position],             partialStateData[Id::PointPosition_1],  QEasingCurve::InOutCubic},
        {0.0f, 0.4f, Id::PointRadius_1,          QVariant(),                            partialPointRadiusSqrt3,                QEasingCurve::InOutCubic},

        {0.5f, 0.5f, Id::PointPosition_0,        partialStateData[Id::PointPosition_1], partialStateData[Id::PointPosition_0],  QEasingCurve::InOutCubic},
        {0.5f, 0.5f, Id::PointPosition_2,        partialStateData[Id::PointPosition_1], partialStateData[Id::PointPosition_2],  QEasingCurve::InOutCubic},
        {0.5f, 0.5f, Id::PointRadius_0,          partialPointRadiusSqrt3,               partialStateData[Id::PointRadius_0],    QEasingCurve::InOutCubic},
        {0.5f, 0.5f, Id::PointRadius_1,          partialPointRadiusSqrt3,               partialStateData[Id::PointRadius_1],    QEasingCurve::InOutCubic},
        {0.5f, 0.5f, Id::PointRadius_2,          partialPointRadiusSqrt3,               partialStateData[Id::PointRadius_2],    QEasingCurve::InOutCubic},
        {1.0f,       &partialStateData},
    };
    }

static void renderCheckMark(QPainter *painter, const QPoint &position, const QColor &color,
                            const QVector<QVariant> &vars)
{
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    QPainterPath pp;

    const QPointF relPosition = vars[Id::Position].toPointF();
    const QPointF linePointPos[] = {
        vars[Id::LinePointPosition_0].toPointF(),
        vars[Id::LinePointPosition_1].toPointF(),
        vars[Id::LinePointPosition_2].toPointF(),
    };
    const QPointF pointPos[] = {
        vars[Id::PointPosition_0].toPointF(),
        vars[Id::PointPosition_1].toPointF(),
        vars[Id::PointPosition_2].toPointF(),
    };
    const float pointRadius[] = {
        vars[Id::PointRadius_0].toFloat(),
        vars[Id::PointRadius_1].toFloat(),
        vars[Id::PointRadius_2].toFloat(),
    };

    int i = 0;
    for(; i < 3; ++i) {
        if(!isInvalidPointF(linePointPos[i])) {
            pp.moveTo(linePointPos[i]);
            break;
        }
    }
    for(; i < 3; ++i) {
        if(!isInvalidPointF(linePointPos[i])) {
            pp.lineTo(linePointPos[i]);
        }
    }
    pp.translate(relPosition + position);
    painter->drawPath(pp);

    painter->setPen(Qt::NoPen);
    for (int i = 0; i < 3; ++i) {
        if (isInvalidPointF(pointPos[i]) || qFuzzyIsNull(pointRadius[i])) {
            continue;
        }
        painter->setBrush(color);
        painter->drawEllipse(pointPos[i] + position, pointRadius[i], pointRadius[i]);
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

    // FIXME (mglb): move to some animation engine
    QVariant lastStateVariant = widget ? widget->property("_breeze_lastState") : QVariant();
    int lastState;
    if(lastStateVariant.isValid()) {
        lastState = lastStateVariant.toInt();
    } else {
        lastState = option->state & (State_On | State_NoChange | State_Off);
    }
    if(widget) {
        auto *widgetRw = const_cast<QWidget*>(widget);
        widgetRw->setProperty("_breeze_lastState", QVariant(option->state & (State_On | State_NoChange | State_Off)));
    }

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
                                                        : CheckBoxState::CheckOff;

        qreal progress = _animations->multiStateEngine().opacity(widget);
        if(!_animations->multiStateEngine().isAnimated(widget)) {
            progress = 1.0;
        }

        const QPoint centerOffset = {rect.width()/2 + rect.x(), rect.height()/2 + rect.y()};

        DataMap<MultiStateData>::Value dataPtr = _animations->multiStateEngine().data(widget);

        static const auto stateToData = [](CheckBoxState state) -> const QVector<QVariant> * {
            switch(state) {
            case CheckOff:      return &offStateData;
            case CheckOn:       return &onStateData;
            case CheckPartial:  return &partialStateData;
            };
            return nullptr;
        };

        const QVector<QVariant> *vars = nullptr;
        if (dataPtr.isNull()) {
            vars = stateToData(checkBoxState);
            Q_CHECK_PTR(vars);
        } else {
            MultiStateData *data = dataPtr.data();
            vars = &data->variables;
            if(data->variables.isEmpty()) {
                // First rendering. Don't animate, it is initial state.
                data->variables = *q_check_ptr(stateToData(checkBoxState));
            } else {
                if (startAnim) {
                    data->timeline->stop();
                    if (previousCheckBoxState == CheckOff       && checkBoxState == CheckOn)        { data->timeline->setTransitions(&offToOnTransition); }
                    if (previousCheckBoxState == CheckOn        && checkBoxState == CheckOff)       { data->timeline->setTransitions(&onToOffTransition); }
                    if (previousCheckBoxState == CheckOff       && checkBoxState == CheckPartial)   { data->timeline->setTransitions(&offToPartialTransition); }
                    if (previousCheckBoxState == CheckPartial   && checkBoxState == CheckOff)       { data->timeline->setTransitions(&partialToOffTransition); }
                    if (previousCheckBoxState == CheckPartial   && checkBoxState == CheckOn)        { data->timeline->setTransitions(&partialToOnTransition); }
                    if (previousCheckBoxState == CheckOn        && checkBoxState == CheckPartial)   { data->timeline->setTransitions(&onToPartialTransition); }
                    data->timeline->start();
                }
            }
        }
        renderCheckMark(painter, centerOffset, foreground, *vars);
    }
}

}
