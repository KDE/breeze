#ifndef breezesimulator_h
#define breezesimulator_h

//////////////////////////////////////////////////////////////////////////////
// breezesimulator.h
// simulates event chain passed to the application
// -------------------
//
// SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include <KLocalizedString>
#include <QAbstractButton>
#include <QTabBar>
#include <QTabWidget>
#include <QWidget>

#include <QBasicTimer>
#include <QEvent>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QTimerEvent>

namespace Breeze
{
class Simulator : public QObject
{
    Q_OBJECT

public:
    //* constructor
    explicit Simulator(QObject *parent)
        : QObject(parent)
        , _previousPosition(-1, -1)
    {
    }

    //*@name high level interface
    //@{

    //* click on button
    void click(QWidget *receiver, int delay = -1);

    //* click on button
    void click(QWidget *, const QPoint &, int = -1);

    //* slide
    void slide(QWidget *receiver, const QPoint &delta, int delay = -1);

    //* select item
    void selectItem(QWidget *, int row, int column = 0, int = -1);

    //* select combobox item
    void selectComboBoxItem(QWidget *, int, int = -1);

    //* select menu item
    void selectMenuItem(QWidget *, int, int = -1);

    //* select tab in tabwidget
    void selectTab(QTabWidget *, int, int = -1);

    //* select tab in tabbar
    void selectTab(QTabBar *, int, int = -1);

    //* write sample text
    void writeSampleText(QWidget *widget, int delay = -1)
    {
        writeText(widget, i18n("This is a sample text"), delay);
    }

    //* write string
    void writeText(QWidget *, QString, int = -1);

    //* clear text
    void clearText(QWidget *, int = -1);

    //* delay
    void wait(int delay);

    //@}

    //* true if aborted
    bool aborted(void) const
    {
        return _aborted;
    }

    //* run stored events
    void run(void);

    //* gab mouse
    static bool grabMouse(void)
    {
        return _grabMouse;
    }

    //* mouse grab
    static void setGrabMouse(bool value)
    {
        _grabMouse = value;
    }

    //* default delay
    static void setDefaultDelay(int value)
    {
        _defaultDelay = value;
    }

Q_SIGNALS:

    //* emitted when simulator starts and stops
    void stateChanged(bool);

public Q_SLOTS:

    //* abort simulations
    void abort(void);

protected:
    //* timer event
    void timerEvent(QTimerEvent *) override;

private:
    //*@name low level interface
    //@{

    //* enter widget
    bool enter(QWidget *receiver, int delay = -1)
    {
        return enter(receiver, receiver->rect().center(), delay);
    }

    //* enter receiver
    bool enter(QWidget *, const QPoint &, int = -1);

    //* mouse click event
    void postMouseClickEvent(QWidget *widget)
    {
        postMouseClickEvent(widget, Qt::LeftButton, widget->rect().center());
    }

    //* mouse click event
    void postMouseClickEvent(QWidget *, Qt::MouseButton, const QPoint &);

    //* 'basic' event
    void postEvent(QWidget *, QEvent::Type) const;

    //* hover
    void postHoverEvent(QWidget *, QEvent::Type, const QPoint &, const QPoint &) const;

    //* mouse event
    void
    postMouseEvent(QWidget *, QEvent::Type, Qt::MouseButton, const QPoint &, Qt::MouseButtons = Qt::NoButton, Qt::KeyboardModifiers = Qt::NoModifier) const;

    //* key event
    void postKeyClickEvent(QWidget *, Qt::Key, QString, Qt::KeyboardModifiers = Qt::NoModifier) const;

    //* key event
    void postKeyModifiersEvent(QWidget *, QEvent::Type, Qt::KeyboardModifiers) const;

    //* key event
    void postKeyEvent(QWidget *, QEvent::Type, Qt::Key, QString, Qt::KeyboardModifiers = Qt::NoModifier) const;

    //* delay
    void postDelay(int);

    //* set focus to widget
    void setFocus(QWidget *);

    //* move cursor
    void moveCursor(const QPoint &, int steps = 10);
    //@}

    using WidgetPointer = QPointer<QWidget>;

    //* event
    class Event
    {
    public:
        enum Type { Wait, Click, Slide, SelectItem, SelectComboBoxItem, SelectMenuItem, SelectTab, WriteText, ClearText };

        //* constructor
        Event(Type type, QWidget *receiver, int delay = 0)
            : _type(type)
            , _receiver(receiver)
            , _delay(delay)
        {
        }

        Type _type;
        WidgetPointer _receiver;
        QPoint _position;
        QString _text;
        int _delay = 0;
    };

    //* process event
    void processEvent(const Event &);

    //* process Qt event
    void postQEvent(QWidget *, QEvent *) const;

    //* convert QChar to key
    Qt::Key toKey(QChar) const;

    //* list of events
    using EventList = QList<Event>;
    EventList _events;

    //* previous position in global coordinates
    /** this is needed to have proper handling of enter/leave/hover events */
    QPoint _previousPosition;

    //* previous widget
    WidgetPointer _previousWidget;

    //* basic timer, for wait
    QBasicTimer _timer;

    //* pending events timer
    QBasicTimer _pendingEventsTimer;

    //* pending event
    WidgetPointer _pendingWidget;
    QList<QEvent *> _pendingEvents;

    //* true when simulations must be aborted
    bool _aborted = false;

    //* true if simulations also grab mouse
    static bool _grabMouse;

    //* default delay
    static int _defaultDelay;
};
}

#endif
