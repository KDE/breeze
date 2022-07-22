/*
 * SPDX-FileCopyrightText: 2021 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "styleredmond.h"

namespace Breeze
{

void RenderStyleRedmond18By18::renderCloseIcon()
{
    // first determine whether the close button should be enlarged to match an enlarged maximized button
    bool isSmallerMaximize = true;
    int roundedBoldPenWidth = 1;
    if (!notInTitlebar) {
        if (boldButtonIcons)
            isSmallerMaximize = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, m_maximizeBoldPenWidthFactor);
        else
            isSmallerMaximize = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
    }

    pen.setWidthF(pen.widthF() * 1.166666666); // thicken up diagonal slightly to give a balanced look
    painter->setPen(pen);

    if (notInTitlebar) {
        RenderDecorationButtonIcon18By18::renderCloseIcon();

    } else {
        if (boldButtonIcons) {
            pen.setWidthF(pen.widthF() * 1.5); // total factor of 1.75
            painter->setPen(pen);
        }

        if (isSmallerMaximize) {
            // slightly larger X than Breeze to tie-in with design of square maximize button
            painter->drawLine(QPointF(4.5, 4.5), QPointF(13.5, 13.5));
            painter->drawLine(QPointF(13.5, 4.5), QPointF(4.5, 13.5));
        } else {
            qreal adjustmentOffset = convertDevicePixelsTo18By18(0.5);

            // very slightly larger X again to match larger maximize button
            painter->drawLine(QPointF(4.5 - adjustmentOffset, 4.5 - adjustmentOffset), QPointF(13.5 + adjustmentOffset, 13.5 + adjustmentOffset));
            painter->drawLine(QPointF(13.5 + adjustmentOffset, 4.5 - adjustmentOffset), QPointF(4.5 - adjustmentOffset, 13.5 + adjustmentOffset));
        }
    }
}

void RenderStyleRedmond18By18::renderMaximizeIcon()
{
    bool isOddPenWidth = true;
    if (!notInTitlebar) {
        int roundedBoldPenWidth = 1;
        if (boldButtonIcons) {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, m_maximizeBoldPenWidthFactor);
        } else {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
        }
        pen.setWidthF(roundedBoldPenWidth + 0.01); // 0.01 prevents glitches as mentioned in breeze.h

        painter->setPen(pen);
    }

    // large square
    QRectF rect(QPointF(4.5, 4.5), QPointF(13.5, 13.5));
    if (!isOddPenWidth) {
        qreal adjustmentOffset = convertDevicePixelsTo18By18(0.5);
        rect.adjust(-adjustmentOffset, -adjustmentOffset, adjustmentOffset, adjustmentOffset);
    }

    painter->drawRoundedRect(rect, 0.025, 0.025, Qt::RelativeSize);
}

void RenderStyleRedmond18By18::renderRestoreIcon()
{
    pen.setJoinStyle(Qt::BevelJoin);
    painter->setPen(pen);

    if (this->notInTitlebar) {
        // disable antialiasing to remove blur at small sizes
        painter->setRenderHints(QPainter::Antialiasing, false);

        // overlapping windows icon
        painter->drawRect(QRectF(QPointF(4, 6), QPointF(11, 13)));
        painter->drawPolyline(QVector<QPointF>{QPointF(6, 6), QPointF(6, 4), QPointF(13, 4), QPointF(13, 11), QPointF(11, 11)});

    } else {
        if (boldButtonIcons) {
            int roundedBoldPenWidth = 1;
            roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, m_restoreBoldPenWidthFactor);
            // thicker pen in titlebar
            pen.setWidthF(roundedBoldPenWidth + 0.01); // 0.01 prevents glitches as mentioned in breeze.h
            painter->setPen(pen);

            renderRestoreIconAfterPenWidthSet();

        } else {
            int roundedBoldPenWidth = 1;
            roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);

            // thicker pen in titlebar
            pen.setWidthF(roundedBoldPenWidth + 0.01); // 0.01 prevents glitches as mentioned in breeze.h
            painter->setPen(pen);

            renderRestoreIconAfterPenWidthSet();
        }
    }
}

// actually renders the overlapping windows icon
void RenderStyleRedmond18By18::renderRestoreIconAfterPenWidthSet()
{
    // this is to calculate the offset to move the two rectangles further from each other onto an aligned pixel
    // they are moved apart as the line thickness increases -- this prevents blurriness when the lines are drawn too close together
    int roundedPenWidth = qRound(pen.widthF());

    qreal shiftOffset;
    if (roundedPenWidth > 1)
        shiftOffset = convertDevicePixelsTo18By18(0.5);
    else
        shiftOffset = 0;

    // overlapping windows icon
    // foreground square
    QRectF foregroundSquare(QPointF(4.5, 6.5), QPointF(11.5, 13.5));
    foregroundSquare.adjust(-shiftOffset, shiftOffset, -shiftOffset, shiftOffset);

    painter->drawRect(foregroundSquare);

    QVector<QPointF> background{QPointF(6.5, 6), QPointF(6.5, 4.5), QPointF(13.5, 4.5), QPointF(13.5, 11.5), QPointF(12, 11.5)};
    background[0].setX(background[0].x() + shiftOffset);
    background[0].setY(background[0].y() + shiftOffset);
    background[1].setX(background[1].x() + shiftOffset);
    background[1].setY(background[1].y() - shiftOffset);
    background[2].setX(background[2].x() + shiftOffset);
    background[2].setY(background[2].y() - shiftOffset);
    background[3].setX(background[3].x() + shiftOffset);
    background[3].setY(background[3].y() - shiftOffset);
    background[4].setX(background[4].x() - shiftOffset);
    background[4].setY(background[4].y() - shiftOffset);

    // background square
    painter->drawPolyline(background);
}

void RenderStyleRedmond18By18::renderMinimizeIcon()
{
    bool isOddPenWidth = true;

    if (!notInTitlebar) {
        int roundedBoldPenWidth;
        if (boldButtonIcons) {
            // thicker pen in titlebar
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, m_maximizeBoldPenWidthFactor);
        } else
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
        pen.setWidthF(roundedBoldPenWidth + 0.01); // 0.01 prevents glitches as mentioned in breeze.h
        painter->setPen(pen);
    }

    // horizontal line
    if (isOddPenWidth)
        painter->drawLine(QPointF(4.5, 12.5), QPointF(13.5, 12.5));
    else {
        qreal adjustmentOffset = convertDevicePixelsTo18By18(0.5);
        painter->drawLine(QPointF(4.5, 12.5 + adjustmentOffset), QPointF(13.5, 12.5 + adjustmentOffset));
    }
}

/*//Experimental 3 squares
    void RenderStyleRedmond18By18::renderKeepBehindIcon()
    {
        pen.setJoinStyle( Qt::RoundJoin );
        painter->setPen( pen );

        //foreground squares
        painter->drawRect( QRectF( QPointF( 3.5, 3.5 ), QPointF( 8.5, 8.5 ) ) );
        painter->drawRect( QRectF( QPointF( 9.5, 9.5 ), QPointF( 14.5, 14.5 ) ) );

        //filled background square
        painter->setBrush( pen.color() );
        painter->drawPolygon( QPolygonF()
            << QPointF( 8.5, 5.5 )
            << QPointF( 12.5, 5.5 )
            << QPointF( 12.5, 9.5 )
            << QPointF( 9.5, 9.5 )
            << QPointF( 9.5, 12.5 )
            << QPointF( 5.5, 12.5 )
            << QPointF( 5.5, 8.5 )
            << QPointF( 8.5, 8.5 )
        );
    }

    void RenderStyleRedmond18By18::renderKeepInFrontIcon()
    {
        pen.setJoinStyle( Qt::RoundJoin );
        painter->setPen( pen );

        //background squares
        painter->drawRect( QRectF( QPointF( 3.5, 3.5 ), QPointF( 8.5, 8.5 ) ) );
        painter->drawRect( QRectF( QPointF( 9.5, 9.5 ), QPointF( 14.5, 14.5 ) ) );

        //filled foreground square
        painter->setBrush( pen.color() );
        painter->drawRect( QRectF( QPointF( 5.5, 5.5 ), QPointF( 12.5, 12.5 ) ) );
    }
