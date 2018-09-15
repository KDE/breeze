//////////////////////////////////////////////////////////////////////////////
// breezeblurhelper.cpp
// handle regions passed to kwin for blurring
// -------------------
//
// Copyright (C) 2018 Alex Nemeth <alex.nemeth329@gmail.com>
//
// Largely rewritten from Oxygen widget style
// Copyright (C) 2007 Thomas Luebking <thomas.luebking@web.de>
// Copyright (c) 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include "breezeblurhelper.h"
#include "breezestyleconfigdata.h"

#include <KWindowEffects>

#include <QApplication>
#include <QEvent>
#include <QVector>

namespace
{

QRegion roundedRegion(const QRect &rect, int radius)
{
    QRegion region(rect, QRegion::Rectangle);

    // Round top-left corner.
    const QRegion topLeftCorner(rect.x(), rect.y(), radius, radius, QRegion::Rectangle);
    const QRegion topLeftRounded(rect.x(), rect.y(), 2 * radius, 2 * radius, QRegion::Ellipse);
    const QRegion topLeftEar = topLeftCorner - topLeftRounded;
    region -= topLeftEar;

    // Round top-right corner.
    const QRegion topRightCorner(
        rect.x() + rect.width() - radius, rect.y(),
        radius, radius, QRegion::Rectangle);
    const QRegion topRightRounded(
        rect.x() + rect.width() - 2 * radius, rect.y(),
        2 * radius, 2 * radius, QRegion::Ellipse);
    const QRegion topRightEar = topRightCorner - topRightRounded;
    region -= topRightEar;

    // Round bottom-right corner.
    const QRegion bottomRightCorner(
        rect.x() + rect.width() - radius, rect.y() + rect.height() - radius,
        radius, radius, QRegion::Rectangle);
    const QRegion bottomRightRounded(
        rect.x() + rect.width() - 2 * radius, rect.y() + rect.height() - 2 * radius,
        2 * radius, 2 * radius, QRegion::Ellipse);
    const QRegion bottomRightEar = bottomRightCorner - bottomRightRounded;
    region -= bottomRightEar;

    // Round bottom-left corner.
    const QRegion bottomLeftCorner(
        rect.x(), rect.y() + rect.height() - radius,
        radius, radius, QRegion::Rectangle);
    const QRegion bottomLeftRounded(
        rect.x(), rect.y() + rect.height() - 2 * radius,
        2 * radius, 2 * radius, QRegion::Ellipse);
    const QRegion bottomLeftEar = bottomLeftCorner - bottomLeftRounded;
    region -= bottomLeftEar;

    return region;
}

}

namespace Breeze
{
    //___________________________________________________________
    BlurHelper::BlurHelper(QObject* parent):
        QObject(parent)
    {
    }

    //___________________________________________________________
    void BlurHelper::registerWidget(QWidget* widget)
    {
        // install event filter
        addEventFilter(widget);

        // schedule shadow area repaint
        update(widget);
    }

    //___________________________________________________________
    void BlurHelper::unregisterWidget(QWidget* widget)
    {
        // remove event filter
        widget->removeEventFilter(this);
    }

    //___________________________________________________________
    bool BlurHelper::eventFilter(QObject* object, QEvent* event)
    {
        switch (event->type()) {
            case QEvent::Hide:
            case QEvent::Show:
            case QEvent::Resize:
            {
                // cast to widget and check
                QWidget* widget(qobject_cast<QWidget*>(object));

                if (!widget)
                    break;

                update(widget);
                break;
            }

            default: break;
        }

        // never eat events
        return false;
    }

    //___________________________________________________________
    void BlurHelper::update(QWidget* widget) const
    {
        #if BREEZE_HAVE_X11
        /*
        directly from bespin code. Supposibly prevent playing with some 'pseudo-widgets'
        that have winId matching some other -random- window
        */
        if (!(widget->testAttribute(Qt::WA_WState_Created) || widget->internalWinId()))
            return;

        #if QT_VERSION >= 0x050300
        const qreal dpr = qApp->devicePixelRatio();
        #else
        const qreal dpr = 1.0;
        #endif

        const QRect rect = widget->rect();
        const QRect scaledRect = QRect(rect.topLeft() * dpr, rect.size() * dpr);
        const int scaledRadius = qRound(Metrics::Frame_FrameRadius * dpr);

        const QRegion blurRegion = roundedRegion(scaledRect, scaledRadius);
        KWindowEffects::enableBlurBehind(widget->winId(), true, blurRegion);

        // force update
        if (widget->isVisible()) {
            widget->update();
        }

        #else
        Q_UNUSED( widget )
        #endif
    }
}
