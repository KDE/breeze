/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef breeze_helper_h
#define breeze_helper_h

#include "breeze.h"
#include "breezemetrics.h"
#include "breezeanimationdata.h"
#include "breezesettings.h"
#include "config-breeze.h"

#include <KColorScheme>
#include <KSharedConfig>
#include <KConfigWatcher>

#include <QToolBar>
#include <QPainterPath>
#include <QIcon>
#include <QWidget>

namespace Breeze
{

    //* breeze style helper class.
    /** contains utility functions used at multiple places in both breeze style and breeze window decoration */
    class Helper : public QObject
    {
        Q_OBJECT

        public:

        //* constructor
        explicit Helper( KSharedConfig::Ptr, QObject *parent = nullptr );

        //* destructor
        virtual ~Helper()
        {}

        //* load configuration
        virtual void loadConfig();

        //* pointer to shared config
        KSharedConfig::Ptr config() const;

        //* pointer to kdecoration config
        QSharedPointer<InternalSettings> decorationConfig() const;

        //*@name color utilities
        //@{

        //* add alpha channel multiplier to color
        QColor alphaColor( QColor color, qreal alpha ) const;

        //* mouse over color
        QColor hoverColor( const QPalette& palette ) const
        { return _viewHoverBrush.brush( palette ).color(); }

        //* focus color
        QColor focusColor( const QPalette& palette ) const
        { return _viewFocusBrush.brush( palette ).color(); }

        //* mouse over color for buttons
        QColor buttonHoverColor( const QPalette& palette ) const
        { return _buttonHoverBrush.brush( palette ).color(); }

        //* focus color for buttons
        QColor buttonFocusColor( const QPalette& palette ) const
        { return _buttonFocusBrush.brush( palette ).color(); }

        //* negative text color (used for close button)
        QColor negativeText( const QPalette& palette ) const
        { return _viewNegativeTextBrush.brush( palette ).color(); }

        //* neutral text color
        QColor neutralText( const QPalette& palette ) const
        { return _viewNeutralTextBrush.brush( palette ).color(); }

        //* shadow
        QColor shadowColor( const QPalette& palette ) const
        { return alphaColor( palette.color( QPalette::Shadow ), 0.15 ); }

        //* titlebar color
        const QColor& titleBarColor( bool active ) const
        { return active ? _activeTitleBarColor:_inactiveTitleBarColor; }

        //* titlebar text color
        const QColor& titleBarTextColor( bool active ) const
        { return active ? _activeTitleBarTextColor : _inactiveTitleBarTextColor; }

        //* frame outline color, using animations
        QColor frameOutlineColor( const QPalette&, bool mouseOver = false, bool hasFocus = false, qreal opacity = AnimationData::OpacityInvalid, AnimationMode = AnimationNone ) const;

        //* focus outline color, using animations
        QColor focusOutlineColor( const QPalette& ) const;

        //* hover outline color, using animations
        QColor hoverOutlineColor( const QPalette& ) const;

        //* focus outline color, using animations
        QColor buttonFocusOutlineColor( const QPalette& ) const;

        //* hover outline color, using animations
        QColor buttonHoverOutlineColor( const QPalette& ) const;

        //* side panel outline color, using animations
        QColor sidePanelOutlineColor( const QPalette&, bool hasFocus = false, qreal opacity = AnimationData::OpacityInvalid, AnimationMode = AnimationNone ) const;

        //* frame background color
        QColor frameBackgroundColor( const QPalette& palette ) const
        { return frameBackgroundColor( palette, palette.currentColorGroup() ); }

        //* frame background color
        QColor frameBackgroundColor( const QPalette&, QPalette::ColorGroup ) const;

        //* arrow outline color
        QColor arrowColor( const QPalette&, QPalette::ColorGroup, QPalette::ColorRole ) const;

        //* arrow outline color
        QColor arrowColor( const QPalette& palette, QPalette::ColorRole role ) const
        { return arrowColor( palette, palette.currentColorGroup(), role ); }

        //* arrow outline color, using animations
        QColor arrowColor( const QPalette&, bool mouseOver, bool hasFocus, qreal opacity = AnimationData::OpacityInvalid, AnimationMode = AnimationNone ) const;

        //* button outline color, using animations
        QColor buttonOutlineColor( const QPalette&, bool mouseOver, bool hasFocus, qreal opacity = AnimationData::OpacityInvalid, AnimationMode = AnimationNone ) const;

