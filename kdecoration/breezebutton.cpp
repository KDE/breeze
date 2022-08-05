/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "breezebutton.h"
#include "colortools.h"
#include "renderdecorationbuttonicon.h"

#include <KColorScheme>
#include <KColorUtils>
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationButtonGroup>
#include <KIconLoader>
#include <KWindowSystem>

#include <QPainter>
#include <QPainterPath>
#include <QVariantAnimation>

namespace Breeze
{

using KDecoration2::ColorGroup;
using KDecoration2::ColorRole;
using KDecoration2::DecorationButtonType;

//__________________________________________________________________
Button::Button(DecorationButtonType type, Decoration *decoration, QObject *parent)
    : DecorationButton(type, decoration, parent)
    , m_animation(new QVariantAnimation(this))
    , m_isGtkCsdButton(false)
{
    auto c = decoration->client().toStrongRef();
    Q_ASSERT(c);

    // setup animation
    // It is important start and end value are of the same type, hence 0.0 and not just 0
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        setOpacity(value.toReal());
    });

    // detect the kde-gtk-config-daemon
    if (QCoreApplication::applicationName() == QStringLiteral("kded5")) {
        // From Chris Holland https://github.com/Zren/material-decoration/
        // kde-gtk-config has a kded5 module which renders the buttons to svgs for gtk.
        m_isGtkCsdButton = true;
    }

    // setup default geometry
    int smallButtonPaddedHeight = decoration->smallButtonPaddedHeight();
    int iconHeight = decoration->iconHeight();
    int smallButtonBackgroundHeight = decoration->smallButtonBackgroundHeight();

    setGeometry(QRect(0, 0, smallButtonPaddedHeight, smallButtonPaddedHeight));
    setSmallButtonPaddedSize(QSize(smallButtonPaddedHeight, smallButtonPaddedHeight));
    setIconSize(QSize(iconHeight, iconHeight));
    setBackgroundVisibleSize((QSizeF(smallButtonBackgroundHeight, smallButtonBackgroundHeight)));

    // connections
    connect(c.data(), SIGNAL(iconChanged(QIcon)), this, SLOT(update()));
    connect(decoration->settings().data(), &KDecoration2::DecorationSettings::reconfigured, this, &Button::reconfigure);
    connect(this, &KDecoration2::DecorationButton::hoveredChanged, this, &Button::updateAnimationState);
    connect(this, &KDecoration2::DecorationButton::hoveredChanged, this, &Button::updateThinWindowOutlineWithButtonColor);
    connect(this, &KDecoration2::DecorationButton::pressedChanged, this, &Button::updateThinWindowOutlineWithButtonColor);

    reconfigure();
}

//__________________________________________________________________
Button::Button(QObject *parent, const QVariantList &args)
    : Button(args.at(0).value<DecorationButtonType>(), args.at(1).value<Decoration *>(), parent)
{
    m_flag = FlagStandalone;
    //! small button size must return to !valid because it was altered from the default constructor,
    //! in Standalone mode the button is not using the decoration metrics but its geometry
    m_smallButtonPaddedSize = QSize(-1, -1);
}

//__________________________________________________________________
Button *Button::create(DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent)
{
    if (auto d = qobject_cast<Decoration *>(decoration)) {
        auto c = d->client().toStrongRef();
        Q_ASSERT(c);

        Button *b = new Button(type, d, parent);
        switch (type) {
        case DecorationButtonType::Close:
            b->setVisible(c->isCloseable());
            QObject::connect(c.data(), &KDecoration2::DecoratedClient::closeableChanged, b, &Breeze::Button::setVisible);
            break;

        case DecorationButtonType::Maximize:
            b->setVisible(c->isMaximizeable());
            QObject::connect(c.data(), &KDecoration2::DecoratedClient::maximizeableChanged, b, &Breeze::Button::setVisible);
            break;

        case DecorationButtonType::Minimize:
            b->setVisible(c->isMinimizeable());
            QObject::connect(c.data(), &KDecoration2::DecoratedClient::minimizeableChanged, b, &Breeze::Button::setVisible);
            break;

        case DecorationButtonType::ContextHelp:
            b->setVisible(c->providesContextHelp());
            QObject::connect(c.data(), &KDecoration2::DecoratedClient::providesContextHelpChanged, b, &Breeze::Button::setVisible);
            break;

        case DecorationButtonType::Shade:
            b->setVisible(c->isShadeable());
            QObject::connect(c.data(), &KDecoration2::DecoratedClient::shadeableChanged, b, &Breeze::Button::setVisible);
            break;

        case DecorationButtonType::Menu:
            QObject::connect(c.data(), &KDecoration2::DecoratedClient::iconChanged, b, [b]() {
                b->update();
            });
            break;

        default:
            break;
        }

        return b;
    }

    return nullptr;
}

