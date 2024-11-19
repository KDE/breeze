/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "breezedecoration.h"
#include <KDecoration3/DecorationButton>

#include <QHash>
#include <QImage>

class QVariantAnimation;

namespace Breeze
{
class Button : public KDecoration3::DecorationButton
{
    Q_OBJECT

public:
    //* constructor
    explicit Button(QObject *parent, const QVariantList &args);

    //* destructor
    virtual ~Button() = default;

    //* button creation
    static Button *create(KDecoration3::DecorationButtonType type, KDecoration3::Decoration *decoration, QObject *parent);

    //* render
    void paint(QPainter *painter, const QRectF &repaintRegion) override;

    //* padding
    void setPadding(const QMargins &value)
    {
        m_padding = value;
    }

    //* left padding, for rendering
    void setLeftPadding(qreal value)
    {
        m_padding.setLeft(value);
    }

    //* right padding, for rendering
    void setRightPadding(qreal value)
    {
        m_padding.setRight(value);
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

    void setPreferredSize(const QSizeF &size)
    {
        m_preferredSize = size;
    }

    QSizeF preferredSize() const
    {
        return m_preferredSize;
    }

private Q_SLOTS:

    //* apply configuration changes
    void reconfigure();

    //* animation state
    void updateAnimationState(bool);

private:
    //* private constructor
    explicit Button(KDecoration3::DecorationButtonType type, Decoration *decoration, QObject *parent = nullptr);

    //* draw button icon
    void drawIcon(QPainter *) const;

    //*@name colors
    //@{
    QColor foregroundColor() const;
    QColor backgroundColor() const;
    //@}

    //* active state change animation
    QVariantAnimation *m_animation;

    //* padding (for rendering)
    QMargins m_padding;

    //* implicit size
    QSizeF m_preferredSize;

    //* active state change opacity
    qreal m_opacity = 0;
};

} // namespace
