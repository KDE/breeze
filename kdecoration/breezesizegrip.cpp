/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "breezesizegrip.h"

#include <KDecoration2/DecoratedClient>

#include <QPainter>
#include <QPolygon>
#include <QTimer>

#if BREEZE_HAVE_X11
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <private/qtx11extras_p.h>
#else
#include <QX11Info>
#endif
#endif

namespace Breeze
{

//* scoped pointer convenience typedef
template<typename T>
using ScopedPointer = QScopedPointer<T, QScopedPointerPodDeleter>;

//_____________________________________________
SizeGrip::SizeGrip(Decoration *decoration)
    : QWidget(nullptr)
    , m_decoration(decoration)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);

    // cursor
    setCursor(Qt::SizeFDiagCursor);

    // size
    setFixedSize(QSize(GripSize, GripSize));

    // mask
    setMask(QRegion(QVector<QPoint>{QPoint(0, GripSize), QPoint(GripSize, 0), QPoint(GripSize, GripSize), QPoint(0, GripSize)}));

    // embed
    embed();
    updatePosition();

    // connections
    auto c = decoration->client().toStrongRef();
    Q_ASSERT(c);

    connect(c.data(), &KDecoration2::DecoratedClient::widthChanged, this, &SizeGrip::updatePosition);
    connect(c.data(), &KDecoration2::DecoratedClient::heightChanged, this, &SizeGrip::updatePosition);
    connect(c.data(), &KDecoration2::DecoratedClient::activeChanged, this, &SizeGrip::updateActiveState);

    // show
    show();
}

//_____________________________________________
SizeGrip::~SizeGrip()
{
}

//_____________________________________________
void SizeGrip::updateActiveState()
{
#if BREEZE_HAVE_X11
    if (QX11Info::isPlatformX11()) {
        const quint32 value = XCB_STACK_MODE_ABOVE;
        xcb_configure_window(QX11Info::connection(), winId(), XCB_CONFIG_WINDOW_STACK_MODE, &value);
        xcb_map_window(QX11Info::connection(), winId());
    }
#endif

    update();
}

//_____________________________________________
void SizeGrip::embed()
{
#if BREEZE_HAVE_X11

    if (!QX11Info::isPlatformX11())
        return;
    auto c = m_decoration.data()->client().toStrongRef();
    Q_ASSERT(c);

    xcb_window_t windowId = c->windowId();
    if (windowId) {
        /*
        find client's parent
        we want the size grip to be at the same level as the client in the stack
        */
        xcb_window_t current = windowId;
        auto connection = QX11Info::connection();
        xcb_query_tree_cookie_t cookie = xcb_query_tree_unchecked(connection, current);
        ScopedPointer<xcb_query_tree_reply_t> tree(xcb_query_tree_reply(connection, cookie, nullptr));
        if (!tree.isNull() && tree->parent)
            current = tree->parent;

        // reparent
        xcb_reparent_window(connection, winId(), current, 0, 0);
        setWindowTitle("Breeze::SizeGrip");

    } else {
        hide();
    }

#endif
}

//_____________________________________________
void SizeGrip::paintEvent(QPaintEvent *)
{
    if (!m_decoration)
        return;

    // get relevant colors
    const QColor backgroundColor(m_decoration.data()->titleBarColor());

    // create and configure painter
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor);

    // polygon
    painter.drawPolygon(QVector<QPoint>{QPoint(0, GripSize), QPoint(GripSize, 0), QPoint(GripSize, GripSize), QPoint(0, GripSize)});
}

//_____________________________________________
void SizeGrip::mousePressEvent(QMouseEvent *event)
{
    switch (event->button()) {
    case Qt::RightButton: {
        hide();
        QTimer::singleShot(5000, this, &QWidget::show);
        break;
    }

    case Qt::MiddleButton: {
        hide();
        break;
    }

    case Qt::LeftButton:
        if (rect().contains(event->pos())) {
            sendMoveResizeEvent(event->pos());
        }
        break;

    default:
        break;
    }
}

