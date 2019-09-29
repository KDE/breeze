#include "breezestyle.h"

#include "breeze.h"
#include "breezeanimations.h"

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

        static const auto rotate = [](const QPointF &p, qreal a, const QPointF &c) {
            const qreal x = cos(a) * (p.x() - c.x()) - sin(a) * (p.y() - c.y()) + c.x();
            const qreal y = sin(a) * (p.x() - c.x()) + cos(a) * (p.y() - c.y()) + c.y();
            return QPointF(x, y);
        };

        static const auto makePropertyAnimation =
            [](QObject *data, const char *property, const QVariant &start,
               const QVariant &end = QVariant(), unsigned duration = 0,
               const QEasingCurve &easing = QEasingCurve::Linear)
        {
            QPropertyAnimation *p = new QPropertyAnimation(data, property);
            p->setStartValue(start);
            p->setEndValue(end.isValid() ? end : start);
            p->setDuration(duration);
            p->setEasingCurve(easing);
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
        static const unsigned totalDuration = _animations->multiStateEngine().duration();

        DataMap<MultiStateData>::Value dataPtr = _animations->multiStateEngine().data(widget);
        if (dataPtr.isNull()) {
            // TODO: draw static mark when animation not supported
            return;
        }
        MultiStateData *data = dataPtr.data();

        static const auto comparePointF = [](const QPointF &a, const QPointF &b) {
            return qAbs(b.x() - a.x()) < 0.1 && qAbs(b.y() - a.y()) < 0.1;
        };

        // mark
        if(checkBoxState == CheckOn || previousCheckBoxState == CheckOn) {
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QPen(foreground, 2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));

            static const char PROPERTIES[][3] = {"p0", "p1", "p2"};

            if (!data->anims.contains(0)) {
                static const QPointF rotationOrigin = {-1, 3};
                static const qreal rotationAngle = 2 * M_PI / 8; // 45°
                // TODO: draw as polygon instead of line
                static const QPointF BASE_POINTS[] = {
                    rotate({-6,  3}, rotationAngle, rotationOrigin),
                    rotate({-1,  3}, rotationAngle, rotationOrigin),
                    rotate({-1, -6}, rotationAngle, rotationOrigin),
                };

                data->setProperty(PROPERTIES[0], QPointF());
                data->setProperty(PROPERTIES[1], QPointF());
                data->setProperty(PROPERTIES[2], QPointF());
                auto *onAnimation = makeSequentialAnimationGroup({
                    makeParallelAnimationGroup({
                        makePropertyAnimation(data, PROPERTIES[0], BASE_POINTS[0], BASE_POINTS[0], 0),
                        makePropertyAnimation(data, PROPERTIES[1], BASE_POINTS[0], BASE_POINTS[1], 0.5 * totalDuration, QEasingCurve::OutQuad),
                        makePropertyAnimation(data, PROPERTIES[2], BASE_POINTS[0], BASE_POINTS[1], 0.5 * totalDuration, QEasingCurve::OutQuad),
                    }),
                    new QPauseAnimation(0.2 * totalDuration),
                    makePropertyAnimation(data, PROPERTIES[2], BASE_POINTS[1], BASE_POINTS[2], 0.3 * totalDuration),
                });
                connectToWidget(onAnimation, widget);
                onAnimation->setParent(data);
                data->anims[0] = onAnimation;

                auto *offAnimation = makeSequentialAnimationGroup({
                    makePropertyAnimation(data, PROPERTIES[0], BASE_POINTS[0], BASE_POINTS[1], 0.5 * totalDuration, QEasingCurve::OutQuad),
                    new QPauseAnimation(0.2 * totalDuration),
                    makeParallelAnimationGroup({
                        makePropertyAnimation(data, PROPERTIES[0], BASE_POINTS[1], BASE_POINTS[2], 0.3 * totalDuration),
                        makePropertyAnimation(data, PROPERTIES[1], BASE_POINTS[1], BASE_POINTS[2], 0.3 * totalDuration),
                    }),
                });
                connectToWidget(offAnimation, widget);
                offAnimation->setParent(data);
                data->anims[1] = offAnimation;
            }
            if (checkBoxState == CheckOn && startAnim) {
                data->anims[0]->start();
            }
            if (previousCheckBoxState == CheckOn && startAnim) {
                data->anims[1]->start();
            }

            QPainterPath pp;

            const QPointF p[] = {
                data->property(PROPERTIES[0]).toPointF(),
                data->property(PROPERTIES[1]).toPointF(),
                data->property(PROPERTIES[2]).toPointF(),
            };

            pp.moveTo(p[0]);
            if (!comparePointF(p[0], p[1])) {
                pp.lineTo(p[1]);
            }
            if (!comparePointF(p[1], p[2])) {
                pp.lineTo(p[2]);
            }
            pp.translate(centerOffset);

            painter->drawPath(pp);
        }
        if(checkBoxState == CheckPartial || previousCheckBoxState == CheckPartial) {
            static const QPointF pointsCenter = {-1, 3};
            static const qreal pointsAngle = 2 * M_PI / 8; // 45°
            static const QPointF points[] = {
                rotate({-5, 3}, pointsAngle, pointsCenter),
                rotate({-1, 3}, pointsAngle, pointsCenter),
                rotate({-1, -1}, pointsAngle, pointsCenter),
                rotate({-1, -5}, pointsAngle, pointsCenter),
            };

            static const char PROP_NAMES[][3] = {"r0", "r1", "r2", "r3", "a0", "a1", "a2", "a3"};
            static const QVariant PROP_INIT_VALUES_ON[] = {0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0};
            static const QVariant PROP_INIT_VALUES_OFF[] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

            if (!data->anims.contains(2)) {
                auto *onAnimation = makeParallelAnimationGroup({
                    makePropertyAnimation(data, PROP_NAMES[0], 0.0, 1.0, 0.4 * totalDuration, QEasingCurve::OutBack),
                    makeSequentialAnimationGroup({
                        new QPauseAnimation(0.2 * totalDuration),
                        makePropertyAnimation(data, PROP_NAMES[1], 0.0, 1.0, 0.4 * totalDuration, QEasingCurve::OutBack),
                    }),
                    makeSequentialAnimationGroup({
                        new QPauseAnimation(0.4 * totalDuration),
                        makePropertyAnimation(data, PROP_NAMES[2], 0.0, 1.0, 0.4 * totalDuration, QEasingCurve::OutBack),
                    }),
                    makeSequentialAnimationGroup({
                        new QPauseAnimation(0.6 * totalDuration),
                        makePropertyAnimation(data, PROP_NAMES[3], 0.0, 1.0, 0.4 * totalDuration, QEasingCurve::OutBack),
                    }),
                });
                connectToWidget(onAnimation, widget);
                onAnimation->setParent(data);
                data->anims[2] = onAnimation;

                auto *offAnimation = makeParallelAnimationGroup({
                    makeParallelAnimationGroup({
                        makePropertyAnimation(data, PROP_NAMES[0], 1.0, 4.0, 0.4 * totalDuration, QEasingCurve::OutCubic),
                        makePropertyAnimation(data, PROP_NAMES[4], 1.0, 0.0, 0.4 * totalDuration, QEasingCurve::OutCubic),
                    }),
                    makeSequentialAnimationGroup({
                        new QPauseAnimation(0.2 * totalDuration),
                        makeParallelAnimationGroup({
                            makePropertyAnimation(data, PROP_NAMES[1], 1.0, 4.0, 0.4 * totalDuration, QEasingCurve::OutCubic),
                            makePropertyAnimation(data, PROP_NAMES[5], 1.0, 0.0, 0.4 * totalDuration, QEasingCurve::OutCubic),
                        }),
                    }),
                    makeSequentialAnimationGroup({
                        new QPauseAnimation(0.4 * totalDuration),
                        makeParallelAnimationGroup({
                            makePropertyAnimation(data, PROP_NAMES[2], 1.0, 4.0, 0.4 * totalDuration, QEasingCurve::OutCubic),
                            makePropertyAnimation(data, PROP_NAMES[6], 1.0, 0.0, 0.4 * totalDuration, QEasingCurve::OutCubic),
                        }),
                    }),
                    makeSequentialAnimationGroup({
                        new QPauseAnimation(0.6 * totalDuration),
                        makeParallelAnimationGroup({
                            makePropertyAnimation(data, PROP_NAMES[3], 1.0, 4.0, 0.4 * totalDuration, QEasingCurve::OutCubic),
                            makePropertyAnimation(data, PROP_NAMES[7], 1.0, 0.0, 0.4 * totalDuration, QEasingCurve::OutCubic),
                        }),
                    }),
                });
                connectToWidget(offAnimation, widget);
                offAnimation->setParent(data);
                data->anims[3] = offAnimation;
            }
            if (checkBoxState == CheckPartial && startAnim) {
                for (int i = 0; i < sizeof(PROP_NAMES)/sizeof(*PROP_NAMES); ++i) {
                    data->setProperty(PROP_NAMES[i], PROP_INIT_VALUES_ON[i]);
                }
                data->anims[2]->start();
            }
            if (previousCheckBoxState == CheckPartial && startAnim) {
                for (int i = 0; i < sizeof(PROP_NAMES)/sizeof(*PROP_NAMES); ++i) {
                    data->setProperty(PROP_NAMES[i], PROP_INIT_VALUES_OFF[i]);
                }
                data->anims[3]->start();
            }

            painter->setPen(Qt::NoPen);

            qreal r[] = {
                data->property(PROP_NAMES[0]).toReal(),
                data->property(PROP_NAMES[1]).toReal(),
                data->property(PROP_NAMES[2]).toReal(),
                data->property(PROP_NAMES[3]).toReal(),
            };
            qreal a[] = {
                data->property(PROP_NAMES[4]).toReal(),
                data->property(PROP_NAMES[5]).toReal(),
                data->property(PROP_NAMES[6]).toReal(),
                data->property(PROP_NAMES[7]).toReal(),
            };

            for (int i = 0; i < sizeof(points)/sizeof(*points); ++i) {
                QColor brush = foreground;
                brush.setAlphaF(a[i]);
                painter->setBrush(brush);
                painter->drawEllipse(points[i] + centerOffset, r[i], r[i]);
            }
        }
    }
}

}
