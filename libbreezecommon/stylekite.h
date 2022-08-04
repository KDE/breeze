#ifndef BREEZE_STYLEKITE_H
#define BREEZE_STYLEKITE_H

/*
 * SPDX-FileCopyrightText: 2022 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "renderdecorationbuttonicon.h"

#include <QPainter>

namespace Breeze
{

class RenderStyleKite18By18 : public RenderDecorationButtonIcon18By18
{
public:
    RenderStyleKite18By18(QPainter *painter,
                          const bool fromKstyle,
                          const bool boldButtonIcons,
                          const qreal devicePixelRatio,
                          const QPointF &deviceOffsetTitleBarTopLeftToIconTopLeft)
        : RenderDecorationButtonIcon18By18(painter, fromKstyle, boldButtonIcons, devicePixelRatio, deviceOffsetTitleBarTopLeftToIconTopLeft){};

    void renderCloseIcon() override;
    void renderMaximizeIcon() override;
    void renderRestoreIcon() override;
    void renderMinimizeIcon() override;
    void renderKeepBehindIcon() override;
    void renderKeepInFrontIcon() override;
    void renderContextHelpIcon() override;

private:
};

}

#endif
