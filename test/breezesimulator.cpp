// krazy:excludeall=qclasses

//////////////////////////////////////////////////////////////////////////////
// breezesimulator.cpp
// simulates event chain passed to the application
// -------------------
//
// SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezesimulator.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCursor>
#include <QFocusEvent>
#include <QHoverEvent>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMouseEvent>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QSlider>
#include <QStyle>
#include <QStyleOptionComboBox>
#include <QStyleOptionSlider>
#include <QToolButton>

#ifdef Q_OS_WIN
/* need windows.h include for Sleep function*/
#include <windows.h>
#endif

#ifdef Q_OS_UNIX
#include <ctime>
#endif

namespace Breeze
{
//_______________________________________________________________________
bool Simulator::_grabMouse = true;
int Simulator::_defaultDelay = 250;

//_______________________________________________________________________
void Simulator::wait(int delay)
{
    _events.append(Event(Event::Wait, nullptr, delay));
}

//_______________________________________________________________________
void Simulator::click(QWidget *receiver, int delay)
{
    QPoint position;
    if (QCheckBox *checkbox = qobject_cast<QCheckBox *>(receiver)) {
        QStyleOptionButton option;
        option.initFrom(checkbox);
        position = checkbox->style()->subElementRect(QStyle::SE_CheckBoxIndicator, &option, checkbox).center();

    } else if (QRadioButton *radiobutton = qobject_cast<QRadioButton *>(receiver)) {
        QStyleOptionButton option;
        option.initFrom(radiobutton);
        position = radiobutton->style()->subElementRect(QStyle::SE_RadioButtonIndicator, &option, radiobutton).center();

    } else if (const QMdiSubWindow *window = qobject_cast<QMdiSubWindow *>(receiver)) {
        QStyleOptionTitleBar option;
        option.initFrom(window);
        int titleBarHeight(window->style()->pixelMetric(QStyle::PM_TitleBarHeight, &option, window));
        QRect titleBarRect(QPoint(0, 0), QSize(window->width(), titleBarHeight));
        if (!titleBarRect.isValid())
            return;
        position = titleBarRect.center();

    } else {
        position = receiver->rect().center();
    }

    click(receiver, position, delay);
}

//_______________________________________________________________________
void Simulator::click(QWidget *receiver, const QPoint &position, int delay)
{
    Event event(Event::Click, receiver, delay);
    event._position = position;
    _events.append(event);
}

//_______________________________________________________________________
void Simulator::slide(QWidget *receiver, const QPoint &position, int delay)
{
    Event event(Event::Slide, receiver, delay);
    event._position = position;
    _events.append(event);
}

//_______________________________________________________________________
void Simulator::selectItem(QWidget *receiver, int row, int column, int delay)
{
    Event event(Event::SelectItem, receiver, delay);
    event._position = QPoint(column, row);
    _events.append(event);
}

//_______________________________________________________________________
void Simulator::selectComboBoxItem(QWidget *receiver, int index, int delay)
{
    Event event(Event::SelectComboBoxItem, receiver, delay);
    event._position.setX(index);
    _events.append(event);
}

//_______________________________________________________________________
void Simulator::selectMenuItem(QWidget *receiver, int index, int delay)
{
    Event event(Event::SelectMenuItem, receiver, delay);
    event._position.setX(index);
    _events.append(event);
}

//_______________________________________________________________________
void Simulator::selectTab(QTabWidget *tabwidget, int index, int delay)
{
    const auto children = tabwidget->children();
    for (QObject *child : children) {
        if (QTabBar *tabbar = qobject_cast<QTabBar *>(child)) {
            selectTab(tabbar, index, delay);
            break;
        }
    }
}

//_______________________________________________________________________
void Simulator::selectTab(QTabBar *receiver, int index, int delay)
{
    Event event(Event::SelectTab, receiver, delay);
    event._position.setX(index);
    _events.append(event);
}

//_______________________________________________________________________
void Simulator::writeText(QWidget *receiver, QString text, int delay)
{
    Event event(Event::WriteText, receiver, delay);
    event._text = text;
    _events.append(event);
}

//_______________________________________________________________________
void Simulator::clearText(QWidget *receiver, int delay)
{
    _events.append(Event(Event::ClearText, receiver, delay));
}

//_______________________________________________________________________
void Simulator::run(void)
{
    if (_events.isEmpty())
        return;

    // clear abort state
    _aborted = false;

    emit stateChanged(true);

    for (const Event &event : std::as_const(_events)) {
        if (_aborted) {
            _events.clear();
            return;
        }

        processEvent(event);
    }

    // add last event to reset previousWidget and previousPosition
    if (_previousWidget) {
        postEvent(_previousWidget.data(), QEvent::Leave);
        if (_previousWidget.data()->testAttribute(Qt::WA_Hover)) {
            const QPoint oldPosition(_previousWidget.data()->mapFromGlobal(_previousPosition));
            const QPoint newPosition(_previousWidget.data()->mapFromGlobal(QPoint(-1, -1)));
            postHoverEvent(_previousWidget.data(), QEvent::HoverLeave, newPosition, oldPosition);
        }

        _previousWidget.clear();
        _previousPosition = QPoint(-1, -1);
    }

    _events.clear();
    emit stateChanged(false);

    return;
}

//_______________________________________________________________________
void Simulator::abort(void)
{
    _aborted = true;
    emit stateChanged(true);
}

//_______________________________________________________________________
void Simulator::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == _timer.timerId()) {
        _timer.stop();

    } else if (event->timerId() == _pendingEventsTimer.timerId()) {
        if (_aborted) {
            qDeleteAll(_pendingEvents);

            _pendingEvents.clear();
            _pendingWidget.clear();

        } else if (_pendingWidget && _pendingWidget.data()->isVisible()) {
            _pendingEventsTimer.stop();
            for (QEvent *event : std::as_const(_pendingEvents)) {
                if (event->type() == QEvent::MouseMove) {
                    QPoint position(static_cast<QMouseEvent *>(event)->pos());
                    moveCursor(_pendingWidget.data()->mapToGlobal(position));
                }

                postQEvent(_pendingWidget.data(), event);
                postDelay(150);
            }

            _pendingEvents.clear();
            _pendingWidget.clear();
        }

    } else
        return QObject::timerEvent(event);
}

