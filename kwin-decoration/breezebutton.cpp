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

#include "breezebutton.h"
#include "breezebutton.moc"
#include "breezeclient.h"

#include <QPainter>
#include <QPen>

#include <KColorUtils>
#include <KColorScheme>
#include <kcommondecoration.h>

namespace Breeze
{
    //_______________________________________________
    Button::Button(
        Client &parent,
        const QString& tip,
        ButtonType type):
        KCommonDecorationButton((::ButtonType)type, &parent),
        _client(parent),
        _helper( parent.helper() ),
        _type(type),
        _status( 0 ),
        _forceInactive( false ),
        _animation( new Animation( 150, this ) ),
        _opacity(0)
    {
        setAutoFillBackground(false);
        setAttribute(Qt::WA_NoSystemBackground);

        int size( _client.buttonSize() );
        setFixedSize( size, size );

        setCursor(Qt::ArrowCursor);
        setToolTip(tip);

        // setup animation
        _animation->setStartValue( 0 );
        _animation->setEndValue( 1.0 );
        _animation->setTargetObject( this );
        _animation->setPropertyName( "opacity" );
        _animation->setEasingCurve( QEasingCurve::InOutQuad );

        // setup connections
        reset(0);


    }

    //_______________________________________________
    Button::~Button()
    {}

    //_______________________________________________
    QColor Button::foregroundColor( void ) const
    {
        if( _client.isAnimated() && !_forceInactive && !_client.isForcedActive())
        {
            return KColorUtils::mix( foregroundColor( false ), foregroundColor( true ), _client.opacity() );

        } else return foregroundColor( isActive() || _client.isForcedActive() );
    }

    //_______________________________________________
    QColor Button::backgroundColor( void ) const
    {
        if( _client.isAnimated() && !_forceInactive && !_client.isForcedActive())
        {
            return KColorUtils::mix( backgroundColor( false ), backgroundColor( true ), _client.opacity() );

        } else return backgroundColor( isActive() || _client.isForcedActive() );
    }

    //___________________________________________________
    QColor Button::foregroundColor( bool active ) const
    { return _client.options()->color( KDecorationDefines::ColorFont, active ); }

    //___________________________________________________
    QColor Button::backgroundColor( bool active ) const
    { return _client.options()->color( KDecorationDefines::ColorTitleBar, active ); }

    //___________________________________________________
    bool Button::isActive( void ) const
    { return (!_forceInactive) && _client.isActive(); }

    //___________________________________________________
    bool Button::animationsEnabled( void ) const
    { return _client.configuration()->animationsEnabled(); }

    //___________________________________________________
    QSize Button::sizeHint() const
    {
        unsigned int size( _client.buttonSize() );
        return QSize( size, size );
    }

    //___________________________________________________
    void Button::reset( unsigned long )
    { _animation->setDuration( _client.configuration()->animationsDuration() ); }

    //___________________________________________________
    void Button::mousePressEvent( QMouseEvent *event )
    {

        if( _type == ButtonMax || event->button() == Qt::LeftButton )
        {
            _status |= Pressed;
            update();
        }

        KCommonDecorationButton::mousePressEvent( event );
    }

    //___________________________________________________
    void Button::mouseReleaseEvent( QMouseEvent* event )
    {
        if (_type != ButtonApplicationMenu)
        {
            _status &= ~Pressed;
            update();
        }

        KCommonDecorationButton::mouseReleaseEvent( event );
    }

    //___________________________________________________
    void Button::enterEvent( QEvent *event )
    {

        KCommonDecorationButton::enterEvent( event );
        _status |= Hovered;

        if( animationsEnabled() )
        {

            _animation->setDirection( Animation::Forward );
            if( !isAnimated() ) _animation->start();

        } else update();

    }

    //___________________________________________________
    void Button::leaveEvent( QEvent *event )
    {

        KCommonDecorationButton::leaveEvent( event );

        if( _status&Hovered && animationsEnabled() )
        {
            _animation->setDirection( Animation::Backward );
            if( !isAnimated() ) _animation->start();
        }

        _status &= ~Hovered;
        update();

    }