*/

/*//Experimental 2 squares
    void RenderStyleRedmond18By18::renderKeepBehindIcon()
    {
        pen.setJoinStyle( Qt::RoundJoin );
        painter->setPen( pen );

        //foreground square
        painter->drawRect( QRectF( QPointF( 7.5, 7.5 ), QPointF( 13.5, 13.5 ) ) );

        //filled background square
        painter->setBrush( pen.color() );
        painter->drawPolygon( QPolygonF()
            << QPointF( 4.5, 4.5 )
            << QPointF( 10.5, 4.5 )
            << QPointF( 10.5, 7.5 )
            << QPointF( 7.5, 7.5 )
            << QPointF( 7.5, 10.5 )
            << QPointF( 4.5, 10.5 )
        );

    }

    void RenderStyleRedmond18By18::renderKeepInFrontIcon()
    {
        pen.setJoinStyle( Qt::RoundJoin );
        painter->setPen( pen );

        //background square
        painter->drawRect( QRectF( QPointF( 7.5, 7.5 ), QPointF( 13.5, 13.5 ) ) );

        //filled foreground square
        painter->setBrush( pen.color() );
        painter->drawRect( QRectF( QPointF( 4.5, 4.5 ), QPointF( 10.5, 10.5 ) ) );
    }
*/

