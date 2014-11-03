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

#include "breezeclient.h"
#include "breezeclient.moc"

#include "breezebutton.h"
#include "breezesizegrip.h"

#include <cassert>
#include <cmath>

#include <KLocalizedString>
#include <KColorUtils>
#include <KStyle>

#include <QApplication>
#include <QDrag>
#include <QLabel>
#include <QPainter>
#include <QBitmap>
#include <QObjectList>
#include <QMimeData>

namespace Breeze
{

    //___________________________________________
    Client::Client(KDecorationBridge *b, Factory *f):
        ParentDecorationClass(b, f),
        _factory( f ),
        _sizeGrip( nullptr ),
        _animation( new Animation( 200, this ) ),
        _opacity(0),
        _initialized( false ),
        _mouseButton( Qt::NoButton ),
        _itemData( this ),
        _sourceItem( -1 ),
        _lastTabId( -1 ),
        _shadowAtom( 0 )
    {
        #if !BREEZE_USE_KDE4
        connect(options(), &KDecorationOptions::compositingChanged, this, &Client::updateCompositing);
        connect(options(), &KDecorationOptions::configChanged, this, &Client::updateConfig);
        #endif
    }

    //___________________________________________
    Client::~Client()
    {

        // delete sizegrip if any
        if( hasSizeGrip() ) deleteSizeGrip();

    }

    //___________________________________________
    QString Client::visibleName() const
    { return i18n("Breeze"); }

    //___________________________________________
    void Client::init()
    {

        // make sure valid configuration is set
        if( !_configuration ) _configuration = _factory->configuration( *this );

        ParentDecorationClass::init();

        widget()->setAttribute(Qt::WA_NoSystemBackground );
        widget()->setAutoFillBackground( false );
        widget()->setAcceptDrops( true );

        // setup glow animation
        _animation->setStartValue( 0 );
        _animation->setEndValue( 1.0 );
        _animation->setTargetObject( this );
        _animation->setPropertyName( "opacity" );
        _animation->setEasingCurve( QEasingCurve::InOutQuad );

        // lists
        connect( _itemData.animation().data(), SIGNAL(finished()), this, SLOT(clearTargetItem()) );

        // in case of preview, one wants to make the label used
        // for the central widget transparent. This allows one to have
        // the correct background (with gradient) rendered
        // Remark: this is minor (and safe) a hack.
        // This should be moved upstream (into kwin/lib/kdecoration)
        if( isPreview() )
        {

            QList<QLabel*> children( widget()->findChildren<QLabel*>() );
            foreach( QLabel* widget, children )
            { widget->setAutoFillBackground( false ); }

        }

        setAlphaEnabled(!isMaximized());

        _initialized = true;

        // first reset is needed to store Breeze configuration
        updateConfig();

    }

    //___________________________________________
    void Client::updateCompositing()
    {
        // update window mask when compositing is changed
        if( !_initialized ) return;
        updateWindowShape();
        widget()->update();
        updateConfig();
    }

    //___________________________________________
    void Client::updateConfig()
    {
        if( !_initialized ) return;

        _configuration = _factory->configuration( *this );

        // glow animations
        _animation->setDuration( _configuration->animationsDuration() );

        // tabs
        _itemData.setAnimationsEnabled( animationsEnabled() );
        _itemData.animation().data()->setDuration( _configuration->animationsDuration() );

        // should also update animations for buttons
        resetButtons();

        // also reset tab buttons
        for( int index = 0; index < _itemData.count(); index++ )
        {
            ClientGroupItemData& item( _itemData[index] );
            if( item._closeButton ) { item._closeButton.data()->reset(0); }
        }

        // reset tab geometry
        _itemData.setDirty( true );

        // handle size grip
        if( _configuration->frameBorder() == Configuration::BorderNone )
        {

            if( !hasSizeGrip() ) createSizeGrip();

        } else if( hasSizeGrip() ) deleteSizeGrip();

        // needs to remove shadow property on window since shadows are handled by the decoration
        removeShadowHint();

    }

    //___________________________________________
    bool Client::decorationBehaviour(DecorationBehaviour behaviour) const
    {
        switch (behaviour)
        {

            case DB_MenuClose:
            return _configuration->closeWindowFromMenuButton();

            case DB_WindowMask:
            return false;

            default:
            return ParentDecorationClass::decorationBehaviour(behaviour);
        }
    }

    //_________________________________________________________
    KCommonDecorationButton *Client::createButton(::ButtonType type)
    {

        switch (type) {

            case MenuButton:
            return new Button(*this, i18n("Window Actions Menu"), ButtonMenu);

            case AppMenuButton:
            return new Button(*this, i18n("Application Menu"), ButtonApplicationMenu);

            case HelpButton:
            return new Button(*this, i18n("Help"), ButtonHelp);

            case MinButton:
            return new Button(*this, i18n("Minimize"), ButtonMin);

            case MaxButton:
            return new Button(*this, i18n("Maximize"), ButtonMax);

            case CloseButton:
            return new Button(*this, i18n("Close"), ButtonClose);

            case AboveButton:
            return new Button(*this, i18n("Keep Above Others"), ButtonAbove);

            case BelowButton:
            return new Button(*this, i18n("Keep Below Others"), ButtonBelow);

            case OnAllDesktopsButton:
            return new Button(*this, i18n("On All Desktops"), ButtonSticky);

            case ShadeButton:
            return new Button(*this, i18n("Shade Button"), ButtonShade);

            default: break;

        }

        return nullptr;

    }