//__________________________________________________________________
void Button::paint(QPainter *painter, const QRect &repaintRegion)
{
    Q_UNUSED(repaintRegion)

    if (!decoration())
        return;
    auto d = qobject_cast<Decoration *>(decoration());
    auto c = d->client().toStrongRef();
    Q_ASSERT(c);

    setDevicePixelRatio(painter);
    setShouldDrawBoldButtonIcons();
    m_systemIconIsAvailable = false;
    if (d->internalSettings()->buttonIconStyle() == InternalSettings::EnumButtonIconStyle::StyleSystemIconTheme)
        m_systemIconIsAvailable = isSystemIconAvailable();
    setStandardScaledPenWidth();

    m_backgroundColor = this->backgroundColor();
    m_foregroundColor = this->foregroundColor();

    m_lowContrastBetweenTitleBarAndBackground = (d->internalSettings()->backgroundColors() != InternalSettings::EnumBackgroundColors::ColorsTitlebarText
                                                 && (KColorUtils::contrastRatio(m_backgroundColor, d->titleBarColor()) < 1.3));

    if (shouldDrawBackgroundStroke())
        m_outlineColor = this->outlineColor();

    if (!m_smallButtonPaddedSize.isValid() || isStandAlone()) {
        m_smallButtonPaddedSize = geometry().size().toSize();
        int iconWidth = qRound(qreal(m_smallButtonPaddedSize.width()) * 0.9);
        setIconSize(QSize(iconWidth, iconWidth));
        setBackgroundVisibleSize(QSizeF(iconWidth, iconWidth));
    }

    painter->save();

    // menu button
    if (type() == DecorationButtonType::Menu) {
        // draw a background only with Full-sized background shapes;
        // for standalone/GTK we draw small buttons so can't draw menu
        if (d->buttonBackgroundType() == ButtonBackgroundType::FullHeight && !(isStandAlone() || m_isGtkCsdButton))
            paintFullHeightButtonBackground(painter);

        // translate from icon offset
        painter->translate(m_iconOffset);

        const QRectF iconRect(geometry().topLeft(), m_smallButtonPaddedSize);
        if (auto deco = qobject_cast<Decoration *>(decoration())) {
            const QPalette activePalette = KIconLoader::global()->customPalette();
            QPalette palette = c->palette();
            palette.setColor(QPalette::WindowText, deco->fontColor());
            KIconLoader::global()->setCustomPalette(palette);
            c->icon().paint(painter, iconRect.toRect());
            if (activePalette == QPalette()) {
                KIconLoader::global()->resetPalette();
            } else {
                KIconLoader::global()->setCustomPalette(palette);
            }
        } else {
            c->icon().paint(painter, iconRect.toRect());
        }

    } else {
        drawIcon(painter);
    }

    painter->restore();
}

//__________________________________________________________________
void Button::drawIcon(QPainter *painter) const
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d)
        return;

    painter->setRenderHints(QPainter::Antialiasing);

    // for standalone/GTK we draw small buttons so don't do anything
    if (!(isStandAlone() || m_isGtkCsdButton)) {
        // draw a background only with Full-sized Rectangle button shape;
        // NB: paintFullHeightRectangleBackground function applies a translation to painter as different full-sized button geometry
        if (d->buttonBackgroundType() == ButtonBackgroundType::FullHeight)
            paintFullHeightButtonBackground(painter);
    }

    // get the device offset of the paddedIcon from the top-left of the titlebar as a reference-point for pixel-snapping algorithms
    //(ideally, the device offset from the top-left of the screen would be better for fractional scaling, but it is not available in the API)
    QPointF deviceOffsetTitleBarTopLeftToIconTopLeft;
    QPointF topLeftPaddedButtonDeviceGeometry = painter->deviceTransform().map(geometry().topLeft());

    // get top-left geometry relative to the titlebar top-left as is the best reference position available that is most likely to be a whole pixel
    //(on button hover sometimes the painter gives geometry relative to the button rather than to titlebar, so this is also why this is necessary)
    QPointF titleBarTopLeftDeviceGeometry = painter->deviceTransform().map(QRectF(d->titleBar()).topLeft());
    deviceOffsetTitleBarTopLeftToIconTopLeft = topLeftPaddedButtonDeviceGeometry - titleBarTopLeftDeviceGeometry;

    painter->translate(geometry().topLeft());

    // translate from icon offset -- translates to the edge of smallButtonPaddedWidth
    painter->translate(m_iconOffset);
    deviceOffsetTitleBarTopLeftToIconTopLeft += (m_iconOffset * painter->device()->devicePixelRatioF());

    const qreal smallButtonPaddedWidth(m_smallButtonPaddedSize.width());
    qreal iconWidth(m_iconSize.width());
    if (d->buttonBackgroundType() == ButtonBackgroundType::Small || isStandAlone() || m_isGtkCsdButton)
        paintSmallSizedButtonBackground(painter);

    // translate to draw icon in the centre of smallButtonPaddedWidth (smallButtonPaddedWidth has additional padding)
    qreal iconTranslationOffset = (smallButtonPaddedWidth - iconWidth) / 2;
    painter->translate(iconTranslationOffset, iconTranslationOffset);
    deviceOffsetTitleBarTopLeftToIconTopLeft += (QPointF(iconTranslationOffset, iconTranslationOffset) * painter->device()->devicePixelRatioF());

    qreal scaleFactor = 1;
    if (!m_systemIconIsAvailable) {
        scaleFactor = iconWidth / 18;
        /*
        scale painter so that all further rendering is preformed inside QRect( 0, 0, 18, 18 )
        */
        painter->scale(scaleFactor, scaleFactor);
    }

    // render mark
    if (m_foregroundColor.isValid()) {
        // setup painter
        QPen pen(m_foregroundColor);

        // this method commented out is for original non-cosmetic pen painting method (gives blurry icons at larger sizes )
        // pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/smallButtonPaddedWidth ) );

        // cannot use a scaled cosmetic pen if GTK CSD as kde-gtk-config generates svg icons. TODO:don't use cosmetic pen for background outlines either
        if (m_isGtkCsdButton) {
            pen.setWidthF(PenWidth::Symbol);
        } else {
            pen.setWidthF(m_standardScaledPenWidth);
            pen.setCosmetic(true);
        }
        painter->setPen(pen);

        std::unique_ptr<RenderDecorationButtonIcon18By18> iconRenderer;

        if (d->internalSettings()->buttonIconStyle() == InternalSettings::EnumButtonIconStyle::StyleSystemIconTheme) {
            iconRenderer = RenderDecorationButtonIcon18By18::factory(d->internalSettings(), painter, false, m_boldButtonIcons, iconWidth, m_devicePixelRatio);
        } else
            iconRenderer = RenderDecorationButtonIcon18By18::factory(d->internalSettings(),
                                                                     painter,
                                                                     false,
                                                                     m_boldButtonIcons,
                                                                     18,
                                                                     m_devicePixelRatio,
                                                                     deviceOffsetTitleBarTopLeftToIconTopLeft);

        switch (type()) {
        case DecorationButtonType::Close: {
            iconRenderer->renderCloseIcon();
            break;
        }

        case DecorationButtonType::Maximize: {
            if (isChecked())
                iconRenderer->renderRestoreIcon();
            else
                iconRenderer->renderMaximizeIcon();
            break;
        }

        case DecorationButtonType::Minimize: {
            iconRenderer->renderMinimizeIcon();
            break;
        }

        case DecorationButtonType::OnAllDesktops: {
            if (isChecked())
                iconRenderer->renderPinnedOnAllDesktopsIcon();
            else
                iconRenderer->renderPinOnAllDesktopsIcon();
            break;
        }

        case DecorationButtonType::Shade: {
            if (isChecked())
                iconRenderer->renderUnShadeIcon();
            else
                iconRenderer->renderShadeIcon();
            break;
        }

        case DecorationButtonType::KeepBelow: {
            iconRenderer->renderKeepBehindIcon();
            break;
        }

        case DecorationButtonType::KeepAbove: {
            iconRenderer->renderKeepInFrontIcon();
            break;
        }

        case DecorationButtonType::ApplicationMenu: {
            iconRenderer->renderApplicationMenuIcon();
            break;
        }

        case DecorationButtonType::ContextHelp: {
            iconRenderer->renderContextHelpIcon();
            break;
        }

        default:
            break;
        }
    }
}