    //___________________________________________________
    void Button::paintEvent(QPaintEvent *event)
    {

        if( _client.hideTitleBar() ) return;

        // create painter
        QPainter painter( this );
        painter.setRenderHints(QPainter::Antialiasing);
        painter.setClipRegion( event->region() );

        QColor foreground = foregroundColor();
        QColor background = backgroundColor();

        const QPalette palette( this->palette() );
        const bool mouseOver( _status&Hovered );

        if( _type == ButtonItemClose || _type == ButtonClose )
        {

            qSwap( foreground, background );
            const QColor negativeColor(
                isActive() || _client.isForcedActive() ?
                KColorUtils::mix( background, _helper.negativeTextColor(palette), 0.5 ):
                _helper.alphaColor( _helper.negativeTextColor(palette), 0.5 ) );

            if( isAnimated() ) background = KColorUtils::mix( background, negativeColor, opacity() );
            else if( mouseOver ) background = negativeColor;

        } else if( isAnimated() ) {

            const QColor copy( background );
            background = KColorUtils::mix( background, foreground, opacity() );
            foreground = KColorUtils::mix( foreground, copy, opacity() );

            if( isActive() || _client.isForcedActive() )
            { background = _helper.alphaColor( background, 0.5 ); }

        } else if( mouseOver ) {

            qSwap( foreground, background );

            if( isActive() || _client.isForcedActive() )
            { background = _helper.alphaColor( background, 0.5 ); }

        }

        // Icon
        // for menu button the application icon is used
        if( isMenuButton() )
        {

            int iconScale( 0 );
            switch( _client.buttonSize() )
            {
                case Configuration::ButtonSmall: iconScale = 13; break;

                default:
                case Configuration::ButtonDefault: iconScale = 16; break;
                case Configuration::ButtonLarge: iconScale = 20; break;
                case Configuration::ButtonVeryLarge: iconScale = 24; break;
                case Configuration::ButtonHuge: iconScale = 35; break;
            }

            const QPixmap& pixmap( _client.icon().pixmap( iconScale ) );
            const double offset = 0.5*(width()-pixmap.width() );
            painter.drawPixmap(offset, offset-1, pixmap );

        } else {

            drawIcon( &painter, foreground, background );

        }

    }

    //___________________________________________________
    void Button::drawIcon( QPainter* painter, QColor foreground, QColor background )
    {

        painter->save();
        painter->setWindow( 0, 0, 18, 18 );
        painter->setRenderHints( QPainter::Antialiasing );

        // outside circle
        if( background.isValid() )
        {

            // render circle
            painter->setPen( Qt::NoPen );
            painter->setBrush( background );
            painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );

        }

        if( foreground.isValid() )
        {
            // render mark
            QPen pen;
            pen.setCapStyle( Qt::RoundCap );
            pen.setJoinStyle( Qt::MiterJoin );
            pen.setColor( foreground );

            const qreal penWidth( 1 );
            pen.setWidth( 2*penWidth );

            painter->setBrush( Qt::NoBrush );
            painter->setPen( pen );

            switch(_type)
            {

                case ButtonItemClose:
                case ButtonClose:

                // render
                painter->drawLine( QPointF( 5 + penWidth, 5 + penWidth ), QPointF( 13 - penWidth, 13 - penWidth ) );
                painter->drawLine( 13 - penWidth, 5 + penWidth, 5 + penWidth, 13 - penWidth );
                break;

                case ButtonMax:
                switch(_client.maximizeMode())
                {
                    case Client::MaximizeRestore:
                    case Client::MaximizeVertical:
                    case Client::MaximizeHorizontal:

                    painter->drawPolyline( QPolygonF()
                        << QPointF( 3.5 + penWidth, 11.5 - penWidth )
                        << QPointF( 9, 5.5 + penWidth )
                        << QPointF( 14.5 - penWidth, 11.5 - penWidth ) );

                    break;

                    case Client::MaximizeFull:
                    pen.setJoinStyle( Qt::RoundJoin );
                    painter->setPen( pen );

                    painter->drawPolygon( QPolygonF()
                        << QPointF( 3.5 + penWidth, 9 )
                        << QPointF( 9, 3.5 + penWidth )
                        << QPointF( 14.5 - penWidth, 9 )
                        << QPointF( 9, 14.5 - penWidth ) );

                    break;

                }
                break;

                case ButtonMin:
                painter->drawPolyline( QPolygonF()
                    << QPointF( 3.5 + penWidth, 6.5 + penWidth )
                    << QPointF( 9, 12.5 - penWidth )
                    << QPointF( 14.5 - penWidth, 6.5 + penWidth ) );

                break;

                default: break;

            }

        }

        painter->restore();

    }

    //___________________________________________________
    void Button::slotAppMenuHidden()
    {
        _status = Normal;
        update();
    }

}