//_______________________________________________________________________
void Simulator::processEvent(const Event &event)
{
    if (_aborted)
        return;
    if (!event._receiver) {
        if (event._type == Event::Wait) {
            postDelay(event._delay);
        }

        return;
    }

    QWidget *receiver(event._receiver.data());
    switch (event._type) {
    // click event
    case Event::Click: {
        // enter widget or move cursor to relevant position
        if (!enter(receiver, event._position, event._delay)) {
            moveCursor(receiver->mapToGlobal(event._position));
            postMouseEvent(receiver, QEvent::MouseMove, Qt::NoButton, event._position);
        }

        postMouseClickEvent(receiver, Qt::LeftButton, event._position);
        break;
    }

    // slide
    case Event::Slide: {
        const QPoint &delta(event._position);

        // calculate begin position depending on widget type
        QPoint begin;
        if (const QSlider *slider = qobject_cast<QSlider *>(receiver)) {
            // this is copied from QSlider::initStyleOption
            QStyleOptionSlider option;
            option.initFrom(slider);
            option.orientation = slider->orientation();
            option.sliderPosition = slider->sliderPosition();
            option.minimum = slider->minimum();
            option.maximum = slider->maximum();
            option.upsideDown = (slider->orientation() == Qt::Horizontal) ? (slider->invertedAppearance() != (option.direction == Qt::RightToLeft))
                                                                          : (!slider->invertedAppearance());

            QRect handleRect(slider->style()->subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderHandle, slider));

            if (!handleRect.isValid())
                break;
            begin = handleRect.center();

        } else if (const QScrollBar *scrollbar = qobject_cast<QScrollBar *>(receiver)) {
            // this is copied from QSlider::initStyleOption
            QStyleOptionSlider option;
            option.initFrom(scrollbar);
            option.orientation = scrollbar->orientation();
            option.sliderPosition = scrollbar->sliderPosition();
            option.minimum = scrollbar->minimum();
            option.maximum = scrollbar->maximum();
            option.upsideDown = scrollbar->invertedAppearance();
            if (scrollbar->orientation() == Qt::Horizontal) {
                option.state |= QStyle::State_Horizontal;
            }

            QRect handleRect(scrollbar->style()->subControlRect(QStyle::CC_ScrollBar, &option, QStyle::SC_ScrollBarSlider, scrollbar));

            if (!handleRect.isValid())
                break;
            begin = handleRect.center();

        } else if (const QMdiSubWindow *window = qobject_cast<QMdiSubWindow *>(receiver)) {
            QStyleOptionTitleBar option;
            option.initFrom(window);
            int titleBarHeight(window->style()->pixelMetric(QStyle::PM_TitleBarHeight, &option, window));
            QRect titleBarRect(QPoint(0, 0), QSize(window->width(), titleBarHeight));
            if (!titleBarRect.isValid())
                break;
            begin = titleBarRect.center();

        } else {
            begin = receiver->rect().center();
        }

        // enter widget or move cursor to relevant position
        if (!enter(receiver, begin, event._delay)) {
            moveCursor(receiver->mapToGlobal(begin));
            postMouseEvent(receiver, QEvent::MouseMove, Qt::NoButton, begin);
        }

        const QPoint end(begin + delta);
        postMouseEvent(receiver, QEvent::MouseButtonPress, Qt::LeftButton, begin, Qt::LeftButton);
        setFocus(receiver);
        postDelay(50);
        const int steps = 10;
        for (int i = 0; i < steps; ++i) {
            QPoint current(begin.x() + qreal((i + 1) * (end.x() - begin.x())) / steps, begin.y() + qreal((i + 1) * (end.y() - begin.y())) / steps);
            moveCursor(receiver->mapToGlobal(current), 1);
            postMouseEvent(receiver, QEvent::MouseMove, Qt::NoButton, current, Qt::LeftButton, Qt::NoModifier);
            postDelay(20);
        }

        postMouseEvent(receiver, QEvent::MouseButtonRelease, Qt::LeftButton, end);
        break;
    }

    case Event::SelectItem: {
        const QAbstractItemView *view = qobject_cast<QAbstractItemView *>(receiver);
        if (!(view && view->model()))
            break;

        const int column(event._position.x());
        const int row(event._position.y());

        // find index
        const QModelIndex modelIndex(view->model()->index(row, column));
        if (!modelIndex.isValid())
            break;

        // get rect
        QRect r(view->visualRect(modelIndex));
        if (!r.isValid())
            break;

        // enter widget or move cursor to relevant position
        const QPoint position(r.center());
        if (!enter(view->viewport(), position, event._delay)) {
            moveCursor(view->viewport()->mapToGlobal(position));
            postMouseEvent(view->viewport(), QEvent::MouseMove, Qt::NoButton, position);
            postDelay(event._delay);
        }

        postMouseClickEvent(view->viewport(), Qt::LeftButton, position);
        break;
    }

    case Event::SelectComboBoxItem: {
        QComboBox *combobox = qobject_cast<QComboBox *>(receiver);
        if (!combobox)
            break;

        // get arrow rect
        QStyleOptionComboBox option;
        option.initFrom(combobox);
        QRect arrowRect(combobox->style()->subControlRect(QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxArrow, combobox));

        // enter widget or move cursor to relevant position
        QPoint position(arrowRect.center());
        if (!enter(combobox, position, event._delay)) {
            moveCursor(combobox->mapToGlobal(position));
            postMouseEvent(combobox, QEvent::MouseMove, Qt::NoButton, position);
            postDelay(event._delay);
        }

        postMouseClickEvent(combobox, Qt::LeftButton, position);

        // select item in view
        QAbstractItemView *view = combobox->view();
        const int row(event._position.x());
        const int column(0);

        // find index
        const QModelIndex modelIndex(view->model()->index(row, column));
        if (!modelIndex.isValid())
            break;

        // get rect
        QRect r(view->visualRect(modelIndex));
        if (!r.isValid())
            break;

        // send event
        position = QPoint(r.center());
        moveCursor(view->viewport()->mapToGlobal(position));
        postMouseEvent(view->viewport(), QEvent::MouseMove, Qt::NoButton, position, Qt::NoButton, Qt::NoModifier);
        postDelay(100);
        postMouseClickEvent(view->viewport(), Qt::LeftButton, position);
        break;
    }

    case Event::SelectMenuItem: {
        // retrieve menu
        QMenu *menu(nullptr);
        if (const QToolButton *button = qobject_cast<QToolButton *>(receiver))
            menu = button->menu();
        else if (const QPushButton *button = qobject_cast<QPushButton *>(receiver))
            menu = button->menu();

        // abort if not found
        if (!menu)
            break;

        // get action and geometry
        const int row(event._position.x());
        QList<QAction *> actions(menu->actions());
        if (row >= actions.size())
            break;

        menu->sizeHint();
        QRect r(menu->actionGeometry(actions[row]));
        if (!r.isValid())
            break;

        /*!
        HACK: As soon as leftMouseButton is pressed on a button with menu,
        the menu is shown and code is interrupted until an action is selected in the menu.
        As a consequence, one must first generate the events, execute them with a delay, and then
        click on the button (before delay is expired). This way, the menu events will be executed
        even if the menu is visible (and blocking further code execution).
        */
        QPoint position(r.center());
        _pendingWidget = menu;
        _pendingEvents.append(new QMouseEvent(QEvent::MouseMove, position, Qt::NoButton, Qt::NoButton, Qt::NoModifier));

        _pendingEvents.append(new QMouseEvent(QEvent::MouseButtonPress, position, Qt::LeftButton, Qt::NoButton, Qt::NoModifier));

        _pendingEvents.append(new QMouseEvent(QEvent::MouseButtonRelease, position, Qt::LeftButton, Qt::NoButton, Qt::NoModifier));

        _pendingEventsTimer.start(10, this);

        // enter widget or move cursor to relevant position
        position = receiver->rect().center();
        if (!enter(receiver, position, event._delay)) {
            moveCursor(receiver->mapToGlobal(position));
            postMouseEvent(receiver, QEvent::MouseMove, Qt::NoButton, position);
            postDelay(event._delay);
        }

        // click
        postMouseEvent(receiver, QEvent::MouseButtonPress, Qt::LeftButton, position, Qt::NoButton, Qt::NoModifier);
        break;
    }

    case Event::SelectTab: {
        const QTabBar *tabbar = qobject_cast<QTabBar *>(receiver);
        if (!tabbar)
            break;

        const int index(event._position.x());

        const QRect r(tabbar->tabRect(index));
        if (!r.isValid())
            break;

        // enter widget or move cursor to relevant position
        const QPoint position(r.center());
        if (!enter(receiver, position, event._delay)) {
            moveCursor(receiver->mapToGlobal(position));
            postMouseEvent(receiver, QEvent::MouseMove, Qt::NoButton, position);
            postDelay(event._delay);
        }

        postMouseClickEvent(receiver, Qt::LeftButton, position);
        break;
    }

    case Event::WriteText: {
        enter(receiver, receiver->rect().center(), event._delay);
        setFocus(receiver);
        const QString &text(event._text);
        for (int i = 0; i < text.length(); ++i) {
            const Qt::Key key(toKey(text.at(i)));
            const QString local(text.at(i));
            postKeyEvent(receiver, QEvent::KeyPress, key, local, Qt::NoModifier);
            postKeyEvent(receiver, QEvent::KeyRelease, key, local, Qt::NoModifier);
            postDelay(20);
        }
        break;
    }

    case Event::ClearText: {
        enter(receiver, receiver->rect().center(), event._delay);
        setFocus(receiver);
        postKeyEvent(receiver, QEvent::KeyPress, Qt::Key_A, QStringLiteral("a"), Qt::ControlModifier);
        postKeyEvent(receiver, QEvent::KeyRelease, Qt::Key_A, QStringLiteral("a"), Qt::ControlModifier);
        postDelay(20);
        postKeyClickEvent(receiver, Qt::Key_Backspace, QString());
    }

    default:
        break;
    }

    // delay
    postDelay(event._delay);

    return;
}

