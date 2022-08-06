#ifndef BREEZE_DECORATION_H
#define BREEZE_DECORATION_H

/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "breeze.h"

#include "breezesettings.h"
#include "colortools.h"

#include <KDecoration2/DecoratedClient>
#include <KDecoration2/Decoration>
#include <KDecoration2/DecorationSettings>

#include <QPainterPath>
#include <QPalette>
#include <QVariant>
#include <QVariantAnimation>

#include <memory>

class QVariantAnimation;

namespace KDecoration2
{
class DecorationButton;
class DecorationButtonGroup;
}

namespace Breeze
{
class SizeGrip;

enum struct ButtonBackgroundType {
    Small,
    FullHeight,
};

class Decoration : public KDecoration2::Decoration
{
    Q_OBJECT

public:
    //* constructor
    explicit Decoration(QObject *parent = nullptr, const QVariantList &args = QVariantList());

    //* destructor
    virtual ~Decoration();

    //* paint
    void paint(QPainter *painter, const QRect &repaintRegion) override;

    //* internal settings
    InternalSettingsPtr internalSettings() const
    {
        return m_internalSettings;
    }

    qreal animationsDuration() const
    {
        return m_animation->duration();
    }

    //* caption height
    int captionHeight() const;

    //*@name active state change animation
    //@{
    void setOpacity(qreal);

    qreal opacity() const
    {
        return m_opacity;
    }

    //@}

    //*@name colors
    //@{
    QColor titleBarColor(bool returnNonAnimatedColor = false) const;
    QColor titleBarColorWithAddedTransparency() const;
    QColor titleBarSeparatorColor() const;
    QColor accentedWindowOutlineColor(QColor customColor = QColor()) const;
    QColor fontMixedAccentWindowOutlineColor(QColor customColor = QColor()) const;
    QColor fontColor() const;
    QColor overriddenOutlineColorAnimateIn() const;
    QColor overriddenOutlineColorAnimateOut(const QColor &destinationColor);
    //@}
    //
    //*@name maximization modes
    //@{
    inline bool isMaximized() const;
    inline bool isMaximizedHorizontally() const;
    inline bool isMaximizedVertically() const;

    inline bool isLeftEdge() const;
    inline bool isRightEdge() const;
    inline bool isTopEdge() const;
    inline bool isBottomEdge() const;

    inline bool hideTitleBar() const;
    //@}

    void setThinWindowOutlineOverrideColor(const bool on, const QColor &color);

    std::shared_ptr<QPainterPath> titleBarPath()
    {
        return m_titleBarPath;
    }
    std::shared_ptr<QPainterPath> windowPath()
    {
        return m_windowPath;
    }
    qreal systemScaleFactor() const
    {
        return m_systemScaleFactor;
    }
    ButtonBackgroundType buttonBackgroundType()
    {
        return m_buttonBackgroundType;
    }
    int smallButtonPaddedHeight()
    {
        return m_smallButtonPaddedHeight;
    }
    int iconHeight()
    {
        return m_iconHeight;
    }
    int smallButtonBackgroundHeight()
    {
        return m_smallButtonBackgroundHeight;
    }
    qreal scaledCornerRadius()
    {
        return m_scaledCornerRadius;
    }

    KDecoration2::DecorationButtonGroup *leftButtons()
    {
        return m_leftButtons;
    }

    KDecoration2::DecorationButtonGroup *rightButtons()
    {
        return m_rightButtons;
    }

public Q_SLOTS:
    void init() override;

private Q_SLOTS:
    void reconfigure();
    void recalculateBorders();
    void updateOpaque();
    void updateBlur();
    void updateButtonsGeometry();
    void updateButtonsGeometryDelayed();
    void updateTitleBar();
    void updateAnimationState();
    void updateSizeGripVisibility();

    void forceUpdateShadow()
    {
        updateShadow(true);
    }
    void onTabletModeChanged(bool mode);

private:
    //* return the rect in which caption will be drawn
    QPair<QRect, Qt::Alignment> captionRect() const;

    void createButtons();
    void calculateWindowAndTitleBarShapes(const bool windowShapeOnly = false);
    void paintTitleBar(QPainter *painter, const QRect &repaintRegion);
    void updateShadow(const bool force = false, const bool noCache = false, const bool isThinWindowOutlineOverride = false);
    QSharedPointer<KDecoration2::DecorationShadow> createShadowObject(const float strengthScale, const bool isThinWindowOutlineOverride = false);
    void setScaledCornerRadius();

    //*@name border size
    //@{
    int borderSize(bool bottom = false) const;
    inline bool hasBorders() const;
    inline bool hasNoBorders() const;
    inline bool hasNoSideBorders() const;
    //@}

    //*@name size grip
    //@{
    void createSizeGrip();
    void deleteSizeGrip();
    SizeGrip *sizeGrip() const
    {
        return m_sizeGrip;
    }
    //@}

    void setScaledTitleBarTopBottomMargins();
    void setScaledTitleBarSideMargins();
    void setAddedTitleBarOpacity();
    bool isOpaqueTitleBar();
    int titleBarSeparatorHeight() const;
    qreal devicePixelRatio(QPainter *painter) const;

    //* button heights
    void calculateButtonHeights();

    //* override thin window outline colour from button colour animation update
    void updateOverrideOutlineFromButtonAnimationState();

    InternalSettingsPtr m_internalSettings;
    KDecoration2::DecorationButtonGroup *m_leftButtons = nullptr;
    KDecoration2::DecorationButtonGroup *m_rightButtons = nullptr;

