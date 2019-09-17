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

namespace Breeze
{

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

        static const auto makePropertyAnimation =
            [](const PropertyWrapperBase &property, const QVariant &start,
               const QVariant &end = QVariant(), unsigned duration = 0,
               const QEasingCurve &easing = QEasingCurve::Linear,
               AbstractVariantInterpolator *interpolator = nullptr,
               QPropertyAnimation **outPtr = nullptr)
        {
            QPropertyAnimation *p = new CustomPropertyAnimation(property, interpolator);
            p->setStartValue(start);
            p->setEndValue(end.isValid() ? end : start);
            p->setDuration(duration);
            p->setEasingCurve(easing);
            if (outPtr != nullptr) {
                *outPtr = p;
            }
            return p;
        };
        static const auto makeParallelAnimationGroup = [](std::initializer_list<QAbstractAnimation *> animations)
        {
            auto *group = new QParallelAnimationGroup();
            for(auto *animation: animations) {
                group->addAnimation(animation);
            }
            return group;
        };
        static const auto makeSequentialAnimationGroup = [](std::initializer_list<QAbstractAnimation *> animations)
        {
            auto *group = new QSequentialAnimationGroup();
            for(auto *animation: animations) {
                group->addAnimation(animation);
            }
            return group;
        };
        static const auto connectToWidget = [](QAbstractAnimation *animation, const QWidget *widget)
        {
            auto *propertyAnimation = qobject_cast<QPropertyAnimation *>(animation);
            if(propertyAnimation != nullptr) {
                connect(propertyAnimation, &QPropertyAnimation::valueChanged,
                        const_cast<QWidget *>(widget), QOverload<>::of(&QWidget::update));
            } else {
                const auto propertyAnimations = animation->findChildren<QPropertyAnimation *>();
                for(auto *propertyAnimation: propertyAnimations) {
                    connect(propertyAnimation, &QPropertyAnimation::valueChanged,
                            const_cast<QWidget *>(widget), QOverload<>::of(&QWidget::update));
                }
            }
        };

        static const QPointF invalidPointF(qQNaN(), qQNaN());
        // FIXME: use duratoin from the new engine after the code is moved
        static const unsigned totalDuration = 1000;//_animations->multiStateEngine().duration();

        DataMap<MultiStateData>::Value dataPtr = _animations->multiStateEngine().data(widget);
        if (dataPtr.isNull()) {
            // TODO: draw static mark when animation not supported
            return;
        }
        MultiStateData *data = dataPtr.data();

        static const auto comparePointF = [](const QPointF &a, const QPointF &b) {
            return qAbs(b.x() - a.x()) < 0.1 && qAbs(b.y() - a.y()) < 0.1;
        };

        ////////////////////////////////////////////////////////////////////////////////

        std::array<PropertyWrapper<QPointF>, 3> pPos = {{
            {data, "p0.pos"},
            {data, "p1.pos"},
            {data, "p2.pos"}
        }};
        std::array<PropertyWrapper<qreal>, 3> pRadius = {{
            {data, "p0.radius"},
            {data, "p1.radius"},
            {data, "p2.radius"}
        }};
        std::array<PropertyWrapper<QPointF>, 3> lPPos = {{
            {data, "lp0.pos"},
            {data, "lp1.pos"},
            {data, "lp2.pos"}
        }};
        PropertyWrapper<QPointF> checkPos = {data, "cp.pos"};

        static const std::array<QPointF, 3> refLPPos    = {{{-4, 0}, {-1, 3}, {4, -2}}};
        static const std::array<QPointF, 3> refPPos     = {{{-4, 0}, {0, 0}, {4, 0}}};

        static const std::array<float, 3> startRadius   = {{0, 0, 0}};
        static const std::array<float, 3> endRadius     = {{1, 1, 1}};

        enum AnimationId {
            OffToOn,
            OnToOff,

            OffToPartial,
            PartialToOff,

            PartialToOn,
            OnToPartial,
        };