//_______________________________________________________________________________
void SizeGrip::updatePosition()
{
#if BREEZE_HAVE_X11
    if (!QX11Info::isPlatformX11())
        return;

    auto c = m_decoration.data()->client().toStrongRef();
    Q_ASSERT(c);

    QPoint position(c->width() - GripSize - Offset, c->height() - GripSize - Offset);

    quint32 values[2] = {quint32(position.x()), quint32(position.y())};
    xcb_configure_window(QX11Info::connection(), winId(), XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
#endif
}

//_____________________________________________
void SizeGrip::sendMoveResizeEvent(QPoint position)
{
#if BREEZE_HAVE_X11
    if (!QX11Info::isPlatformX11())
        return;

    // pointer to connection
    auto connection(QX11Info::connection());

    // client
    auto c = m_decoration.data()->client().toStrongRef();
    Q_ASSERT(c);

    /*
    get root position matching position
    need to use xcb because the embedding of the widget
    breaks QT's mapToGlobal and other methods
    */
    QPoint rootPosition(position);
    xcb_get_geometry_cookie_t cookie(xcb_get_geometry(connection, winId()));
    ScopedPointer<xcb_get_geometry_reply_t> reply(xcb_get_geometry_reply(connection, cookie, nullptr));
    if (reply) {
        // translate coordinates
        xcb_translate_coordinates_cookie_t coordCookie(
            xcb_translate_coordinates(connection, winId(), reply.data()->root, -reply.data()->border_width, -reply.data()->border_width));

        ScopedPointer<xcb_translate_coordinates_reply_t> coordReply(xcb_translate_coordinates_reply(connection, coordCookie, nullptr));

        if (coordReply) {
            rootPosition.rx() += coordReply.data()->dst_x;
            rootPosition.ry() += coordReply.data()->dst_y;
        }
    }

    // move/resize atom
    if (!m_moveResizeAtom) {
        // create atom if not found
        const QString atomName("_NET_WM_MOVERESIZE");
        xcb_intern_atom_cookie_t cookie(xcb_intern_atom(connection, false, atomName.size(), qPrintable(atomName)));
        ScopedPointer<xcb_intern_atom_reply_t> reply(xcb_intern_atom_reply(connection, cookie, nullptr));
        m_moveResizeAtom = reply ? reply->atom : 0;
    }

    if (!m_moveResizeAtom)
        return;

    // button release event
    xcb_button_release_event_t releaseEvent;
    memset(&releaseEvent, 0, sizeof(releaseEvent));

    releaseEvent.response_type = XCB_BUTTON_RELEASE;
    releaseEvent.event = winId();
    releaseEvent.child = XCB_WINDOW_NONE;
    releaseEvent.root = QX11Info::appRootWindow();
    releaseEvent.event_x = position.x();
    releaseEvent.event_y = position.y();
    releaseEvent.root_x = rootPosition.x();
    releaseEvent.root_y = rootPosition.y();
    releaseEvent.detail = XCB_BUTTON_INDEX_1;
    releaseEvent.state = XCB_BUTTON_MASK_1;
    releaseEvent.time = XCB_CURRENT_TIME;
    releaseEvent.same_screen = true;
    xcb_send_event(connection, false, winId(), XCB_EVENT_MASK_BUTTON_RELEASE, reinterpret_cast<const char *>(&releaseEvent));

    xcb_ungrab_pointer(connection, XCB_TIME_CURRENT_TIME);

    // move resize event
    xcb_client_message_event_t clientMessageEvent;
    memset(&clientMessageEvent, 0, sizeof(clientMessageEvent));

    clientMessageEvent.response_type = XCB_CLIENT_MESSAGE;
    clientMessageEvent.type = m_moveResizeAtom;
    clientMessageEvent.format = 32;
    clientMessageEvent.window = c->windowId();
    clientMessageEvent.data.data32[0] = rootPosition.x();
    clientMessageEvent.data.data32[1] = rootPosition.y();
    clientMessageEvent.data.data32[2] = 4; // bottom right
    clientMessageEvent.data.data32[3] = Qt::LeftButton;
    clientMessageEvent.data.data32[4] = 0;

    xcb_send_event(connection,
                   false,
                   QX11Info::appRootWindow(),
                   XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
                   reinterpret_cast<const char *>(&clientMessageEvent));

    xcb_flush(connection);
#endif
}

}