    //* size grip widget
    SizeGrip *m_sizeGrip = nullptr;

    //* active state change animation
    QVariantAnimation *m_animation;
    QVariantAnimation *m_shadowAnimation;
    QVariantAnimation *m_overrideOutlineFromButtonAnimation;

    //* active state change opacity
    qreal m_opacity = 0;
    //* shadow change opacity
    qreal m_shadowOpacity = 0;
    //* overridden thin window outline change opacity
    qreal m_overrideOutlineAnimationProgress = 0;

    //* tilebar main state opacity
    qreal m_addedTitleBarOpacityActive = 1;
    qreal m_addedTitleBarOpacityInactive = 1;

    //* frame corner radius, scaled according to smallspacing
    qreal m_scaledCornerRadius = 3.0;

    bool m_tabletMode = false;

    // QColor m_maximizedWindowHighlight = QColor();

    //* titleBar top margin, scaled according to smallspacing
    int m_scaledTitleBarTopMargin = 1;
    //* titleBar bottom margin, scaled according to smallspacing
    int m_scaledTitleBarBottomMargin = 1;

    //* titleBar side margins, scaled according to smallspacing
    int m_scaledTitleBarLeftMargin = 1;
    int m_scaledTitleBarRightMargin = 1;

    //* Rectangular area of titlebar without clipped corners
    QRect m_titleRect;

    //* Exact titlebar path, with clipped rounded corners
    std::shared_ptr<QPainterPath> m_titleBarPath = std::make_shared<QPainterPath>();
    //* Exact window path, with clipped rounded corners
    std::shared_ptr<QPainterPath> m_windowPath = std::make_shared<QPainterPath>();

    qreal m_systemScaleFactor = 1.0;

    ButtonBackgroundType m_buttonBackgroundType = ButtonBackgroundType::Small;
    int m_smallButtonPaddedHeight = 20;
    int m_iconHeight = 18;
    int m_smallButtonBackgroundHeight = 18;

    bool m_colorSchemeHasHeaderColor = true;
    bool m_toolsAreaWillBeDrawn = true;

    //*colour to override thin window outline with, set from decoration button
    QColor m_thinWindowOutlineOverride = QColor();
    //*buffered existing thin window outline colours in case the above override colour is set (needed for animations)
    QColor m_originalThinWindowOutlineActivePreOverride = QColor();
    QColor m_originalThinWindowOutlineInactivePreOverride = QColor();
    //*flag to animate out an overridden thin window outline
    bool m_animateOutOverriddenThinWindowOutline = false;
};

bool Decoration::hasBorders() const
{
    if (m_internalSettings && m_internalSettings->mask() & BorderSize)
        return m_internalSettings->borderSize() > InternalSettings::BorderNoSides;
    else
        return settings()->borderSize() > KDecoration2::BorderSize::NoSides;
}

bool Decoration::hasNoBorders() const
{
    if (m_internalSettings && m_internalSettings->mask() & BorderSize)
        return m_internalSettings->borderSize() == InternalSettings::BorderNone;
    else
        return settings()->borderSize() == KDecoration2::BorderSize::None;
}

bool Decoration::hasNoSideBorders() const
{
    if (m_internalSettings && m_internalSettings->mask() & BorderSize)
        return m_internalSettings->borderSize() == InternalSettings::BorderNoSides;
    else
        return settings()->borderSize() == KDecoration2::BorderSize::NoSides;
}

bool Decoration::isMaximized() const
{
    auto c = client().toStrongRef();
    Q_ASSERT(c);
    return c->isMaximized() && !m_internalSettings->drawBorderOnMaximizedWindows();
}

bool Decoration::isMaximizedHorizontally() const
{
    auto c = client().toStrongRef();
    Q_ASSERT(c);
    return c->isMaximizedHorizontally() && !m_internalSettings->drawBorderOnMaximizedWindows();
}

bool Decoration::isMaximizedVertically() const
{
    auto c = client().toStrongRef();
    Q_ASSERT(c);
    return c->isMaximizedVertically() && !m_internalSettings->drawBorderOnMaximizedWindows();
}

bool Decoration::isLeftEdge() const
{
    auto c = client().toStrongRef();
    Q_ASSERT(c);
    return (c->isMaximizedHorizontally() || c->adjacentScreenEdges().testFlag(Qt::LeftEdge)) && !m_internalSettings->drawBorderOnMaximizedWindows();
}

bool Decoration::isRightEdge() const
{
    auto c = client().toStrongRef();
    Q_ASSERT(c);

    return (c->isMaximizedHorizontally() || c->adjacentScreenEdges().testFlag(Qt::RightEdge)) && !m_internalSettings->drawBorderOnMaximizedWindows();
}

bool Decoration::isTopEdge() const
{
    auto c = client().toStrongRef();
    Q_ASSERT(c);

    return (c->isMaximizedVertically() || c->adjacentScreenEdges().testFlag(Qt::TopEdge)) && !m_internalSettings->drawBorderOnMaximizedWindows();
}

bool Decoration::isBottomEdge() const
{
    auto c = client().toStrongRef();
    Q_ASSERT(c);

    return (c->isMaximizedVertically() || c->adjacentScreenEdges().testFlag(Qt::BottomEdge)) && !m_internalSettings->drawBorderOnMaximizedWindows();
}

bool Decoration::hideTitleBar() const
{
    auto c = client().toStrongRef();
    Q_ASSERT(c);
    return m_internalSettings->hideTitleBar() && !c->isShaded();
}

} // end Breeze namespace

#endif