//_______________________________________________________________________
void Simulator::postEvent(QWidget *receiver, QEvent::Type type) const
{
    postQEvent(receiver, new QEvent(type));
}

//_______________________________________________________________________
void Simulator::postHoverEvent(QWidget *receiver, QEvent::Type type, const QPoint &newPosition, const QPoint &oldPosition) const
{
    postQEvent(receiver, new QHoverEvent(type, newPosition, oldPosition));
}

//_______________________________________________________________________
bool Simulator::enter(QWidget *receiver, const QPoint &position, int delay)
{
    if (receiver == _previousWidget.data())
        return false;

    // store position
    moveCursor(receiver->mapToGlobal(position));

    // leave previous widget
    if (_previousWidget) {
        postEvent(_previousWidget.data(), QEvent::Leave);
        if (_previousWidget.data()->testAttribute(Qt::WA_Hover)) {
            const QPoint oldPosition(_previousWidget.data()->mapFromGlobal(_previousPosition));
            const QPoint newPosition(_previousWidget.data()->mapFromGlobal(receiver->mapToGlobal(position)));
            postHoverEvent(_previousWidget.data(), QEvent::HoverLeave, newPosition, oldPosition);
        }
    }

    // enter or move in current widget
    if (!receiver->rect().contains(receiver->mapFromGlobal(_previousPosition))) {
        // enter current widget if needed
        postEvent(receiver, QEvent::Enter);
        if (receiver->testAttribute(Qt::WA_Hover)) {
            const QPoint oldPosition(receiver->mapFromGlobal(_previousPosition));
            const QPoint newPosition(position);
            postHoverEvent(receiver, QEvent::HoverEnter, newPosition, oldPosition);
        }

    } else if (receiver->mapFromGlobal(_previousPosition) != position) {
        // move mouse if needed
        postMouseEvent(receiver, QEvent::MouseMove, Qt::NoButton, position);
        if (receiver->testAttribute(Qt::WA_Hover)) {
            const QPoint oldPosition(receiver->mapFromGlobal(_previousPosition));
            const QPoint newPosition(position);
            postHoverEvent(receiver, QEvent::HoverMove, newPosition, oldPosition);
        }
    }

    // update previous widget and position
    _previousWidget = receiver;
    _previousPosition = receiver->mapToGlobal(position);
    postDelay(delay);

    return true;
}