//__________________________________________________________________
QColor Button::foregroundColor() const
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d)
        return QColor();

    auto c = d->client().toStrongRef();
    Q_ASSERT(c);

    QColor color;
    if (!d->internalSettings()->translucentButtonBackgrounds())
        color = ColorTools::getHigherContrastForegroundColor(d->fontColor(), m_backgroundColor, 2.3);
    else
        color = d->fontColor();

    if (isPressed()) {
        if (type() == DecorationButtonType::Close)
            return Qt::GlobalColor::white;
        else if (d->internalSettings()->backgroundColors() != InternalSettings::EnumBackgroundColors::ColorsTitlebarText) {
            return color;
        } else if (d->internalSettings()->translucentButtonBackgrounds())
            return d->fontColor();
        else
            return d->titleBarColor();

    } else if ((type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade)
               && isChecked()) {
        if (d->internalSettings()->backgroundColors() != InternalSettings::EnumBackgroundColors::ColorsTitlebarText)
            return color;
        else if (d->internalSettings()->translucentButtonBackgrounds())
            return d->fontColor();
        else
            return d->titleBarColor();
    } else if (type() == DecorationButtonType::OnAllDesktops && isChecked()
               && d->internalSettings()->backgroundColors() != InternalSettings::EnumBackgroundColors::ColorsTitlebarText) {
        return color;
    } else if (m_animation->state() == QAbstractAnimation::Running) {
        if (type() == DecorationButtonType::Close) {
            if (d->internalSettings()->alwaysShow() == InternalSettings::EnumAlwaysShow::AlwaysShowIconsAndCloseButtonBackground) {
                if (d->internalSettings()->redAlwaysShownClose() && c->isActive()) {
                    return KColorUtils::mix(d->fontColor(), Qt::GlobalColor::white, m_opacity);
                } else if (d->internalSettings()->backgroundColors() != InternalSettings::EnumBackgroundColors::ColorsTitlebarText) {
                    return KColorUtils::mix(d->fontColor(), Qt::GlobalColor::white, m_opacity);
                } else if (d->internalSettings()->translucentButtonBackgrounds())
                    return KColorUtils::mix(d->fontColor(), Qt::GlobalColor::white, m_opacity);
                else
                    return KColorUtils::mix(d->titleBarColor(), Qt::GlobalColor::white, m_opacity);
            } else {
                return KColorUtils::mix(d->fontColor(), Qt::GlobalColor::white, m_opacity);
            }
        } else if (d->internalSettings()->backgroundColors() != InternalSettings::EnumBackgroundColors::ColorsTitlebarText)
            return KColorUtils::mix(d->fontColor(), color, m_opacity);
        else if (d->internalSettings()->translucentButtonBackgrounds())
            return d->fontColor();
        else
            return KColorUtils::mix(d->fontColor(), d->titleBarColor(), m_opacity);

    } else if (isHovered()) {
        if (type() == DecorationButtonType::Close)
            return Qt::GlobalColor::white;
        else if (d->internalSettings()->backgroundColors() != InternalSettings::EnumBackgroundColors::ColorsTitlebarText)
            return color;
        else if (d->internalSettings()->translucentButtonBackgrounds())
            return d->fontColor();
        else
            return d->titleBarColor();
    } else if (type() == DecorationButtonType::Close
               && d->internalSettings()->alwaysShow() == InternalSettings::EnumAlwaysShow::AlwaysShowIconsAndCloseButtonBackground) {
        if (d->internalSettings()->redAlwaysShownClose() && c->isActive()) {
            return d->fontColor();
        } else if (d->internalSettings()->backgroundColors() != InternalSettings::EnumBackgroundColors::ColorsTitlebarText) {
            return d->fontColor();
        } else if (d->internalSettings()->translucentButtonBackgrounds())
            return d->fontColor();
        else
            return d->titleBarColor();
    } else {
        return d->fontColor();
    }
}