    //_________________________________________________________
    QRegion Client::calcMask( void ) const
    {

        if( isMaximized() ) { return widget()->rect(); }
        const QRect frame( widget()->rect().adjusted(
            layoutMetric( LM_OuterPaddingLeft ), layoutMetric( LM_OuterPaddingTop ),
            -layoutMetric( LM_OuterPaddingRight ), -layoutMetric( LM_OuterPaddingBottom ) ) );

        QRegion mask;
        if( _configuration->frameBorder() == Configuration::BorderNone && !isShade() )
        {

            if( hideTitleBar() ) mask = QRegion();
            else if( compositingActive() ) mask = QRegion();
            else mask = QRegion( helper().roundedMask( frame.size(), CornerTopLeft|CornerTopRight ) ).translated( frame.topLeft() );

        } else {

            if( compositingActive() ) mask = QRegion();
            else mask = QRegion( helper().roundedMask( frame.size(), AllCorners ) ).translated( frame.topLeft() );

        }

        return mask;

    }

    //___________________________________________
    int Client::layoutMetric(LayoutMetric lm, bool respectWindowState, const KCommonDecorationButton *btn) const
    {

        const bool maximized( isMaximized() );
        const bool shaded( isShade() );
        const int frameBorder( this->frameBorder() );
        const int buttonSize( hideTitleBar() ? 0 : this->buttonSize() );
        const int shadowSize( Metrics::Shadow_Size - Metrics::Shadow_Overlap );

        switch (lm)
        {
            case LM_BorderLeft:
            case LM_BorderRight:
            {
                int border( frameBorder );
                if( respectWindowState && maximized )
                {

                    border = 0;

                } else if( _configuration->frameBorder() < Configuration::BorderTiny ) {

                    border = 0;

                } else if( !compositingActive() && _configuration->frameBorder() == Configuration::BorderTiny ) {

                    border = qMax( frameBorder, 3 );

                }

                return border;
            }

            case LM_BorderBottom:
            {
                int border( frameBorder );
                if( (respectWindowState && maximized) || shaded )
                {

                    border = 0;

                } else if( _configuration->frameBorder() >= Configuration::BorderNoSide ) {

                    // for tiny border, the convention is to have a larger bottom area in order to
                    // make resizing easier
                    border = qMax(frameBorder, 4);

                } else if( _configuration->frameBorder() < Configuration::BorderTiny ) {

                    border = 0;

                } else if( !compositingActive() && _configuration->frameBorder() == Configuration::BorderTiny ) {

                    border = qMax( frameBorder, 3 );

                }

                return border;
            }

            case LM_TitleEdgeTop:
            {
                int border = 0;
                if( _configuration->frameBorder() == Configuration::BorderNone && hideTitleBar() )
                {

                    border = 0;

                } else if( !( respectWindowState && maximized )) {

                    border = Metrics::TitleBar_TopMargin;

                }

                return border;

            }

            case LM_TitleEdgeBottom:
            {
                return Metrics::TitleBar_BottomMargin;
            }

            case LM_TitleEdgeLeft:
            case LM_TitleEdgeRight:
            {
                int border = 0;
                if( !(respectWindowState && maximized) )
                { border = 4; }

                return border;

            }

            case LM_TitleBorderLeft:
            case LM_TitleBorderRight:
            {
                return Metrics::TitleBar_SideMargin;
            }

            case LM_ButtonWidth:
            case LM_ButtonHeight:
            {
                return buttonSize;
            }

            case LM_TitleHeight:
            {
                if( hideTitleBar() ) return 0;
                else {
                    const int titleHeight = QFontMetrics(options()->font(true)).height();
                    return qMax(buttonSize, titleHeight);
                }
            }

            case LM_ButtonSpacing:
            return Metrics::TitleBar_ButtonSpacing;

            case LM_ButtonMarginTop:
            return 0;

            // outer margin for shadow
            // TODO: implement light source
            case LM_OuterPaddingLeft: return maximized ? 0 : (shadowSize - Metrics::Shadow_Offset);
            case LM_OuterPaddingRight: return maximized ? 0 : shadowSize;
            case LM_OuterPaddingTop: return maximized ? 0 : (shadowSize - Metrics::Shadow_Offset);
            case LM_OuterPaddingBottom: return maximized ? 0: shadowSize;

            default:
            return ParentDecorationClass::layoutMetric(lm, respectWindowState, btn);
        }

    }