//_______________________________________________________________________
void Simulator::postMouseClickEvent(QWidget *receiver, Qt::MouseButton button, const QPoint &position)
{
    // button press and button release
    postMouseEvent(receiver, QEvent::MouseButtonPress, button, position, button);
    setFocus(receiver);
    postDelay(50);
    postMouseEvent(receiver, QEvent::MouseButtonRelease, button, position, button);
}

//_______________________________________________________________________
void Simulator::postMouseEvent(QWidget *receiver,
                               QEvent::Type type,
                               Qt::MouseButton button,
                               const QPoint &position,
                               Qt::MouseButtons buttons,
                               Qt::KeyboardModifiers modifiers) const
{
    postQEvent(receiver, new QMouseEvent(type, position, receiver->mapToGlobal(position), button, buttons, modifiers));
}

//_______________________________________________________________________
void Simulator::postKeyClickEvent(QWidget *receiver, Qt::Key key, QString text, Qt::KeyboardModifiers modifiers) const
{
    postKeyModifiersEvent(receiver, QEvent::KeyPress, modifiers);
    postKeyEvent(receiver, QEvent::KeyPress, key, text, modifiers);
    postKeyEvent(receiver, QEvent::KeyRelease, key, text, modifiers);
    postKeyModifiersEvent(receiver, QEvent::KeyRelease, modifiers);
}