//__________________________________________________________________
QColor Button::backgroundColor(bool getNonAnimatedColor) const
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d) {
        return QColor();
    }

    auto c = d->client().toStrongRef();
    Q_ASSERT(c);

    QColor buttonAlwaysShowColor;
    QColor buttonHoverColor;
    QColor buttonFocusColor;

    // set hover and focus colours
    if (d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccent
        || d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights) {
        if (d->internalSettings()->translucentButtonBackgrounds()) {
            if (type() == DecorationButtonType::Close) {
                if (d->internalSettings()->alwaysShow() == InternalSettings::EnumAlwaysShow::AlwaysShowIconsAndCloseButtonBackground) {
                    if (d->internalSettings()->redAlwaysShownClose() && c->isActive()) {
                        buttonAlwaysShowColor = g_decorationColors->negativeReducedOpacityBackground;
                        buttonHoverColor = g_decorationColors->negativeReducedOpacityOutline;
                        buttonFocusColor = g_decorationColors->negativeReducedOpacityLessSaturatedBackground;
                    } else {
                        buttonAlwaysShowColor = g_decorationColors->buttonReducedOpacityBackground;
                        buttonHoverColor = g_decorationColors->negativeReducedOpacityOutline;
                        buttonFocusColor = g_decorationColors->negativeReducedOpacityLessSaturatedBackground;
                    }
                } else {
                    buttonHoverColor = g_decorationColors->negativeReducedOpacityBackground;
                    buttonFocusColor = g_decorationColors->negativeReducedOpacityOutline;
                }
            } else if (type() == DecorationButtonType::Minimize
                       && d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights) {
                buttonHoverColor = g_decorationColors->neutralReducedOpacityBackground;
                buttonFocusColor = g_decorationColors->neutralReducedOpacityOutline;
            } else if (type() == DecorationButtonType::Maximize
                       && d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights) {
                buttonHoverColor = g_decorationColors->positiveReducedOpacityBackground;
                buttonFocusColor = g_decorationColors->positiveReducedOpacityOutline;
            } else {
                buttonHoverColor = g_decorationColors->buttonReducedOpacityBackground;
                buttonFocusColor = g_decorationColors->buttonReducedOpacityOutline;
            }
        } else {
            if (type() == DecorationButtonType::Close) {
                if (d->internalSettings()->alwaysShow() == InternalSettings::EnumAlwaysShow::AlwaysShowIconsAndCloseButtonBackground) {
                    if (d->internalSettings()->redAlwaysShownClose() && c->isActive()) {
                        buttonAlwaysShowColor = g_decorationColors->negative;
                        buttonHoverColor = g_decorationColors->negativeSaturated;
                        buttonFocusColor = g_decorationColors->negativeLessSaturated;
                    } else {
                        buttonAlwaysShowColor = g_decorationColors->buttonHover;
                        buttonHoverColor = g_decorationColors->negativeSaturated;
                        buttonFocusColor = g_decorationColors->negativeLessSaturated;
                    }
                } else {
                    buttonHoverColor = g_decorationColors->negative;
                    buttonFocusColor = g_decorationColors->negativeSaturated;
                }
            } else if (type() == DecorationButtonType::Minimize
                       && d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights) {
                buttonHoverColor = g_decorationColors->neutralLessSaturated;
                buttonFocusColor = g_decorationColors->neutral;
            } else if (type() == DecorationButtonType::Maximize
                       && d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights) {
                buttonHoverColor = g_decorationColors->positiveLessSaturated;
                buttonFocusColor = g_decorationColors->positive;
            } else {
                buttonHoverColor = g_decorationColors->buttonHover;
                buttonFocusColor = g_decorationColors->buttonFocus;
            }
        }

    } else {
        if (d->internalSettings()->translucentButtonBackgrounds()) {
            if (type() == DecorationButtonType::Close) {
                if (d->internalSettings()->alwaysShow() == InternalSettings::EnumAlwaysShow::AlwaysShowIconsAndCloseButtonBackground) {
                    if (d->internalSettings()->redAlwaysShownClose() && c->isActive()) {
                        buttonAlwaysShowColor = g_decorationColors->negativeReducedOpacityBackground;
                        buttonHoverColor = g_decorationColors->negativeReducedOpacityOutline;
                        buttonFocusColor = g_decorationColors->negativeReducedOpacityLessSaturatedBackground;
                    } else {
                        buttonAlwaysShowColor = d->fontColor();
                        buttonAlwaysShowColor.setAlphaF(buttonAlwaysShowColor.alphaF() * 0.15);
                        buttonHoverColor = g_decorationColors->negativeReducedOpacityOutline;
                        buttonFocusColor = g_decorationColors->negativeReducedOpacityBackground;
                    }
                } else {
                    buttonHoverColor = g_decorationColors->negativeReducedOpacityBackground;
                    buttonFocusColor = g_decorationColors->negativeReducedOpacityOutline;
                }
            } else {
                buttonHoverColor = d->fontColor();
                buttonHoverColor.setAlphaF(buttonHoverColor.alphaF() * 0.15);
                buttonFocusColor = d->fontColor();
                buttonFocusColor.setAlphaF(buttonFocusColor.alphaF() * 0.25);
            }
        } else {
            if (type() == DecorationButtonType::Close) {
                if (d->internalSettings()->alwaysShow() == InternalSettings::EnumAlwaysShow::AlwaysShowIconsAndCloseButtonBackground) {
                    if (d->internalSettings()->redAlwaysShownClose() && c->isActive()) {
                        buttonAlwaysShowColor = g_decorationColors->negative;
                        buttonHoverColor = g_decorationColors->negativeSaturated;
                        buttonFocusColor = g_decorationColors->negativeLessSaturated;
                    } else {
                        buttonAlwaysShowColor = d->fontColor();
                        buttonHoverColor = g_decorationColors->negativeSaturated;
                        buttonFocusColor = g_decorationColors->negativeLessSaturated;
                    }
                } else {
                    buttonHoverColor = g_decorationColors->negative;
                    buttonFocusColor = g_decorationColors->negativeSaturated;
                }
            } else {
                buttonHoverColor = d->fontColor();
                buttonFocusColor = KColorUtils::mix(d->titleBarColor(), d->fontColor(), 0.3);
            }
        }
    }

    if (isPressed()) {
        return buttonFocusColor;

    } else if ((type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade)
               && isChecked()) {
        if (d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccent
            || d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights)
            return buttonFocusColor;
        else
            return buttonHoverColor;

    } else if (type() == DecorationButtonType::OnAllDesktops && isChecked()
               && (d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccent
                   || d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights)) {
        return buttonFocusColor;
    } else if (m_animation->state() == QAbstractAnimation::Running && !getNonAnimatedColor) {
        if (type() == DecorationButtonType::Close) {
            if (d->internalSettings()->alwaysShow() == InternalSettings::EnumAlwaysShow::AlwaysShowIconsAndCloseButtonBackground) {
                return KColorUtils::mix(buttonAlwaysShowColor, buttonHoverColor, m_opacity);
            } else {
                QColor color(buttonHoverColor);
                color.setAlphaF(color.alphaF() * m_opacity);
                return color;
            }

        } else {
            QColor color(buttonHoverColor);
            color.setAlphaF(color.alphaF() * m_opacity);
            return color;
        }

    } else if (isHovered()) {
        return buttonHoverColor;

    } else if (type() == DecorationButtonType::Close
               && d->internalSettings()->alwaysShow() == InternalSettings::EnumAlwaysShow::AlwaysShowIconsAndCloseButtonBackground) {
        return buttonAlwaysShowColor;
    } else {
        return QColor();
    }
}