    //_________________________________________________________
    QRect Client::defaultTitleRect( void ) const
    {

        QRect titleRect( this->titleRect().adjusted( 0, -layoutMetric( LM_TitleEdgeTop ), 0, 1 ) );

        // buttons are properly accounted for in titleBoundingRect method
        titleRect.setLeft( widget()->rect().left() + layoutMetric( LM_OuterPaddingLeft ) );
        titleRect.setRight( widget()->rect().right() - layoutMetric( LM_OuterPaddingRight ) );

        return titleRect;

    }

    //_________________________________________________________
    QRect Client::titleBoundingRect( const QFont& font, QRect rect, const QString& caption ) const
    {

        // get title bounding rect
        QRect boundingRect( QFontMetrics( font ).boundingRect( rect, titleAlignment() | Qt::AlignVCenter, caption ) );

        // adjust to make sure bounding rect
        // 1/ has same vertical alignment as original titleRect
        // 2/ does not exceeds available horizontal space
        boundingRect.setTop( rect.top() );
        boundingRect.setBottom( rect.bottom() );

        // check bounding rect against input rect
        boundRectTo( boundingRect, rect );

        if( _configuration->titleAlignment() == Configuration::AlignCenterFullWidth )
        {

            /*
            check bounding rect against max available space, for buttons
            this is not needed if centerTitleOnFullWidth flag is set to false,
            because it was already done before calling titleBoundingRect
            */
            boundRectTo( boundingRect, titleRect() );

        }

        return boundingRect;

    }

    //_________________________________________________________
    void Client::boundRectTo( QRect& rect, const QRect& bound ) const
    {

        if( bound.left() > rect.left() )
        {
            rect.moveLeft( bound.left() );
            if( bound.right() < rect.right() )
            { rect.setRight( bound.right() ); }

        } else if( bound.right() < rect.right() ) {

            rect.moveRight( bound.right() );
            if( bound.left() > rect.left() )
            { rect.setLeft( bound.left() ); }

        }

        return;
    }

    //_________________________________________________________
    void Client::clearTargetItem( void )
    {

        if( _itemData.animationType() == AnimationLeave )
        { _itemData.setDirty( true ); }

    }

    //_________________________________________________________
    void Client::updateItemBoundingRects( bool alsoUpdate )
    {

        // make sure items are not animated
        _itemData.animate( AnimationNone );

        // maximum available space
        const QRect titleRect( this->titleRect() );

        // get tabs
        const int items( tabCount() );

        // make sure item data have the correct number of items
        while( _itemData.count() < items ) _itemData.append( ClientGroupItemData() );
        while( _itemData.count() > items )
        {
            if( _itemData.back()._closeButton ) delete _itemData.back()._closeButton.data();
            _itemData.pop_back();
        }

        assert( !_itemData.isEmpty() );

        // create buttons
        if( _itemData.count() == 1 )
        {

            // remove button
            if( _itemData.front()._closeButton )
            { delete _itemData.front()._closeButton.data(); }

            // set active rect
            _itemData.front()._activeRect = titleRect.adjusted( 0, -layoutMetric( LM_TitleEdgeTop ), 0, 1 );

        } else {

            int left( titleRect.left() );
            const int width( titleRect.width()/items );
            for( int index = 0; index < _itemData.count(); index++ )
            {

                ClientGroupItemData& item(_itemData[index]);

                // make sure button exists
                if( !item._closeButton )
                {
                    item._closeButton = ClientGroupItemData::ButtonPointer( new Button( *this, QStringLiteral("Close this tab"), ButtonItemClose ) );
                    item._closeButton.data()->show();
                    item._closeButton.data()->installEventFilter( this );
                }

                // set active rect
                QRect local(  QPoint( left, titleRect.top() ), QSize( width, titleRect.height() ) );
                local.adjust( 0, -layoutMetric( LM_TitleEdgeTop ), 0, 1 );
                item._activeRect = local;
                left += width;

            }

        }

        if( _itemData.count() == 1 )
        {

            _itemData.front().reset( defaultTitleRect() );

        } else {

            for( int index = 0; index < _itemData.count(); index++ )
            { _itemData[index].reset( _itemData[index]._activeRect ); }

        }

        // button activity
        _itemData.updateButtonActivity( currentTabId() );

        // reset buttons location
        _itemData.updateButtons( alsoUpdate );
        _itemData.setDirty( false );

        return;

    }

    //_______________________________________________
    QColor Client::foregroundColor( void ) const
    {
        if( isAnimated() )
        {
            return KColorUtils::mix( foregroundColor( false ), foregroundColor( true ), opacity() );

        } else return foregroundColor( isActive() );
    }

    //_______________________________________________
    QColor Client::backgroundColor( void ) const
    {
        if( isAnimated() )
        {
            return KColorUtils::mix( backgroundColor( false ), backgroundColor( true ), opacity() );

        } else return backgroundColor( isActive() );
    }

    //_______________________________________________
    QColor Client::outlineColor( void ) const
    {
        if( isAnimated() )
        {

            return helper().alphaColor( outlineColor( true ), opacity() );

        } else return outlineColor( isActive() );
    }

    //___________________________________________________
    QColor Client::foregroundColor( bool active ) const
    { return options()->color( KDecorationDefines::ColorFont, active ); }

