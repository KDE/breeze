#ifndef breeze_h
#define breeze_h
/*************************************************************************
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>    *
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program; if not, write to the                         *
 * Free Software Foundation, Inc.,                                       *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 *************************************************************************/

#include "breezeconfiguration.h"

#include <QSharedPointer>
#include <QList>

namespace Breeze
{

    //*@name convenience typedef
    //@{

    #if QT_VERSION >= 0x050000
    //* scoped pointer convenience typedef
    template <typename T> using WeakPointer = QPointer<T>;
    #else
    //* scoped pointer convenience typedef
    template <typename T> using WeakPointer = QWeakPointer<T>;
    #endif

    //* scoped pointer convenience typedef
    template <typename T> using ScopedPointer = QScopedPointer<T, QScopedPointerPodDeleter>;

    //* disable QStringLiteral for older Qt version
    #if QT_VERSION < 0x050000
    using QStringLiteral = QString;
    #endif

    //@}

    //* this should move to some global declaration
    typedef QSharedPointer<Configuration> ConfigurationPtr;
    typedef QList<ConfigurationPtr> ConfigurationList;
    typedef QListIterator<ConfigurationPtr> ConfigurationListIterator;

    //* buttons
    enum ButtonType {
        ButtonHelp=0,
        ButtonMax,
        ButtonMin,
        ButtonClose,
        ButtonMenu,
        ButtonSticky,
        ButtonAbove,
        ButtonBelow,
        ButtonShade,
        ButtonApplicationMenu,
        ButtonTypeCount,

        // Close only one tab
        ButtonItemClose=100,

        // shows the window menu for one tab
        ButtonItemMenu

    };

    //* buttons status
    enum ButtonStatus {
        Normal = 0,
        Hovered = 1<<0,
        Pressed = 1<<1
    };

    Q_DECLARE_FLAGS(ButtonState, ButtonStatus)

    //* metrics
    enum Metrics
    {

        //* corner radius
        Frame_FrameRadius = 3,

        //* top title bar edge
        TitleBar_TopMargin = 5,
        TitleBar_BottomMargin = 5,
        TitleBar_SideMargin = 5,
        TitleBar_ButtonSpacing = 3,

        // shadow dimensions
        Shadow_Size = 20,
        Shadow_Offset = 6,
        Shadow_Overlap = 2

    };

    //* animation type
    enum AnimationType
    {
        AnimationNone = 0,
        AnimationEnter = 1<<0,
        AnimationMove = 1<<1,
        AnimationLeave = 1<<2,
        AnimationSameTarget = 1<<3
    };

    Q_DECLARE_FLAGS(AnimationTypes, AnimationType)

    //* corners
    enum Corner
    {
        CornerTopLeft = 0x1,
        CornerTopRight = 0x2,
        CornerBottomLeft = 0x4,
        CornerBottomRight = 0x8,
        CornersTop = CornerTopLeft|CornerTopRight,
        CornersBottom = CornerBottomLeft|CornerBottomRight,
        CornersLeft = CornerTopLeft|CornerBottomLeft,
        CornersRight = CornerTopRight|CornerBottomRight,
        AllCorners = CornerTopLeft|CornerTopRight|CornerBottomLeft|CornerBottomRight
    };

    Q_DECLARE_FLAGS( Corners, Corner );

}


Q_DECLARE_OPERATORS_FOR_FLAGS(Breeze::AnimationTypes)
Q_DECLARE_OPERATORS_FOR_FLAGS(Breeze::Corners);

#endif