        //* button panel color, using animations
        QPair<QColor,QColor> buttonBackgroundColor( const QPalette&, bool mouseOver, bool hasFocus, bool sunken, qreal opacity = AnimationData::OpacityInvalid, AnimationMode = AnimationNone ) const;

        //* tool button color
        QColor toolButtonColor( const QPalette&, bool mouseOver, bool hasFocus, bool sunken, qreal opacity = AnimationData::OpacityInvalid, AnimationMode = AnimationNone ) const;

        //* slider outline color, using animations
        QColor sliderOutlineColor( const QPalette&, bool mouseOver, bool hasFocus, qreal opacity = AnimationData::OpacityInvalid, AnimationMode = AnimationNone ) const;

        //* scrollbar handle color, using animations
        QColor scrollBarHandleColor( const QPalette&, bool mouseOver, bool hasFocus, qreal opacity = AnimationData::OpacityInvalid, AnimationMode = AnimationNone ) const;

        //* checkbox indicator, using animations
        QColor checkBoxIndicatorColor( const QPalette&, bool mouseOver, bool active, qreal opacity = AnimationData::OpacityInvalid, AnimationMode = AnimationNone ) const;

        //* separator color
        QColor separatorColor( const QPalette& ) const;

        //* merge active and inactive palettes based on ratio, for smooth enable state change transition
        QPalette disabledPalette( const QPalette&, qreal ratio ) const;

        //@}

        //*@name rendering utilities
        //@{

        //* debug frame
        void renderDebugFrame( QPainter*, const QRect& ) const;

        //* focus rect
        void renderFocusRect( QPainter*, const QRect&, const QColor&, const QColor& outline = QColor(), Sides = {} ) const;

        //* focus line
        void renderFocusLine( QPainter*, const QRect&, const QColor& ) const;

        //* generic frame
        void renderFrame( QPainter*, const QRect&, const QColor& color, const QColor& outline = QColor() ) const;

        //* side panel frame
        void renderSidePanelFrame( QPainter*, const QRect&, const QColor& outline, Side ) const;

        //* menu frame
        void renderMenuFrame( QPainter*, const QRect&, const QColor& color, const QColor& outline, bool roundCorners = true, bool isTopMenu = false ) const;

        //* button frame
        void renderButtonFrame( QPainter*, const QRect&, const QPair<QColor,QColor>& highlightAndBase, const QColor& outline, const QColor& shadow, bool focus, bool sunken ) const;

        //* toolbutton frame
        void renderToolButtonFrame( QPainter*, const QRect&, const QColor& color, bool sunken ) const;

        //* toolbutton frame
        void renderToolBoxFrame( QPainter*, const QRect&, int tabWidth, const QColor& color ) const;

        //* tab widget frame
        void renderTabWidgetFrame( QPainter*, const QRect&, const QColor& color, const QColor& outline, Corners ) const;

        //* selection frame
        void renderSelection( QPainter*, const QRect&, const QColor& ) const;

        //* separator
        void renderSeparator( QPainter*, const QRect&, const QColor&, bool vertical = false ) const;

        //* checkbox
        void renderCheckBoxBackground( QPainter*, const QRect&, const QPalette& palette, CheckBoxState state, bool neutalHighlight, qreal animation = AnimationData::OpacityInvalid ) const;

        //* checkbox
        void renderCheckBox( QPainter*, const QRect&, const QPalette& palette, bool mouseOver, CheckBoxState state, CheckBoxState target, bool neutalHighlight, qreal animation = AnimationData::OpacityInvalid, qreal hoverAnimation = AnimationData::OpacityInvalid ) const;

        //* radio button
        void renderRadioButtonBackground( QPainter*, const QRect&, const QPalette& palette, RadioButtonState state, bool neutalHighlight, qreal animation = AnimationData::OpacityInvalid ) const;

        //* radio button
        void renderRadioButton( QPainter*, const QRect&, const QPalette& palette, bool mouseOver, RadioButtonState state, bool neutalHighlight, qreal animation = AnimationData::OpacityInvalid, qreal hoverAnimation = AnimationData::OpacityInvalid ) const;

        //* slider groove
        void renderSliderGroove( QPainter*, const QRect&, const QColor& ) const;

        //* slider handle
        void renderSliderHandle( QPainter*, const QRect&, const QColor&, const QColor& outline, const QColor& shadow, bool sunken ) const;

        //* dial groove
        void renderDialGroove( QPainter*, const QRect&, const QColor& fg, const QColor& bg, qreal first, qreal last ) const;