    //___________________________________________________
    QColor Client::backgroundColor( bool active ) const
    { return options()->color( KDecorationDefines::ColorTitleBar, active ); }

    //___________________________________________________
    QColor Client::outlineColor( bool active ) const
    {
        if( isShade() || !active ) return QColor();
        else {

            // palette
            #if BREEZE_USE_KDE4
            const QPalette palette = widget()->palette();
            #else
            const QPalette palette = ParentDecorationClass::palette();
            #endif

            return helper().focusColor( palette );
        }

    }

    //_________________________________________________________
    void Client::renderTitleText( QPainter* painter, const QRect& rect, const QColor& color ) const
    { renderTitleText( painter, rect, caption(), color ); }

    //_______________________________________________________________________
    void Client::renderTitleText( QPainter* painter, const QRect& rect, const QString& caption, const QColor& color, bool elide ) const
    {

        const Qt::Alignment alignment( titleAlignment() | Qt::AlignVCenter );
        const QString local( elide ? QFontMetrics( painter->font() ).elidedText( caption, Qt::ElideRight, rect.width() ):caption );

        painter->setPen( color );
        painter->drawText( rect, alignment, local );

    }

    //_______________________________________________________________________
    void Client::renderShadow( QPainter* painter, const QRect& rect ) const
    { _factory->shadowTiles().render( rect, painter ); }

    //_______________________________________________________________________
    void Client::renderBackground( QPainter* painter, const QRect& rect, bool isShade ) const
    {
        painter->save();

        const QColor background( backgroundColor() );
        painter->setPen( Qt::NoPen );

        // window background
        if( isShade )
        {
            // path
            QPainterPath path( helper().roundedPath( rect ) );
            QLinearGradient gradient( rect.topLeft(), rect.bottomLeft() );
            gradient.setColorAt( 0, background.lighter( isActive() ? 120:100 ) );
            gradient.setColorAt( 0.8, background );
            painter->setBrush( gradient );
            painter->drawPath( path );

        } else {

            // path
            QPainterPath path;
            if( isMaximized() ) path = helper().roundedPath( rect, NoCorners );
            else if( _configuration->frameBorder() == Configuration::BorderNone ) path = helper().roundedPath( rect, CornerTopLeft|CornerTopRight );
            else path = helper().roundedPath( rect );

            // store old clip region
            QRegion clipRegion( painter->clipRegion() );

            // title bar background
            QRect topRect( rect );
            if( !hideTitleBar() )
            {
                topRect.setHeight( this->titleRect().height() + layoutMetric( LM_TitleEdgeTop ) + 1 );

                painter->setClipRect( topRect, Qt::IntersectClip );

                QLinearGradient gradient( topRect.topLeft(), topRect.bottomLeft() );
                gradient.setColorAt( 0, background.lighter( isActive() ? 120:100 ) );
                gradient.setColorAt( 0.8, background );
                painter->setBrush( gradient );

                painter->drawPath( path );

            }

            // palette
            #if BREEZE_USE_KDE4
            const QPalette palette = widget()->palette();
            #else
            const QPalette palette = ParentDecorationClass::palette();
            #endif

            const QColor base( palette.color( QPalette::Window ) );

            // window background
            QRect bottomRect( rect );
            if( !hideTitleBar() )
            {
                bottomRect.setTop( topRect.bottom() + 1 );
                painter->setClipRegion( clipRegion );
                painter->setClipRect( bottomRect, Qt::IntersectClip );
            }

            painter->setPen( Qt::NoPen );
            painter->setBrush( palette.color( QPalette::Window ) );
            painter->drawPath( path );

        }

        painter->restore();
    }


    //_______________________________________________________________________
    void Client::renderOutline( QPainter* painter, const QRect& rect ) const
    {

        // get outline color, check validity
        const QColor outline( outlineColor() );
        if( !outline.isValid() ) return;

        painter->save();
        painter->setBrush( Qt::NoBrush );
        painter->setPen( outline );

        // remove space used for buttons
        if( _itemData.count() > 1  )
        {
            for( int index = 0; index < _itemData.count(); index++ )
            {
                if( tabId( index ) != currentTabId() ) continue;
                const QRect topRect = _itemData[index]._boundingRect;
                painter->drawLine( QPointF( 0.5 + topRect.left(), 0.5 + topRect.bottom() ), QPointF( 0.5 + topRect.right(), 0.5 + topRect.bottom() ) );
            }

        } else {

            // title rect
            QRect topRect( rect );
            topRect.setHeight( this->titleRect().height() + layoutMetric( LM_TitleEdgeTop ) + 1 );
            painter->drawLine( QPointF( 0.5 + topRect.left(), 0.5 + topRect.bottom() ), QPointF( 0.5 + topRect.right(), 0.5 + topRect.bottom() ) );

        }

        painter->restore();

    }