//__________________________________________________________________
QColor Button::outlineColor(bool getNonAnimatedColor) const
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d)
        return QColor();

    auto c = d->client().toStrongRef();
    Q_ASSERT(c);

    QColor buttonOutlineColor;
    // set colour
    if (d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccent
        || d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights) {
        if (d->internalSettings()->translucentButtonBackgrounds()) {
            if (type() == DecorationButtonType::Close) {
                buttonOutlineColor = g_decorationColors->negativeReducedOpacityOutline;
            } else if (type() == DecorationButtonType::Minimize
                       && d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights) {
                buttonOutlineColor = g_decorationColors->neutralReducedOpacityOutline;
            } else if (type() == DecorationButtonType::Maximize
                       && d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights) {
                buttonOutlineColor = g_decorationColors->positiveReducedOpacityOutline;
            } else {
                buttonOutlineColor = g_decorationColors->buttonReducedOpacityOutline;
            }
        } else {
            if (type() == DecorationButtonType::Close) {
                buttonOutlineColor = g_decorationColors->negativeSaturated;
            } else if (type() == DecorationButtonType::Minimize
                       && d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights) {
                buttonOutlineColor = g_decorationColors->neutral;
            } else if (type() == DecorationButtonType::Maximize
                       && d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights) {
                buttonOutlineColor = g_decorationColors->positive;
            } else {
                buttonOutlineColor = g_decorationColors->buttonFocus;
            }
        }

    } else {
        if (d->internalSettings()->translucentButtonBackgrounds()) {
            if (type() == DecorationButtonType::Close) {
                buttonOutlineColor = g_decorationColors->negativeReducedOpacityOutline;
            } else {
                buttonOutlineColor = d->fontColor();
                buttonOutlineColor.setAlphaF(buttonOutlineColor.alphaF() * 0.25);
            }
        } else {
            if (type() == DecorationButtonType::Close) {
                buttonOutlineColor = g_decorationColors->negativeSaturated;
            } else {
                buttonOutlineColor = KColorUtils::mix(d->titleBarColor(), d->fontColor(), 0.3);
            }
        }
    }

    if ((d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccent
         || d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights)
        && KColorUtils::contrastRatio(buttonOutlineColor, d->titleBarColor()) < 1.3) {
        buttonOutlineColor = d->fontColor();
        buttonOutlineColor.setAlphaF(buttonOutlineColor.alphaF() * 0.8);
    }

    if (isPressed()
        || (isChecked() && (type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove || type() == DecorationButtonType::Shade))) {
        return buttonOutlineColor;
    } else if (type() == DecorationButtonType::OnAllDesktops && isChecked()
               && (d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccent
                   || d->internalSettings()->backgroundColors() == InternalSettings::EnumBackgroundColors::ColorsAccentWithTrafficLights)) {
        return buttonOutlineColor;
    } else if (m_animation->state() == QAbstractAnimation::Running && !getNonAnimatedColor) {
        QColor color(buttonOutlineColor);
        color.setAlphaF(color.alphaF() * m_opacity);
        return color;
    } else if (type() == DecorationButtonType::Close
               && d->internalSettings()->alwaysShow() == InternalSettings::EnumAlwaysShow::AlwaysShowIconsAndCloseButtonBackground) {
        if (isHovered()) {
            return buttonOutlineColor;
        } else
            return QColor();

    } else {
        return buttonOutlineColor;
    }
}