        if (!data->anims.contains(OffToOn)) { // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
            auto *onAnimation = makeSequentialAnimationGroup({
                makePropertyAnimation(lPPos[0], refLPPos[0]),
                makePropertyAnimation(lPPos[2], refLPPos[0]),
                makePropertyAnimation(lPPos[1], refLPPos[0], refLPPos[1], 0.4 * totalDuration, QEasingCurve::InOutCubic),
                new QPauseAnimation(0.1 * totalDuration),
                makePropertyAnimation(lPPos[2], refLPPos[1], refLPPos[2], 0.5 * totalDuration, QEasingCurve::InOutCubic)
            });
            connectToWidget(onAnimation, widget);
            onAnimation->setParent(data);
            data->anims[OffToOn] = onAnimation;

            auto *offAnimation = makeSequentialAnimationGroup({
                makePropertyAnimation(lPPos[0], refLPPos[0], refLPPos[1], 0.5 * totalDuration, QEasingCurve::InOutCubic),
                new QPauseAnimation(0.1 * totalDuration),
                makeParallelAnimationGroup({
                    makePropertyAnimation(lPPos[0], refLPPos[1], refLPPos[2], 0.4 * totalDuration, QEasingCurve::InOutCubic),
                    makePropertyAnimation(lPPos[1], refLPPos[1], refLPPos[2], 0.4 * totalDuration, QEasingCurve::InOutCubic),
                })
            });
            connectToWidget(offAnimation, widget);
            offAnimation->setParent(data);
            data->anims[OnToOff] = offAnimation;
        }
        if (!data->anims.contains(OffToPartial)) { // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
            auto *onAnimation = makeSequentialAnimationGroup({
                makePropertyAnimation(pPos[0], refPPos[0]),
                makePropertyAnimation(pPos[1], refPPos[1]),
                makePropertyAnimation(pPos[2], refPPos[2]),
                makeParallelAnimationGroup({
                    makePropertyAnimation(pRadius[0], startRadius[0], endRadius[0], 0.6 * totalDuration, QEasingCurve::OutCubic),
                    makeSequentialAnimationGroup({
                        new QPauseAnimation(0.2 * totalDuration),
                        makePropertyAnimation(pRadius[1], startRadius[1], endRadius[1], 0.6 * totalDuration, QEasingCurve::OutCubic),
                    }),
                    makeSequentialAnimationGroup({
                        new QPauseAnimation(0.4 * totalDuration),
                        makePropertyAnimation(pRadius[2], startRadius[2], endRadius[2], 0.6 * totalDuration, QEasingCurve::OutCubic),
                    }),
                }),
            });
            connectToWidget(onAnimation, widget);
            onAnimation->setParent(data);
            data->anims[OffToPartial] = onAnimation;

            auto *offAnimation = makeParallelAnimationGroup({
                makePropertyAnimation(pRadius[0], endRadius[0], startRadius[0], 0.6 * totalDuration, QEasingCurve::OutCubic),
                makeSequentialAnimationGroup({
                    new QPauseAnimation(0.2 * totalDuration),
                    makePropertyAnimation(pRadius[1], endRadius[1], startRadius[1], 0.6 * totalDuration, QEasingCurve::OutCubic),
                }),
                makeSequentialAnimationGroup({
                    new QPauseAnimation(0.4 * totalDuration),
                    makePropertyAnimation(pRadius[2], endRadius[2], startRadius[2], 0.6 * totalDuration, QEasingCurve::OutCubic),
                }),
            });
            connectToWidget(offAnimation, widget);
            offAnimation->setParent(data);
            data->anims[PartialToOff] = offAnimation;
        }
        if (!data->anims.contains(PartialToOn)) { // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
            auto *onAnimation = makeSequentialAnimationGroup({
                makePropertyAnimation(lPPos[0], refLPPos[0]),
                makePropertyAnimation(lPPos[2], refLPPos[0]),
                makePropertyAnimation(pPos[0], refPPos[0]),
                makePropertyAnimation(pPos[1], refPPos[1]),
                makePropertyAnimation(pPos[2], refPPos[2]),

                makeParallelAnimationGroup({
                    makePropertyAnimation(lPPos[1], refLPPos[0], refLPPos[1], 0.4 * totalDuration, QEasingCurve::InOutCubic),
                    makePropertyAnimation(pPos[1],  refPPos[1],  refLPPos[1], 0.5 * totalDuration, QEasingCurve::InOutCubic),
                    makePropertyAnimation(pPos[2],  refPPos[2],  refLPPos[1], 0.5 * totalDuration, QEasingCurve::InOutCubic),
                    makePropertyAnimation(pRadius[1], endRadius[1], endRadius[1] * sqrt(2), 0.5 * totalDuration, QEasingCurve::InOutCubic),
                    makePropertyAnimation(pRadius[2], endRadius[2], endRadius[2] * sqrt(2), 0.5 * totalDuration, QEasingCurve::InOutCubic),
                }),
                makeParallelAnimationGroup({
                    makePropertyAnimation(lPPos[2], refLPPos[1], refLPPos[2], 0.5 * totalDuration, QEasingCurve::InOutCubic),
                    makePropertyAnimation(pRadius[1], endRadius[1] * sqrt(2), endRadius[1], 0.5 * totalDuration, QEasingCurve::InOutCubic),
                    makePropertyAnimation(pRadius[2], endRadius[2] * sqrt(2), endRadius[2], 0.5 * totalDuration, QEasingCurve::InOutCubic),
                    makePropertyAnimation(pPos[2], refLPPos[1],  refLPPos[2], 0.5 * totalDuration, QEasingCurve::InOutCubic),
                }),

                makePropertyAnimation(pRadius[0], 0),
                makePropertyAnimation(pRadius[1], 0),
                makePropertyAnimation(pRadius[2], 0),
            });
            connectToWidget(onAnimation, widget);
            onAnimation->setParent(data);
            data->anims[PartialToOn] = onAnimation;

            auto *offAnimation = makeSequentialAnimationGroup({
                makePropertyAnimation(pRadius[0], 0),
                makePropertyAnimation(pRadius[2], 0),
                makeParallelAnimationGroup({
                    makePropertyAnimation(lPPos[0], refLPPos[0], refLPPos[1], 0.4 * totalDuration, QEasingCurve::InOutCubic),
                    makePropertyAnimation(lPPos[2], refLPPos[2], refLPPos[1], 0.4 * totalDuration, QEasingCurve::InOutCubic),

                    makePropertyAnimation(checkPos, QPointF{0,0}, refPPos[1] - refLPPos[1], 0.4 * totalDuration, QEasingCurve::InOutCubic),
                    makePropertyAnimation(pPos[1], refLPPos[1], refPPos[1], 0.4 * totalDuration, QEasingCurve::InOutCubic),
                    makePropertyAnimation(pRadius[1], startRadius[1], endRadius[1] * sqrt(3), 0.4 * totalDuration, QEasingCurve::InOutCubic),
                }),
                new QPauseAnimation(0.1 * totalDuration),
                makeParallelAnimationGroup({
                    makePropertyAnimation(pRadius[0], endRadius[0] * sqrt(3), endRadius[0], 0.5 * totalDuration, QEasingCurve::InOutCubic),
                    makePropertyAnimation(pRadius[1], endRadius[1] * sqrt(3), endRadius[0], 0.5 * totalDuration, QEasingCurve::InOutCubic),
                    makePropertyAnimation(pRadius[2], endRadius[2] * sqrt(3), endRadius[0], 0.5 * totalDuration, QEasingCurve::InOutCubic),
                    makePropertyAnimation(pPos[0], refPPos[1], refPPos[0], 0.5 * totalDuration, QEasingCurve::InOutCubic),
                    makePropertyAnimation(pPos[2], refPPos[1], refPPos[2], 0.5 * totalDuration, QEasingCurve::InOutCubic),
                }),
                makePropertyAnimation(checkPos, QPointF{0,0}),
            });
            connectToWidget(offAnimation, widget);
            offAnimation->setParent(data);
            data->anims[OnToPartial] = offAnimation;
        }