    //_______________________________________________________________________
    void Client::renderItem( QPainter* painter, int index )
    {

        const ClientGroupItemData& item( _itemData[index] );

        // see if tag is active
        const int itemCount( _itemData.count() );

        // check item bounding rect
        if( !item._boundingRect.isValid() ) return;

        // create rect in which text is to be drawn
        QRect textRect( item._boundingRect.adjusted( 0, layoutMetric( LM_TitleEdgeTop ), 0, -1 ) );

        // add extra space needed for title outline
        if( itemCount > 1 || _itemData.isAnimated() )
        { textRect.adjust( layoutMetric( LM_TitleBorderLeft ), 0, -layoutMetric(LM_TitleBorderRight), 0 ); }

        // add extra space for the button
        if( itemCount > 1 && item._closeButton && item._closeButton.data()->isVisible() )
        { textRect.adjust( 0, 0, - buttonSize() - layoutMetric(LM_TitleEdgeRight), 0 ); }

        // check if current item is active
        const bool active( tabId(index) == currentTabId() );

        // get current item caption and update text rect
        const QString caption( itemCount == 1 ? this->caption() : this->caption(index) );

        if( _configuration->titleAlignment() != Configuration::AlignCenterFullWidth )
        { boundRectTo( textRect, titleRect() ); }

        // adjust textRect
        textRect = titleBoundingRect( painter->font(), textRect, caption );

        // render text and outline if needed
        if( active || itemCount == 1 )
        {

            // in multiple tabs render title outline in all cases
            // for active tab, current caption is "merged" with old caption, if any
            renderTitleText( painter, textRect, foregroundColor() );

        } else {

            const QColor foreground = KColorUtils::mix( foregroundColor(), backgroundColor(), 0.5 );
            renderTitleText( painter, textRect, caption, foreground );

        }

    }

    //_______________________________________________________________________
    void Client::renderTargetRect( QPainter* painter )
    {
        if( _itemData.targetRect().isNull() || _itemData.isAnimationRunning() ) return;

        // palette
        #if BREEZE_USE_KDE4
        const QPalette palette = widget()->palette();
        #else
        const QPalette palette = ParentDecorationClass::palette();
        #endif

        const QColor color = palette.color(QPalette::Highlight);

        painter->setPen(KColorUtils::mix(color, palette.color(QPalette::Active, QPalette::WindowText)));
        painter->setBrush( helper().alphaColor( color, 0.5 ) );
        painter->drawRect( QRectF(_itemData.targetRect()).adjusted( 4.5, 2.5, -4.5, -2.5 ) );

    }

    //_________________________________________________________
    void Client::activeChange( void )
    {

        ParentDecorationClass::activeChange();
        _itemData.setDirty( true );

        // tab id
        const bool tabChanged( tabCount() > 1 &&  _lastTabId != currentTabId() );
        if( tabChanged ) _lastTabId = currentTabId();

        // do not trigger animations on tab change because it triggers false positive
        if( animationsEnabled() && !tabChanged )
        {
            // reset animation
            _animation->setDirection( isActive() ? Animation::Forward : Animation::Backward );
            if(!isAnimated()) { _animation->start(); }
        }

        // update size grip so that it gets the right color
        // also make sure it is remaped to from z stack,
        // unless hidden
        if( hasSizeGrip() && !(isShade() || isMaximized() ))
        {
            sizeGrip().activeChange();
            sizeGrip().update();
        }

    }

    //_________________________________________________________
    void Client::maximizeChange( void  )
    {
        if( hasSizeGrip() ) sizeGrip().setVisible( !( isShade() || isMaximized() ) );
        setAlphaEnabled(!isMaximized());
        ParentDecorationClass::maximizeChange();
    }

    //_________________________________________________________
    void Client::shadeChange( void  )
    {
        if( hasSizeGrip() ) sizeGrip().setVisible( !( isShade() || isMaximized() ) );
        ParentDecorationClass::shadeChange();
    }

    //_________________________________________________________
    void Client::captionChange( void  )
    {

        ParentDecorationClass::captionChange();
        _itemData.setDirty( true );

    }

    //________________________________________________________________
    void Client::updateWindowShape()
    {

        if(isMaximized()) clearMask();
        else setMask( calcMask() );

    }

    //______________________________________________________________________________
    bool Client::eventFilter( QObject* object, QEvent* event )
    {

        // all dedicated event filtering is here to handle multiple tabs.

        bool state = false;
        switch( event->type() )
        {

            case QEvent::Show:
            if( widget() == object )
            { _itemData.setDirty( true ); }
            break;

            case QEvent::MouseButtonPress:
            if( widget() == object )
            { state = mousePressEvent( static_cast< QMouseEvent* >( event ) ); }
            break;

            case QEvent::MouseButtonRelease:
            if( widget() == object ) state = mouseReleaseEvent( static_cast< QMouseEvent* >( event ) );
            else if( Button *btn = qobject_cast< Button* >( object ) )
            {
                QMouseEvent* mouseEvent( static_cast< QMouseEvent* >( event ) );
                if( mouseEvent->button() == Qt::LeftButton && btn->rect().contains( mouseEvent->pos() ) )
                { state = closeItem( btn ); }
            }

            break;

            case QEvent::MouseMove:
            state = mouseMoveEvent( static_cast< QMouseEvent* >( event ) );
            break;

            case QEvent::DragEnter:
            if(  widget() == object )
            { state = dragEnterEvent( static_cast< QDragEnterEvent* >( event ) ); }
            break;

            case QEvent::DragMove:
            if( widget() == object )
            { state = dragMoveEvent( static_cast< QDragMoveEvent* >( event ) ); }
            break;

            case QEvent::DragLeave:
            if( widget() == object )
            { state = dragLeaveEvent( static_cast< QDragLeaveEvent* >( event ) ); }
            break;

            case QEvent::Drop:
            if( widget() == object )
            { state = dropEvent( static_cast< QDropEvent* >( event ) ); }
            break;

            default: break;

        }
        return state || ParentDecorationClass::eventFilter( object, event );

    }