//_______________________________________________________________________
void Simulator::postKeyModifiersEvent(QWidget *receiver, QEvent::Type type, Qt::KeyboardModifiers modifiers) const
{
    if (modifiers == Qt::NoModifier)
        return;

    switch (type) {
    case QEvent::KeyPress: {
        if (modifiers & Qt::ShiftModifier) {
            postKeyEvent(receiver, QEvent::KeyPress, Qt::Key_Shift, QString());
        }

        if (modifiers & Qt::ControlModifier) {
            postKeyEvent(receiver, QEvent::KeyPress, Qt::Key_Control, QString(), modifiers & Qt::ShiftModifier);
        }

        if (modifiers & Qt::AltModifier) {
            postKeyEvent(receiver, QEvent::KeyPress, Qt::Key_Alt, QString(), modifiers & (Qt::ShiftModifier | Qt::ControlModifier));
        }

        if (modifiers & Qt::MetaModifier) {
            postKeyEvent(receiver, QEvent::KeyPress, Qt::Key_Meta, QString(), modifiers & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier));
        }

        break;
    }

    case QEvent::KeyRelease: {
        if (modifiers & Qt::MetaModifier) {
            postKeyEvent(receiver, QEvent::KeyRelease, Qt::Key_Meta, QString());
        }

        if (modifiers & Qt::AltModifier) {
            postKeyEvent(receiver, QEvent::KeyRelease, Qt::Key_Alt, QString(), modifiers & Qt::MetaModifier);
        }

        if (modifiers & Qt::ControlModifier) {
            postKeyEvent(receiver, QEvent::KeyRelease, Qt::Key_Control, QString(), modifiers & (Qt::MetaModifier | Qt::AltModifier));
        }

        if (modifiers & Qt::ShiftModifier) {
            postKeyEvent(receiver, QEvent::KeyRelease, Qt::Key_Shift, QString(), modifiers & (Qt::MetaModifier | Qt::AltModifier | Qt::ControlModifier));
        }
    }

    default:
        break;
    }
}