        if (startAnim) {
            if (previousCheckBoxState == CheckOff       && checkBoxState == CheckOn)        { data->anims[OffToOn]->start(); }
            if (previousCheckBoxState == CheckOn        && checkBoxState == CheckOff)       { data->anims[OnToOff]->start(); }
            if (previousCheckBoxState == CheckOff       && checkBoxState == CheckPartial)   { data->anims[OffToPartial]->start(); }
            if (previousCheckBoxState == CheckPartial   && checkBoxState == CheckOff)       { data->anims[PartialToOff]->start(); }
            if (previousCheckBoxState == CheckPartial   && checkBoxState == CheckOn)        { data->anims[PartialToOn]->start(); }
            if (previousCheckBoxState == CheckOn        && checkBoxState == CheckPartial)   { data->anims[OnToPartial]->start(); }
        }

        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(foreground, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        QPainterPath pp;
        pp.moveTo(lPPos[0] + centerOffset);
        if (!comparePointF(lPPos[1], lPPos[0])) { pp.lineTo(lPPos[1] + centerOffset); }
        if (!comparePointF(lPPos[2], lPPos[0])) { pp.lineTo(lPPos[2] + centerOffset); }
        pp.translate(checkPos);
        painter->drawPath(pp);

        painter->setPen(Qt::NoPen);
        for (int i = 0; i < 3; ++i) {
            painter->setBrush(foreground);
            painter->drawEllipse(pPos[i] + centerOffset, pRadius[i], pRadius[i]);
        }
    }
}

}