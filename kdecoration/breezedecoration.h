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

#include <KDecoration2/Decoration>
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationSettings>

#include <QPalette>
#include <QVariant>
#include <QVariantAnimation>
#include <QPainterPath>

class QVariantAnimation;

namespace KDecoration2
{
    class DecorationButton;
    class DecorationButtonGroup;
}

namespace Breeze
{
    class SizeGrip;
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
        { return m_internalSettings; }

        qreal animationsDuration() const
        { return m_animation->duration();}

        //* caption height
        int captionHeight() const;

        //* button height
        int buttonHeight() const;

        //*@name active state change animation
        //@{
        void setOpacity( qreal );

        qreal opacity() const
        { return m_opacity; }

        //@}

        //*@name colors
        //@{
        QColor titleBarColor() const;
        QColor outlineColor() const;
        QColor fontColor() const;
        //@}

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
        
        QPainterPath* titleBarPath(){ return &m_titleBarPath; }
        QPainterPath* windowPath(){ return &m_windowPath; }
        qreal systemScaleFactor(){ return m_systemScaleFactor; }
        

        public Q_SLOTS:
        void init() override;

        private Q_SLOTS:
        void reconfigure();
        void recalculateBorders();
        void updateButtonsGeometry();
        void updateButtonsGeometryDelayed();
        void updateTitleBar();
        void updateAnimationState();
        void updateSizeGripVisibility();
        void updateShadow();

        private:

        //* return the rect in which caption will be drawn
        QPair<QRect,Qt::Alignment> captionRect() const;

        void createButtons();
        void setWindowAndTitleBarGeometries();
        void paintTitleBar(QPainter *painter, const QRect &repaintRegion);
        QSharedPointer<KDecoration2::DecorationShadow> createShadowObject( const float strengthScale );
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
        SizeGrip* sizeGrip() const
        { return m_sizeGrip; }
        //@}
        
        int titleBarTopBottomMargins() const;
        void setTitleBarOpacity();

        InternalSettingsPtr m_internalSettings;
        KDecoration2::DecorationButtonGroup *m_leftButtons = nullptr;
        KDecoration2::DecorationButtonGroup *m_rightButtons = nullptr;

        //* size grip widget
        SizeGrip *m_sizeGrip = nullptr;

        //* active state change animation
        QVariantAnimation *m_animation;
        QVariantAnimation *m_shadowAnimation;

        //* active state change opacity
        qreal m_opacity = 0;
        qreal m_shadowOpacity = 0;
        
        //* tilebar main state opacity
        int m_titleBarOpacityActive = 255;
        int m_titleBarOpacityInactive = 255;
        
        //* frame corner radius, scaled according to DPI
        qreal m_scaledCornerRadius = 3;
        
        //* Rectangular area of titlebar without clipped corners
        QRect m_titleRect;
        
        //* Exact titlebar path, with clipped rounded corners
        QPainterPath m_titleBarPath;
        //* Exact window path, with clipped rounded corners
        QPainterPath m_windowPath;
        
        qreal m_systemScaleFactor = 1.0;

    };

    bool Decoration::hasBorders() const
    {
        if( m_internalSettings && m_internalSettings->mask() & BorderSize ) return m_internalSettings->borderSize() > InternalSettings::BorderNoSides;
        else return settings()->borderSize() > KDecoration2::BorderSize::NoSides;
    }

    bool Decoration::hasNoBorders() const
    {
        if( m_internalSettings && m_internalSettings->mask() & BorderSize ) return m_internalSettings->borderSize() == InternalSettings::BorderNone;
        else return settings()->borderSize() == KDecoration2::BorderSize::None;
    }

    bool Decoration::hasNoSideBorders() const
    {
        if( m_internalSettings && m_internalSettings->mask() & BorderSize ) return m_internalSettings->borderSize() == InternalSettings::BorderNoSides;
        else return settings()->borderSize() == KDecoration2::BorderSize::NoSides;
    }

    bool Decoration::isMaximized() const
    { return client().data()->isMaximized() && !m_internalSettings->drawBorderOnMaximizedWindows(); }

    bool Decoration::isMaximizedHorizontally() const
    { return client().data()->isMaximizedHorizontally() && !m_internalSettings->drawBorderOnMaximizedWindows(); }

    bool Decoration::isMaximizedVertically() const
    { return client().data()->isMaximizedVertically() && !m_internalSettings->drawBorderOnMaximizedWindows(); }

    bool Decoration::isLeftEdge() const
    { return (client().data()->isMaximizedHorizontally() || client().data()->adjacentScreenEdges().testFlag( Qt::LeftEdge ) ) && !m_internalSettings->drawBorderOnMaximizedWindows(); }

    bool Decoration::isRightEdge() const
    { return (client().data()->isMaximizedHorizontally() || client().data()->adjacentScreenEdges().testFlag( Qt::RightEdge ) ) && !m_internalSettings->drawBorderOnMaximizedWindows(); }

    bool Decoration::isTopEdge() const
    { return (client().data()->isMaximizedVertically() || client().data()->adjacentScreenEdges().testFlag( Qt::TopEdge ) ) && !m_internalSettings->drawBorderOnMaximizedWindows(); }

    bool Decoration::isBottomEdge() const
    { return (client().data()->isMaximizedVertically() || client().data()->adjacentScreenEdges().testFlag( Qt::BottomEdge ) ) && !m_internalSettings->drawBorderOnMaximizedWindows(); }

    bool Decoration::hideTitleBar() const
    { return m_internalSettings->hideTitleBar() && !client().data()->isShaded(); }

}

#endif