        //* progress bar groove
        void renderProgressBarGroove( QPainter*, const QRect&, const QColor& fg, const QColor& bg) const;

        //* progress bar contents (animated)
        void renderProgressBarBusyContents( QPainter* painter, const QRect& rect, const QColor& first, const QColor& second, bool horizontal, bool reverse, int progress  ) const;

        //* scrollbar groove
        void renderScrollBarGroove( QPainter* painter, const QRect& rect, const QColor& color ) const
        { return renderScrollBarHandle( painter, rect, color ); }

        //* scrollbar handle
        void renderScrollBarHandle( QPainter*, const QRect&, const QColor& ) const;

        //* separator between scrollbar and contents
        void renderScrollBarBorder( QPainter*, const QRect&, const QColor& ) const;

        //* tabbar tab
        void renderTabBarTab( QPainter*, const QRect&, const QColor& color, const QColor& highlight, const QColor& outline, Corners, bool document, bool bottom ) const;
        // TODO(janet): document should be set based on whether or not we consider the
        // tab user-editable, but Qt apps often misuse or don't use documentMode property
        // so we're currently just always setting it to true for now

        //* generic arrow
        void renderArrow( QPainter*, const QRect&, const QColor&, ArrowOrientation ) const;

        //* generic button (for mdi decorations, tabs and dock widgets)
        void renderDecorationButton( QPainter*, const QRect&, const QColor&, ButtonType, bool inverted, bool paintBackground = false, const QColor& backgroundColor = QColor() ) const;

        //* generic shadow for rounded rectangles
        void renderRoundedRectShadow ( QPainter*, const QRectF&, const QColor&, qreal radius = Metrics::Frame_FrameRadius - 0.5 ) const;
        
        //* generic shadow for ellipses
        void renderEllipseShadow( QPainter*, const QRectF&, const QColor& ) const;
        
        //@}

        //*@name compositing utilities
        //@{

        //* true if style was compiled for and is running on X11
        static bool isX11();

        //* true if running on platform Wayland
        static bool isWayland();

        //* returns true if compositing is active
        bool compositingActive() const;

        //* returns true if a given widget supports alpha channel
        bool hasAlphaChannel( const QWidget* ) const;

        //* returns true if the tools area should be drawn
        bool shouldDrawToolsArea ( const QWidget* ) const;

        //@}

        //* return device pixel ratio for a given pixmap
        virtual qreal devicePixelRatio( const QPixmap& ) const;

        //* frame radius
        constexpr qreal frameRadius( const int penWidth = PenWidth::NoPen, const qreal bias = 0 ) const
        { return qMax( Metrics::Frame_FrameRadius - (0.5 * penWidth) + bias, 0.0 ); }
        
        //* frame radius with new pen width
        constexpr qreal frameRadiusForNewPenWidth( const qreal oldRadius, const int penWidth ) const
        { return qMax( oldRadius - (0.5 * penWidth), 0.0 ); }
        
        //* return a QRectF with the appropriate size for a rectangle with a pen stroke
        QRectF strokedRect( const QRectF &rect, const int penWidth = PenWidth::Frame ) const;
        
        //* return a QRectF with the appropriate size for a rectangle with a pen stroke
        QRectF strokedRect( const QRect &rect, const int penWidth = PenWidth::Frame ) const;
        
        QPixmap coloredIcon(const QIcon &icon, const QPalette& palette, const QSize &size,
                            QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);

        protected:

        //* return rounded path in a given rect, with only selected corners rounded, and for a given radius
        QPainterPath roundedPath( const QRectF&, Corners, qreal ) const;

        private:

        //* configuration
        KSharedConfig::Ptr _config;

        //* KWin configuration
        KSharedConfig::Ptr _kwinConfig;

        //* decoration configuration
        QSharedPointer<InternalSettings> _decorationConfig;

        //*@name brushes
        //@{
        KStatefulBrush _viewFocusBrush;
        KStatefulBrush _viewHoverBrush;
        KStatefulBrush _buttonFocusBrush;
        KStatefulBrush _buttonHoverBrush;
        KStatefulBrush _viewNegativeTextBrush;
        KStatefulBrush _viewNeutralTextBrush;
        //@}

        //*@name windeco colors
        //@{
        QColor _activeTitleBarColor;
        QColor _activeTitleBarTextColor;
        QColor _inactiveTitleBarColor;
        QColor _inactiveTitleBarTextColor;
        //@}

        mutable bool _cachedAutoValid = false;

        friend class ToolsAreaManager;

    };

}

#endif
