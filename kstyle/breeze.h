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

#include <QFlags>
#include <QPointer>
#include <QScopedPointer>
#include <QWeakPointer>

namespace Breeze
{

    //*@name convenience typedef
    //@{

    //* scoped pointer convenience typedef
    template <typename T> using WeakPointer = QPointer<T>;

    //* scoped pointer convenience typedef
    template <typename T> using ScopedPointer = QScopedPointer<T, QScopedPointerPodDeleter>;

    //@}

    //* metrics
    enum Metrics
    {

        // frames
        Frame_FrameWidth = 3,
        Frame_FrameRadius = 4,

        // layout
        Layout_TopLevelMarginWidth = 10,
        Layout_ChildMarginWidth = 6,
        Layout_DefaultSpacing = 6,

        // line editors
        LineEdit_FrameWidth = 4,

        // menu items
        Menu_FrameWidth = 0,
        MenuItem_MarginWidth = 5,
        MenuItem_MarginHeight = 3,
        MenuItem_ItemSpacing = 20,
        MenuItem_AcceleratorSpace = 16,
        MenuButton_IndicatorWidth = 20,

        // combobox
        ComboBox_FrameWidth = 4,

        // spinbox
        SpinBox_FrameWidth = LineEdit_FrameWidth,
        SpinBox_ArrowButtonWidth = 15,

        // groupbox title margin
        GroupBox_TitleMarginWidth = 4,

        // buttons
        Button_MinWidth = 80,
        Button_MarginWidth = 3,
        Button_ItemSpacing = 2,

        // tool buttons
        ToolButton_MarginWidth = 3,
        ToolButton_ItemSpacing = 2,
        ToolButton_InlineIndicatorWidth = 8,

        // checkboxes and radio buttons
        CheckBox_Size = 20,
        CheckBox_FocusMarginWidth = 2,
        CheckBox_ItemSpacing = 4,

        // menubar items
        MenuBarItem_MarginWidth = 10,
        MenuBarItem_MarginHeight = 8,

        // scrollbars
        ScrollBar_Extend = 21,
        ScrollBar_SliderWidth = 6,
        ScrollBar_MinSliderHeight = 15,
        ScrollBar_NoButtonHeight = (ScrollBar_Extend-ScrollBar_SliderWidth)/2,
        ScrollBar_SingleButtonHeight = ScrollBar_Extend,
        ScrollBar_DoubleButtonHeight = 2*ScrollBar_Extend,

        // toolbars
        ToolBar_FrameWidth = 2,
        ToolBar_HandleExtent = 10,
        ToolBar_HandleWidth = 6,
        ToolBar_SeparatorWidth = 8,
        ToolBar_ExtensionWidth = 15,
        ToolBar_ItemSpacing = 0,

        // progressbars
        ProgressBar_BusyIndicatorSize = 14,
        ProgressBar_Thickness = 6,
        ProgressBar_ItemSpacing = 4,

        // mdi title bar
        TitleBar_MarginWidth = 4,

        // sliders
        Slider_TickLength = 8,
        Slider_TickMarginWidth = 2,
        Slider_GrooveThickness = 6,
        Slider_ControlThickness = 15,

        // tabbar
        TabBar_TabMarginHeight = 4,
        TabBar_TabMarginWidth = 8,
        TabBar_TabMinWidth = 80,
        TabBar_TabMinHeight = 18,
        TabBar_TabItemSpacing = 8,
        TabBar_TabOverlap = 1,
        TabBar_BaseOverlap = 2,

        // tab widget
        TabWidget_MarginWidth = 4,

        // toolbox
        ToolBox_TabMinWidth = 80,
        ToolBox_TabItemSpacing = 4,
        ToolBox_TabMarginWidth = 8,

        // tooltips
        ToolTip_FrameWidth = 3,

        // list headers
        Header_MarginWidth = 6,
        Header_ItemSpacing = 4,
        Header_ArrowSize = 8,

        // tree view
        ItemView_ArrowSize = 8,
        ItemView_ItemMarginWidth = 3,
        SidePanel_ItemMarginWidth = 4,

        // splitter
        Splitter_SplitterWidth = 1,

        // shadow dimensions
        Shadow_Overlap = 2

    };

    //* standard pen widths
    namespace PenWidth
    {
        /* Using 1 instead of slightly more than 1 causes symbols drawn with
         * pen strokes to look skewed. The exact amount added does not matter
         * as long as it isn't too visible.
         */
        // The standard pen stroke width for symbols.
        static constexpr qreal Symbol = 1.01;

        // The standard pen stroke width for frames.
        static constexpr int Frame = 1;

        // The standard pen stroke width for shadows.
        static constexpr int Shadow = 1;
        
        // A value for pen width arguments to make it clear that there is no pen stroke
        static constexpr int NoPen = 0;
    }
    
    //* animation mode
    enum AnimationMode
    {
        AnimationNone = 0,
        AnimationHover = 0x1,
        AnimationFocus = 0x2,
        AnimationEnable = 0x4,
        AnimationPressed = 0x8
    };

    Q_DECLARE_FLAGS(AnimationModes, AnimationMode)

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

    Q_DECLARE_FLAGS( Corners, Corner )

    //* sides
    enum Side
    {
        SideLeft = 0x1,
        SideTop = 0x2,
        SideRight = 0x4,
        SideBottom = 0x8,
        AllSides = SideLeft|SideTop|SideRight|SideBottom
    };

    Q_DECLARE_FLAGS( Sides, Side )

    //* checkbox state
    enum CheckBoxState
    {
        CheckOff,
        CheckPartial,
        CheckOn,
        CheckAnimated
    };

    //* radio button state
    enum RadioButtonState
    {
        RadioOff,
        RadioOn,
        RadioAnimated
    };

    //* arrow orientation
    enum ArrowOrientation
    {
        ArrowNone,
        ArrowUp,
        ArrowDown,
        ArrowLeft,
        ArrowRight,
        ArrowDown_Small,
    };

    //* button type
    enum ButtonType
    {
        ButtonClose,
        ButtonMaximize,
        ButtonMinimize,
        ButtonRestore
    };

}

Q_DECLARE_OPERATORS_FOR_FLAGS( Breeze::AnimationModes )
Q_DECLARE_OPERATORS_FOR_FLAGS( Breeze::Corners )
Q_DECLARE_OPERATORS_FOR_FLAGS( Breeze::Sides )

#endif