//________________________________________________________________
void Button::reconfigure()
{
    // animation
    auto d = qobject_cast<Decoration *>(decoration());
    if (d)
        m_animation->setDuration(d->animationsDuration());
}

//__________________________________________________________________
void Button::updateAnimationState(bool hovered)
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!(d && d->animationsDuration() > 0))
        return;

    m_animation->setDirection(hovered ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
    if (m_animation->state() != QAbstractAnimation::Running)
        m_animation->start();
}

void Button::updateThinWindowOutlineWithButtonColor(bool on)
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d || !d->internalSettings()->colorizeThinWindowOutlineWithButton() || isStandAlone())
        return;

    QColor color = QColor();
    if (on) {
        if (shouldDrawBackgroundStroke() && m_outlineColor.isValid()) // outline color valid as sometimes the former is true even when not a valid colour (i.e.
                                                                      // when always show close background active with red button)
            color = this->outlineColor(true);
        else
            color = this->backgroundColor(true);
        d->setThinWindowOutlineOverrideColor(on, color);
    } else {
        bool otherButtonIsHoveredOrPressed = false;

        // Check if any other button is hovered/pressed.
        // This is to prevent glitches when you directly mouse over one button to another and the second button does not trigger on.
        // In the case where another button is hovered/pressed do not send an off flag.
        for (QPointer<KDecoration2::DecorationButton> &decButton : d->leftButtons()->buttons() + d->rightButtons()->buttons()) {
            Button *button = static_cast<Button *>(decButton.data());

            if (button != this && (button->isHovered() || button->isPressed())) {
                otherButtonIsHoveredOrPressed = true;
                break;
            }
        }
        if (!otherButtonIsHoveredOrPressed)
            d->setThinWindowOutlineOverrideColor(on, color);
    }
}

bool Button::shouldDrawBackgroundStroke() const
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d)
        return false;

    if (d->internalSettings()->alwaysShowIconHighlightUsing()
        == InternalSettings::EnumAlwaysShowIconHighlightUsing::AlwaysShowIconHighlightUsingBackgroundAndOutline)
        return true;
    else
        return (m_lowContrastBetweenTitleBarAndBackground);
}

