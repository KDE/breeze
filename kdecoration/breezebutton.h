/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "breezedecoration.h"
#include <KDecoration2/DecorationButton>

#include <QHash>
#include <QImage>

class QVariantAnimation;

namespace Breeze
{
class Button : public KDecoration2::DecorationButton
{
    Q_OBJECT

public:
    //* constructor
    explicit Button(QObject *parent, const QVariantList &args);

    //* destructor
    virtual ~Button() = default;

    //* button creation
    static Button *create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent);

    //* render
    virtual void paint(QPainter *painter, const QRectF &repaintRegion) override;

    //* flag
    enum Flag {
        FlagNone,
        FlagStandalone,
        FlagFirstInList,
        FlagLastInList,
    };

    //* flag
    void setFlag(Flag value)
    {
        m_flag = value;
    }

    //* standalone buttons
    bool isStandAlone() const
    {
        return m_flag == FlagStandalone;
    }

    //* offset
    void setOffset(const QPointF &value)
    {
        m_offset = value;
    }

    //* horizontal offset, for rendering
    void setHorizontalOffset(qreal value)
    {
        m_offset.setX(value);
    }

    //* vertical offset, for rendering
    void setVerticalOffset(qreal value)
    {
        m_offset.setY(value);
    }

    //* set icon size
    void setIconSize(const QSize &value)
    {
        m_iconSize = value;
    }

    //*@name active state change animation
    //@{
    void setOpacity(qreal value)
    {
        if (m_opacity == value) {
            return;
        }
        m_opacity = value;
        update();
    }

    qreal opacity() const
    {
        return m_opacity;
    }

    //@}

private Q_SLOTS:

    //* apply configuration changes
    void reconfigure();

    //* animation state
    void updateAnimationState(bool);

private:
    //* private constructor
    explicit Button(KDecoration2::DecorationButtonType type, Decoration *decoration, QObject *parent = nullptr);

    //* draw button icon
    void drawIcon(QPainter *) const;

    //*@name colors
    //@{
    QColor foregroundColor() const;
    QColor backgroundColor() const;
    //@}

    Flag m_flag = FlagNone;

    //* active state change animation
    QVariantAnimation *m_animation;

    //* vertical offset (for rendering)
    QPointF m_offset;

    //* icon size
    QSize m_iconSize;

    //* active state change opacity
    qreal m_opacity = 0;
};

} // namespace