// For consistency with breeze icon set
void RenderStyleRedmond18By18::renderKeepBehindIcon()
{
    bool isOddPenWidth = true;
    if (!notInTitlebar) {
        int roundedBoldPenWidth = 1;
        if (boldButtonIcons) {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1.1);
        } else {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
        }

        // thicker pen in titlebar
        pen.setWidthF(roundedBoldPenWidth + 0.01);
        painter->setPen(pen);
    }

    if (!isOddPenWidth) {
        qreal adjustmentOffset = convertDevicePixelsTo18By18(0.5);
        painter->translate(QPointF(-adjustmentOffset, -adjustmentOffset));
    }

    // horizontal lines
    painter->drawLine(QPointF(4.5, 14.5), QPointF(13.5, 14.5));
    painter->drawLine(QPointF(9.5, 10.5), QPointF(13.5, 10.5));
    painter->drawLine(QPointF(9.5, 6.5), QPointF(13.5, 6.5));

    // arrow
    painter->drawLine(QPointF(4.5, 4.5), QPointF(4.5, 12.5));

    painter->drawPolyline(QVector<QPointF>{QPointF(2.5, 10.5), QPointF(4.5, 12.5), QPointF(6.5, 10.5)});
}

void RenderStyleRedmond18By18::renderKeepInFrontIcon()
{
    bool isOddPenWidth = true;
    if (!notInTitlebar) {
        int roundedBoldPenWidth = 1;
        if (boldButtonIcons) {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1.1);
        } else {
            isOddPenWidth = roundedPenWidthIsOdd(pen.widthF(), roundedBoldPenWidth, 1);
        }

        // thicker pen in titlebar
        pen.setWidthF(roundedBoldPenWidth + 0.01);
        painter->setPen(pen);
    }

    if (!isOddPenWidth) {
        qreal adjustmentOffset = convertDevicePixelsTo18By18(0.5);
        painter->translate(QPointF(-adjustmentOffset, -adjustmentOffset));
    }

    // horizontal lines
    painter->drawLine(QPointF(4.5, 4.5), QPointF(13.5, 4.5));
    painter->drawLine(QPointF(4.5, 8.5), QPointF(8.5, 8.5));
    painter->drawLine(QPointF(4.5, 12.5), QPointF(8.5, 12.5));

    // arrow
    painter->drawLine(QPointF(13.5, 6.5), QPointF(13.5, 14.5));

    painter->drawPolyline(QVector<QPointF>{QPointF(11.5, 8.5), QPointF(13.5, 6.5), QPointF(15.5, 8.5)});
}

void RenderStyleRedmond18By18::renderContextHelpIcon()
{
    if ((!notInTitlebar) && boldButtonIcons) {
        // thicker pen in titlebar
        pen.setWidthF(pen.widthF() * 1.6);
    }

    pen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(pen);

    // main body of question mark
    QPainterPath path;
    path.moveTo(7, 5);
    path.arcTo(QRectF(6.5, 3.5, 5.5, 5), 150, -160);
    path.cubicTo(QPointF(12, 9.5), QPointF(9, 7.5), QPointF(9, 11.5));
    painter->drawPath(path);

    // dot of question mark
    painter->setPen(Qt::NoPen);
    painter->setBrush(pen.color());
    if ((!notInTitlebar) && boldButtonIcons)
        painter->drawEllipse(QRectF(8, 14, 2, 2));
    else
        painter->drawEllipse(QRectF(8.25, 14.25, 1.5, 1.5));
}

}