    //_________________________________________________________
    void Client::resizeEvent( QResizeEvent* event )
    {

        // prepare item data updates
        _itemData.setDirty( true );

        // base class implementation
        ParentDecorationClass::resizeEvent( event );
    }

    //_________________________________________________________
    QRegion Client::region( KDecorationDefines::Region r )
    {

        // return empty region for anything but extended borders, when enabled
        if( r != KDecorationDefines::ExtendedBorderRegion )
        { return QRegion(); }

        // return empty region for maximized windows
        if( isMaximized() ) return QRegion();

        // return 3 pixels extended borders for sides that have no visible borders
        // also add the invisible pixels at the masked rounded corners, in non compositing mode
        if( configuration()->frameBorder() <= Configuration::BorderNoSide || !compositingActive() )
        {

            QRect rect = widget()->rect().adjusted(
                layoutMetric( LM_OuterPaddingLeft ),
                layoutMetric( LM_OuterPaddingTop ),
                - layoutMetric( LM_OuterPaddingRight ),
                - layoutMetric( LM_OuterPaddingBottom ) );

            rect.translate( -layoutMetric( LM_OuterPaddingLeft ), -layoutMetric( LM_OuterPaddingTop ) );

            // mask
            QRegion mask( calcMask() );
            if( mask.isEmpty() ) mask = rect;
            else mask.translate( -layoutMetric( LM_OuterPaddingLeft ), -layoutMetric( LM_OuterPaddingTop ) );

            // only return non-empty region on the sides for which there is no border
            enum { extendedBorderSize = 3 };

            if( configuration()->frameBorder() == Configuration::BorderNone ) return QRegion( rect.adjusted( -extendedBorderSize, 0, extendedBorderSize, extendedBorderSize ) ) - mask;
            else if( configuration()->frameBorder() == Configuration::BorderNoSide ) return QRegion( rect.adjusted( -extendedBorderSize, 0, extendedBorderSize, 0 ) ) - mask;
            else if( !compositingActive() ) return QRegion( rect ) - mask;

        }

        // fall back
        return QRegion();

    }


    //_________________________________________________________
    void Client::paintEvent( QPaintEvent* event )
    {

        // factory
        if(!( _initialized && _factory->initialized() ) ) return;

        QPainter painter(widget());
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setClipRegion( event->region() );

        // define frame
        QRect frame = widget()->rect();

        // draw shadows
        if( compositingActive() && !isMaximized() ) renderShadow( &painter, frame );

        // adjust frame
        frame.adjust(
            layoutMetric(LM_OuterPaddingLeft),
            layoutMetric(LM_OuterPaddingTop),
            -layoutMetric(LM_OuterPaddingRight),
            -layoutMetric(LM_OuterPaddingBottom) );

        // make sure ItemData and tabList are synchronized
        if(  _itemData.isDirty() || _itemData.count() != tabCount() )
        { updateItemBoundingRects( false ); }

        // render background
        renderBackground( &painter, frame, isShade() );

        if( !hideTitleBar() )
        {

            // outline
            renderOutline( &painter, frame );

            // title bounding rect
            painter.setFont( options()->font(isActive(), false) );

            // draw ClientGroupItems
            const int itemCount( _itemData.count() );
            for( int i = 0; i < itemCount; i++ ) renderItem( &painter, i );

            // draw target rect
            renderTargetRect( &painter );

        }

        // also update buttons in non compositin mode
        if( !compositingActive() )
        {
            // update buttons
            QList<Button*> buttons( widget()->findChildren<Button*>() );
            foreach( Button* button, buttons )
            { if( button->isVisible() ) button->update(); }

        }

    }

    //_____________________________________________________________
    bool Client::mousePressEvent( QMouseEvent* event )
    {

        const QPoint point = event->pos();
        if( tabIndexAt( point ) < 0 ) return false;
        _dragPoint = point;

        _mouseButton = event->button();
        bool accepted( false );
        if( buttonToWindowOperation( _mouseButton ) == TabDragOp )
        {

            accepted = true;

        } else if( buttonToWindowOperation( _mouseButton ) == OperationsOp ) {

            QPoint point = event->pos();
            const int clickedIndex( tabIndexAt( point ) );
            _mouseButton = Qt::NoButton;
            if ( tabIndexAt( point ) > -1)
            { showWindowMenu( widget()->mapToGlobal( event->pos() ), tabId(clickedIndex) ); }

            accepted = true;

        }

        return accepted;
    }