//_______________________________________________________________________
void Simulator::postKeyEvent(QWidget *receiver, QEvent::Type type, Qt::Key key, QString text, Qt::KeyboardModifiers modifiers) const
{
    postQEvent(receiver, new QKeyEvent(type, key, modifiers, text));
}

//_______________________________________________________________________
void Simulator::postDelay(int delay)
{
    // check value
    if (delay == -1)
        delay = _defaultDelay;
    if (delay <= 0)
        return;

    // this is largely inspired from qtestlib's qsleep implementation
    _timer.start(delay, this);
    while (_timer.isActive()) {
        // flush events in loop
        QCoreApplication::processEvents(QEventLoop::AllEvents, delay);
        int ms(10);

// sleep
#ifdef Q_OS_WIN
        Sleep(uint(ms));
#else
        struct timespec ts = {ms / 1000, (ms % 1000) * 1000 * 1000};
        nanosleep(&ts, nullptr);
#endif
    }
}

//_______________________________________________________________________
void Simulator::moveCursor(const QPoint &position, int steps)
{
    // do nothing if mouse grab is disabled
    if (!_grabMouse)
        return;
    if (_aborted)
        return;
    const QPoint begin(QCursor::pos());
    const QPoint end(position);
    if (begin == end)
        return;

    if (steps > 1) {
        for (int i = 0; i < steps; ++i) {
            const QPoint current(begin.x() + qreal((i + 1) * (end.x() - begin.x())) / steps, begin.y() + qreal((i + 1) * (end.y() - begin.y())) / steps);
            QCursor::setPos(current);
            postDelay(10);
        }

    } else {
        QCursor::setPos(end);
    }
}

//_______________________________________________________________________
void Simulator::setFocus(QWidget *receiver)
{
    if (receiver->focusPolicy() != Qt::NoFocus) {
        receiver->setFocus();
    }
}

//_______________________________________________________________________
Qt::Key Simulator::toKey(QChar a) const
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return (Qt::Key)QKeySequence(a)[0];
#else
    return QKeySequence(a)[0].key();
#endif
}

//_______________________________________________________________________
void Simulator::postQEvent(QWidget *receiver, QEvent *event) const
{
    if (_aborted)
        delete event;
    else
        qApp->postEvent(receiver, event);
}
}