void Button::paintFullHeightButtonBackground(QPainter *painter) const
{
    if (m_backgroundColor.isValid()) {
        auto d = qobject_cast<Decoration *>(decoration());
        if (!d)
            return;
        auto s = d->settings();

        painter->save();
        painter->translate(m_fullHeightVisibleBackgroundOffset);

        QRectF backgroundBoundingRect = (QRectF(geometry().topLeft(), m_backgroundVisibleSize));
        painter->setClipRect(backgroundBoundingRect);
        QPainterPath background;
        QPainterPath outline;
        painter->setPen(Qt::NoPen);

        bool drawOutline = false;
        bool drawOutlineUsingPath = false;

        qreal penWidth = PenWidth::Symbol;
        qreal geometryShrinkOffsetHorizontal = PenWidth::Symbol * 1.5;
        if (KWindowSystem::isPlatformX11()) {
            penWidth *= m_devicePixelRatio;
            geometryShrinkOffsetHorizontal *= m_devicePixelRatio;
        }

        if (shouldDrawBackgroundStroke()) {
            QRectF innerRect;
            QRectF outerRect;
            if (m_outlineColor.isValid()) {
                drawOutline = true;
            }
            // drawOutline=false is for the case when you still want to shrink the button but don't want an outline e.g. with always show
            // highlighted close button and not hovered/pressed

            qreal geometryShrinkOffsetVertical = geometryShrinkOffsetHorizontal;

            if (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeFullHeightRoundedRectangle) {
                // shrink the backgroundBoundingRect to make border more visible
                backgroundBoundingRect = QRectF(backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontal,
                                                                                geometryShrinkOffsetVertical,
                                                                                -geometryShrinkOffsetHorizontal,
                                                                                -geometryShrinkOffsetVertical));
                background.addRoundedRect(backgroundBoundingRect, d->scaledCornerRadius(), d->scaledCornerRadius());

            } else if (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeIntegratedRoundedRectangle) {
                qreal halfPenWidth = penWidth / 2;
                geometryShrinkOffsetHorizontal = halfPenWidth;
                geometryShrinkOffsetVertical = qMax(0.0, d->internalSettings()->integratedRoundedRectangleBottomPadding() * s->smallSpacing() - penWidth);
                qreal geometryShrinkOffsetHorizontalOuter = geometryShrinkOffsetHorizontal - halfPenWidth;
                qreal geometryShrinkOffsetHorizontalInner = geometryShrinkOffsetHorizontal + halfPenWidth;
                qreal geometryShrinkOffsetVerticalOuter = geometryShrinkOffsetVertical - halfPenWidth;
                qreal geometryShrinkOffsetVerticalInner = geometryShrinkOffsetVertical + halfPenWidth;
                qreal extensionByCornerRadiusInnerOuter = d->scaledCornerRadius() + halfPenWidth;
                drawOutlineUsingPath = true;

                if (m_rightmostRightVisible && !d->internalSettings()->titlebarRightMargin()) { // right-most-right
                    outerRect = backgroundBoundingRect.adjusted(0,
                                                                -extensionByCornerRadiusInnerOuter,
                                                                extensionByCornerRadiusInnerOuter,
                                                                -geometryShrinkOffsetVerticalOuter);
                    innerRect = backgroundBoundingRect.adjusted(penWidth,
                                                                -extensionByCornerRadiusInnerOuter,
                                                                extensionByCornerRadiusInnerOuter,
                                                                -geometryShrinkOffsetVerticalInner);
                    backgroundBoundingRect =
                        backgroundBoundingRect.adjusted(halfPenWidth, -d->scaledCornerRadius(), d->scaledCornerRadius(), -geometryShrinkOffsetVertical);
                } else if (m_leftmostLeftVisible && !d->internalSettings()->titlebarLeftMargin()) { // left-most-left
                    outerRect = backgroundBoundingRect.adjusted(-extensionByCornerRadiusInnerOuter,
                                                                -extensionByCornerRadiusInnerOuter,
                                                                0,
                                                                -geometryShrinkOffsetVerticalOuter);
                    innerRect = backgroundBoundingRect.adjusted(-extensionByCornerRadiusInnerOuter,
                                                                -extensionByCornerRadiusInnerOuter,
                                                                -penWidth,
                                                                -geometryShrinkOffsetVerticalInner);
                    backgroundBoundingRect =
                        backgroundBoundingRect.adjusted(-d->scaledCornerRadius(), -d->scaledCornerRadius(), -halfPenWidth, -geometryShrinkOffsetVertical);
                } else {
                    outerRect = backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontalOuter,
                                                                -extensionByCornerRadiusInnerOuter,
                                                                -geometryShrinkOffsetHorizontalOuter,
                                                                -geometryShrinkOffsetVerticalOuter);
                    innerRect = backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontalInner,
                                                                -extensionByCornerRadiusInnerOuter,
                                                                -geometryShrinkOffsetHorizontalInner,
                                                                -geometryShrinkOffsetVerticalInner);
                    backgroundBoundingRect = backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontal,
                                                                             -d->scaledCornerRadius(),
                                                                             -geometryShrinkOffsetHorizontal,
                                                                             -geometryShrinkOffsetVertical);
                }

                qreal outerCornerRadius;
                if (d->scaledCornerRadius() >= 0.05)
                    outerCornerRadius = d->scaledCornerRadius() + halfPenWidth;
                else
                    outerCornerRadius = 0;
                qreal innerCornerRadius = qMax(0.0, d->scaledCornerRadius() - halfPenWidth);
                QPainterPath inner;
                inner.addRoundedRect(innerRect, innerCornerRadius, innerCornerRadius);
                outline.addRoundedRect(outerRect, outerCornerRadius, outerCornerRadius);
                outline = outline.subtracted(inner);
                background.addRoundedRect(backgroundBoundingRect, d->scaledCornerRadius(), d->scaledCornerRadius());
            } else { // plain rectangle

                // shrink the backgroundBoundingRect to make border more visible
                backgroundBoundingRect = backgroundBoundingRect.adjusted(geometryShrinkOffsetHorizontal,
                                                                         geometryShrinkOffsetVertical,
                                                                         -geometryShrinkOffsetHorizontal,
                                                                         -geometryShrinkOffsetVertical);
                background.addRect(backgroundBoundingRect);
            }

        } else {
            painter->setPen(Qt::NoPen);
            if (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeFullHeightRoundedRectangle)
                background.addRoundedRect(backgroundBoundingRect, d->scaledCornerRadius(), d->scaledCornerRadius());

            else if (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeIntegratedRoundedRectangle) {
                qreal geometryShrinkOffsetVertical = d->internalSettings()->integratedRoundedRectangleBottomPadding() * s->smallSpacing() - penWidth;
                if (m_rightmostRightVisible && !d->internalSettings()->titlebarRightMargin()) { // right-most-right
                    backgroundBoundingRect =
                        backgroundBoundingRect.adjusted(0, -d->scaledCornerRadius(), d->scaledCornerRadius(), -geometryShrinkOffsetVertical);
                } else if (m_leftmostLeftVisible && !d->internalSettings()->titlebarLeftMargin()) { // left-most-left
                    backgroundBoundingRect =
                        backgroundBoundingRect.adjusted(-d->scaledCornerRadius(), -d->scaledCornerRadius(), 0, -geometryShrinkOffsetVertical);
                } else {
                    backgroundBoundingRect = backgroundBoundingRect.adjusted(0, -d->scaledCornerRadius(), 0, -geometryShrinkOffsetVertical);
                }
                background.addRoundedRect(backgroundBoundingRect, d->scaledCornerRadius(), d->scaledCornerRadius());
            } else // plain rectangle
                background.addRect(backgroundBoundingRect);
        }

        // clip the rounded corners using the windowPath
        if (!d->isMaximized())
            background = background.intersected(*(d->windowPath()));

        if (drawOutline && !drawOutlineUsingPath) {
            QPen pen(m_outlineColor);
            pen.setWidthF(m_standardScaledPenWidth);
            pen.setCosmetic(true);
            painter->setPen(pen);
        }
        painter->setBrush(m_backgroundColor);
        painter->drawPath(background);

        if (drawOutline && drawOutlineUsingPath) {
            // clip the rounded corners using the windowPath
            if (!d->isMaximized())
                outline = outline.intersected(*(d->windowPath()));
            painter->setBrush(m_outlineColor);
            painter->drawPath(outline);
        }

        painter->restore();
    }
}