    //_____________________________________________________________
    bool Client::mouseReleaseEvent( QMouseEvent* event )
    {

        bool accepted( false );
        if( _mouseButton == event->button() && buttonToWindowOperation( _mouseButton ) != OperationsOp )
        {

            const QPoint point = event->pos();

            const long visibleItem = currentTabId();
            const int clickedIndex( tabIndexAt( point ) );
            if( clickedIndex >= 0 && visibleItem != tabId(clickedIndex) )
            {
                setCurrentTab( tabId(clickedIndex) );
                accepted = true;
            }

        }

        _mouseButton = Qt::NoButton;
        return accepted;

    }

    //_____________________________________________________________
    bool Client::mouseMoveEvent( QMouseEvent* event )
    {

        // check button and distance to drag point
        if( hideTitleBar() || _mouseButton == Qt::NoButton  || ( event->pos() - _dragPoint ).manhattanLength() <= QApplication::startDragDistance() )
        { return false; }

        bool accepted( false );
        if( buttonToWindowOperation( _mouseButton ) == TabDragOp )
        {

            const QPoint point = event->pos();
            const int clickedIndex( tabIndexAt( point ) );
            if( clickedIndex < 0 ) return false;

            QDrag *drag = new QDrag( widget() );
            QMimeData *groupData = new QMimeData();
            groupData->setData( tabDragMimeType(), QByteArray().setNum( (qint64) tabId(clickedIndex) ) );
            drag->setMimeData( groupData );
            _sourceItem = tabIndexAt( _dragPoint );

            // get tab geometry
            QRect geometry( _itemData[clickedIndex]._boundingRect );

            // remove space used for buttons
            if( _itemData.count() > 1  )
            {

                geometry.adjust( 0, 0,  - buttonSize() - layoutMetric(LM_TitleEdgeRight), 0 );

            } else {

                geometry.adjust(
                    buttonsLeftWidth() + layoutMetric( LM_TitleEdgeLeft ) , 0,
                    -( buttonsRightWidth() + layoutMetric( LM_TitleEdgeRight ) ), 0 );

            }

            // adjust geometry to include shadow size
            const bool drawShadow(
                compositingActive() &&
                KStyle::customStyleHint( QStringLiteral("SH_ArgbDndWindow"), widget() ) );

            if( drawShadow )
            { geometry.adjust( -layoutMetric( LM_OuterPaddingLeft ), -layoutMetric( LM_OuterPaddingTop ), layoutMetric( LM_OuterPaddingRight ), layoutMetric( LM_OuterPaddingBottom ) ); }

            // compute pixmap and assign
            QPixmap pixmap( itemDragPixmap( clickedIndex, geometry, drawShadow ) );
            drag->setPixmap( pixmap );

            // note: the pixmap is moved just above the pointer on purpose
            // because overlapping pixmap and pointer slows down the pixmap a lot.
            QPoint hotSpot( QPoint( event->pos().x() - geometry.left(), -1 ) );
            if( drawShadow ) hotSpot += QPoint( 0, layoutMetric( LM_OuterPaddingTop ) );

            // make sure the horizontal hotspot position is not too far away (more than 1px)
            // from the pixmap
            if( hotSpot.x() < -1 ) hotSpot.setX(-1);
            if( hotSpot.x() > geometry.width() ) hotSpot.setX( geometry.width() );

            drag->setHotSpot( hotSpot );

            _dragStartTimer.start( 50, this );
            drag->exec( Qt::MoveAction );

            // detach tab from window
            if( drag->target() == 0 && _itemData.count() > 1 )
            {
                _itemData.setDirty( true );
                untab( tabId(_sourceItem),
                    widget()->frameGeometry().adjusted(
                    layoutMetric( LM_OuterPaddingLeft ),
                    layoutMetric( LM_OuterPaddingTop ),
                    -layoutMetric( LM_OuterPaddingRight ),
                    -layoutMetric( LM_OuterPaddingBottom )
                    ).translated( QCursor::pos() - event->pos() +
                    QPoint( layoutMetric( LM_OuterPaddingLeft ), layoutMetric( LM_OuterPaddingTop )))
                    );
            }

            // reset button
            _mouseButton = Qt::NoButton;
            accepted = true;

        }

        return accepted;

    }

    //_____________________________________________________________
    bool Client::dragEnterEvent( QDragEnterEvent* event )
    {

        // check if drag enter is allowed
        if( !event->mimeData()->hasFormat( tabDragMimeType() ) || hideTitleBar() ) return false;

        // accept event
        event->acceptProposedAction();

        // animate
        if( event->source() != widget() )
        {

            _itemData.animate( AnimationEnter, tabIndexAt( event->pos(), true ) );

        } else if( _itemData.count() > 1 )  {

            _itemData.animate( AnimationEnter|AnimationSameTarget, tabIndexAt( event->pos(), true ) );

        }

        return true;

    }

    //_____________________________________________________________
    bool Client::dragLeaveEvent( QDragLeaveEvent* )
    {

        if( _itemData.animationType() & AnimationSameTarget )
        {

            if( _dragStartTimer.isActive() ) _dragStartTimer.stop();
            _itemData.animate( AnimationLeave|AnimationSameTarget, _sourceItem );

        } else if( _itemData.isAnimated() ) {

            _itemData.animate( AnimationLeave );

        }

        return true;

    }

    //_____________________________________________________________
    bool Client::dragMoveEvent( QDragMoveEvent* event )
    {

        // check format
        if( !event->mimeData()->hasFormat( tabDragMimeType() ) ) return false;

        // animate
        if( event->source() != widget() )
        {

            _itemData.animate( AnimationMove, tabIndexAt( event->pos(), true ) );

        } else if( _itemData.count() > 1 )  {

            if( _dragStartTimer.isActive() ) _dragStartTimer.stop();
            _itemData.animate( AnimationMove|AnimationSameTarget, tabIndexAt( event->pos(), true ) );

        }

        return false;

    }

    //_____________________________________________________________
    bool Client::dropEvent( QDropEvent* event )
    {

        const QPoint point = event->pos();
        _itemData.animate( AnimationNone );

        const QMimeData *groupData = event->mimeData();
        if( !groupData->hasFormat( tabDragMimeType() ) ) return false;

        _itemData.setDirty( true );

        const long source = QString::fromUtf8( groupData->data( tabDragMimeType() ) ).toLong();
        const int clickedIndex( tabIndexAt( point, true ) );
        if( clickedIndex < 0 ) tab_A_behind_B( source, tabId(_itemData.count()-1) );
        else tab_A_before_B( source, tabId(clickedIndex) );

        // update title
        if( widget() == event->source() ) updateTitleRect();

        return true;

    }

    //_____________________________________________________________
    void Client::timerEvent( QTimerEvent* event )
    {

        if( event->timerId() != _dragStartTimer.timerId() )
        { return ParentDecorationClass::timerEvent( event ); }

        _dragStartTimer.stop();

        // do nothing if there is only one tab
        if( _itemData.count() > 1 )
        {
            _itemData.animate( AnimationMove|AnimationSameTarget, _sourceItem );
            _itemData.animate( AnimationLeave|AnimationSameTarget, _sourceItem );
        }

    }

    //_____________________________________________________________
    bool Client::closeItem( const Button* button )
    {

        for( int i=0; i <  _itemData.count(); i++ )
        {
            if( button == _itemData[i]._closeButton.data() )
            {
                _itemData.setDirty( true );
                closeTab( tabId(i) );
                return true;
            }
        }
        return false;

    }

    //________________________________________________________________
    QPixmap Client::itemDragPixmap( int index, QRect geometry, bool drawShadow )
    {
        const bool itemValid( index >= 0 && index < tabCount() );

        QPixmap pixmap( geometry.size() );
        pixmap.fill( Qt::transparent );
        QPainter painter( &pixmap );
        painter.setRenderHints(QPainter::SmoothPixmapTransform|QPainter::Antialiasing);

        painter.translate( -geometry.topLeft() );

        // draw shadows
        if( drawShadow )
        {

            _factory->shadowTiles().render( geometry, &painter );
            geometry.adjust( layoutMetric( LM_OuterPaddingLeft ), layoutMetric( LM_OuterPaddingTop ), -layoutMetric( LM_OuterPaddingRight ), -layoutMetric( LM_OuterPaddingBottom ) );
        }


        // render background
        renderBackground( &painter, geometry, true );

        // render title text
        painter.setFont( options()->font(isActive(), false) );
        QRect textRect( geometry.adjusted( 0, layoutMetric( LM_TitleEdgeTop ), 0, -1 ) );

        if( itemValid )
        { textRect.adjust( layoutMetric( LM_TitleBorderLeft ), 0, -layoutMetric(LM_TitleBorderRight), 0 ); }

        const QString caption( itemValid ? this->caption(index) : this->caption() );
        renderTitleText( &painter, textRect, caption, foregroundColor( isActive() ) );

        painter.end();
        return pixmap;

    }

    //_________________________________________________________________
    void Client::createSizeGrip( void )
    {

        assert( !hasSizeGrip() );
        if( ( isResizable() && windowId() != 0 ) || isPreview() )
        {
            _sizeGrip = new SizeGrip( this );
            sizeGrip().setVisible( !( isMaximized() || isShade() ) );
        }

    }

    //_________________________________________________________________
    void Client::deleteSizeGrip( void )
    {
        assert( hasSizeGrip() );
        _sizeGrip->deleteLater();
        _sizeGrip = 0;
    }

    //_________________________________________________________________
    void Client::removeShadowHint( void )
    {

        // do nothing if no window id
        if( !windowId() ) return;

        // create atom
        if( !_shadowAtom )
        { _shadowAtom = helper().createAtom( QStringLiteral( "_KDE_NET_WM_SHADOW" ) ); }

        xcb_delete_property( helper().connection(), (xcb_window_t) windowId(), _shadowAtom);
    }

}