void Button::paintSmallSizedButtonBackground(QPainter *painter) const
{
    if (m_backgroundColor.isValid()) {
        auto d = qobject_cast<Decoration *>(decoration());
        if (!d)
            return;

        painter->save();

        qreal translationOffset = (m_smallButtonPaddedSize.width() - m_backgroundVisibleSize.width()) / 2;
        painter->translate(translationOffset, translationOffset);
        qreal geometryShrinkOffset = 0;
        qreal backgroundSize = m_backgroundVisibleSize.width();

        if (shouldDrawBackgroundStroke() && m_outlineColor.isValid()) {
            QPen pen(m_outlineColor);
            pen.setWidthF(m_standardScaledPenWidth);
            pen.setCosmetic(true);
            painter->setPen(pen);
        } else
            painter->setPen(Qt::NoPen);
        painter->setBrush(m_backgroundColor);

        if (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeSmallSquare
            || d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeFullHeightRectangle
            || ((d->internalSettings()->cornerRadius() < 0.2)
                && (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeFullHeightRoundedRectangle))
            || ((d->internalSettings()->cornerRadius() < 0.2)
                && (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeIntegratedRoundedRectangle))) {
            painter->drawRect(
                QRectF(0 + geometryShrinkOffset, 0 + geometryShrinkOffset, backgroundSize - geometryShrinkOffset, backgroundSize - geometryShrinkOffset));
        } else if (d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeSmallRoundedSquare
                   || d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeFullHeightRoundedRectangle // case where standalone
                   || d->internalSettings()->buttonShape() == InternalSettings::EnumButtonShape::ShapeIntegratedRoundedRectangle // case where standalone
        ) {
            painter->drawRoundedRect(
                QRectF(0 + geometryShrinkOffset, 0 + geometryShrinkOffset, backgroundSize - geometryShrinkOffset, backgroundSize - geometryShrinkOffset),
                20,
                20,
                Qt::RelativeSize);
        } else
            painter->drawEllipse(
                QRectF(0 + geometryShrinkOffset, 0 + geometryShrinkOffset, backgroundSize - geometryShrinkOffset, backgroundSize - geometryShrinkOffset));

        painter->restore();
    }
}

void Button::setDevicePixelRatio(QPainter *painter)
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d)
        return;
    // determine DPR
    m_devicePixelRatio = painter->device()->devicePixelRatioF();

    // on X11 Kwin just returns 1.0 for the DPR instead of the correct value, so use the scaling setting directly
    if (KWindowSystem::isPlatformX11())
        m_devicePixelRatio = d->systemScaleFactor();
    if (m_isGtkCsdButton)
        m_devicePixelRatio = d->systemScaleFactor();
}

void Button::setStandardScaledPenWidth()
{
    m_standardScaledPenWidth = PenWidth::Symbol;
    m_standardScaledPenWidth *= m_devicePixelRatio; // this is assuming you are going to use setCosmetic(true) for pen sizes
}

void Button::setShouldDrawBoldButtonIcons()
{
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d)
        return;

    m_boldButtonIcons = false;

    if (!m_isGtkCsdButton) {
        switch (d->internalSettings()->boldButtonIcons()) {
        default:
            break;
        case InternalSettings::BoldIconsHiDpiOnly:
            // If HiDPI system scaling use bold icons
            if (m_devicePixelRatio > 1.2)
                m_boldButtonIcons = true;
            break;
        case InternalSettings::BoldIconsBold:
            m_boldButtonIcons = true;
            break;
        case InternalSettings::BoldIconsFine:
            break;
        }
    }
}

// When "Use system icon theme" is selected for the icons then not all icons are available as a window-*-symbolic icon
bool Button::isSystemIconAvailable()
{
    if (type() == DecorationButtonType::Menu || type() == DecorationButtonType::ApplicationMenu || type() == DecorationButtonType::OnAllDesktops
        || type() == DecorationButtonType::ContextHelp || type() == DecorationButtonType::Shade || type() == DecorationButtonType::Custom)
        return false;
    else
        return true;
}

} // namespace
