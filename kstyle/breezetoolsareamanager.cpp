#include "breezetoolsareamanager.h"
#include "breezepropertynames.h"

#include <QMainWindow>
#include <QMdiArea>
#include <QMenuBar>
#include <QObject>
#include <QToolBar>
#include <QWidget>
#include <QWindow>

#include <KColorUtils>
#include <QBoxLayout>
#include <QTabBar>

const char *colorProperty = "KDE_COLOR_SCHEME_PATH";

namespace Breeze
{
ToolsAreaManager::ToolsAreaManager()
    : QObject()
{
    if (qApp && qApp->property(colorProperty).isValid()) {
        auto path = qApp->property(colorProperty).toString();
        _config = KSharedConfig::openConfig(path);
    } else {
        _config = KSharedConfig::openConfig();
    }
    _watcher = KConfigWatcher::create(_config);
    connect(_watcher.data(), &KConfigWatcher::configChanged, this, &ToolsAreaManager::configUpdated);
    configUpdated();
}

ToolsAreaManager::~ToolsAreaManager()
{
}

template<class T1, class T2>
inline void appendIfNotAlreadyExists(T1 *list, T2 item)
{
    for (auto listItem : *list) {
        if (listItem == item) {
            return;
        }
    }
    list->append(item);
}

template<class T>
inline T &purge(T &c)
{
    auto it = std::remove_if(c.begin(), c.end(), [](auto t) {
        return t == nullptr;
    });
    c.erase(it, c.end());
    return c;
}

inline bool widgetCouldBeEligible(const QWidget *widget)
{
    if (widget == nullptr)
        return false;

    return qobject_cast<const QToolBar *>(widget) || qobject_cast<const QTabBar *>(widget) || qobject_cast<const QMenuBar *>(widget);
}

inline bool widgetIsEligibleForToolsAreaPalette(const QWidget *widget, const QMainWindow *window)
{
    Q_UNUSED(window)

    return qobject_cast<const QToolBar *>(widget);
}

inline bool widgetIsEligibleForToolsAreaIfNoSideWidgets(const QWidget *widget, const QMainWindow *window)
{
    if (window->centralWidget() != nullptr) {
        auto layout = window->centralWidget()->layout();

        // if a QWidget is the menuBar of the centralWidget, it's part of the tools area
        if (layout != nullptr && layout->menuBar() == widget) {
            return true;
        }

        const auto *tabWidget = qobject_cast<const QTabWidget *>(window->centralWidget());
        const auto *tabBar = qobject_cast<const QTabBar *>(widget);
        // if a QTabBar is the tab bar of a centralWidget QTabWidget and it's at the top, it's part of the tools area
        if (tabWidget && tabBar && tabWidget->tabBar() == tabBar && tabWidget->tabPosition() == QTabWidget::North) {
            return true;
        }
    }

    // if a QTabBar is the first item in a vertical layout in the window's main
    // content, then that's tools area too

    // extraneous loop so we can simplify logic with `break`
    do {
        const auto *tabBar = qobject_cast<const QTabBar *>(widget);
        if (tabBar == nullptr) {
            break;
        }
        if (window->centralWidget() != tabBar->parentWidget() || window->centralWidget() == nullptr) {
            break;
        }
        const auto *layout = qobject_cast<const QBoxLayout *>(window->centralWidget()->layout());
        if (layout == nullptr || layout->direction() != QBoxLayout::TopToBottom) {
            break;
        }
        const auto *item = layout->itemAt(0);
        if (item == nullptr) {
            break;
        }
        if (const_cast<QLayoutItem *>(item)->widget() == tabBar) { // const cast needed for qt5
            return true;
        }
    } while (false);

    return false;
}

inline bool widgetIsEligibleForToolsArea(const QWidget *widget, const QMainWindow *window, bool hasWidgetsToSide)
{
    if (widget == nullptr)
        return false;

    // if a QToolBar is in the TopToolBarArea, that's pretty obvious
    if (const auto *toolBar = qobject_cast<const QToolBar *>(widget)) {
        if (window->toolBarArea(const_cast<QToolBar *>(toolBar)) == Qt::TopToolBarArea) { // const cast needed for qt5
            return true;
        }
    }

    if (!hasWidgetsToSide) {
        if (widgetIsEligibleForToolsAreaIfNoSideWidgets(widget, window)) {
            return true;
        }
    }

    return false;
}

void ToolsAreaManager::registerApplication(QApplication *application)
{
    _listener = new AppListener(this);
    _listener->manager = this;
    if (application->property(colorProperty).isValid()) {
        auto path = application->property(colorProperty).toString();
        _config = KSharedConfig::openConfig(path);
        _watcher = KConfigWatcher::create(_config);
        connect(_watcher.data(), &KConfigWatcher::configChanged, this, &ToolsAreaManager::configUpdated);
    }
    application->installEventFilter(_listener);
    configUpdated();
}

QRect ToolsAreaManager::toolsAreaRect(const QMainWindow *window)
{
    Q_ASSERT(window);

    bool sideWidgetsExist = false;
    for (auto *child : window->children()) {
        if (auto *toolBar = qobject_cast<QToolBar *>(child)) {
            if (toolBar->isVisible() && window->toolBarArea(toolBar) & (Qt::LeftToolBarArea | Qt::RightToolBarArea)) {
                sideWidgetsExist = true;
                break;
            }
        } else if (auto *dockWidget = qobject_cast<QDockWidget *>(child)) {
            if (dockWidget->isVisible() && window->dockWidgetArea(dockWidget) & (Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea)) {
                sideWidgetsExist = true;
                break;
            }
        }
    }

    int itemHeight = window->menuWidget() ? window->menuWidget()->height() : 0;
    for (auto item : purge(_windows[window])) {
        if (item->isVisible() && widgetIsEligibleForToolsArea(item, window, sideWidgetsExist)) {
            itemHeight = qMax(item->mapTo(window, item->rect().bottomLeft()).y(), itemHeight);
        }
    }
    if (itemHeight > 0) {
        itemHeight += 1;
    }

    return QRect(0, 0, window->width(), itemHeight);
}

bool ToolsAreaManager::tryRegister(QPointer<QMainWindow> window, QPointer<QWidget> widget)
{
    Q_ASSERT(!widget.isNull());

    if (widgetIsEligibleForToolsArea(widget, window, false)) {
        if (widgetIsEligibleForToolsAreaPalette(widget, window)) {
            widget->setPalette(palette());
        }
        appendIfNotAlreadyExists(&_windows[window], widget);
        return true;
    }

    return false;
}

void ToolsAreaManager::tryUnregister(QPointer<QMainWindow> window, QPointer<QWidget> widget)
{
    Q_ASSERT(!widget.isNull());

    if (_windows[window].contains(widget)) {
        widget->setPalette(window->palette());
        _windows[window].removeAll(widget);
    }
}

void ToolsAreaManager::configUpdated()
{
    auto active = KColorScheme(QPalette::Active, KColorScheme::Header, _config);
    auto inactive = KColorScheme(QPalette::Inactive, KColorScheme::Header, _config);
    auto disabled = KColorScheme(QPalette::Disabled, KColorScheme::Header, _config);

    _palette = KColorScheme::createApplicationPalette(_config);

    _palette.setBrush(QPalette::Active, QPalette::Window, active.background());
    _palette.setBrush(QPalette::Active, QPalette::WindowText, active.foreground());
    _palette.setBrush(QPalette::Disabled, QPalette::Window, disabled.background());
    _palette.setBrush(QPalette::Disabled, QPalette::WindowText, disabled.foreground());
    _palette.setBrush(QPalette::Inactive, QPalette::Window, inactive.background());
    _palette.setBrush(QPalette::Inactive, QPalette::WindowText, inactive.foreground());

    for (auto window : _windows) {
        for (auto toolbar : window) {
            if (!toolbar.isNull()) {
                toolbar->setPalette(_palette);
            }
        }
    }

    _colorSchemeHasHeaderColor = KColorScheme::isColorSetSupported(_config, KColorScheme::Header);
}

bool AppListener::eventFilter(QObject *watched, QEvent *event)
{
    Q_ASSERT(watched);
    Q_ASSERT(event);

    if (watched == qApp && event->type() == QEvent::DynamicPropertyChange) {
        auto ev = static_cast<QDynamicPropertyChangeEvent *>(event);
        if (ev->propertyName() == colorProperty) {
            if (qApp && qApp->property(colorProperty).isValid()) {
                auto path = qApp->property(colorProperty).toString();
                manager->_config = KSharedConfig::openConfig(path);
            } else {
                manager->_config = KSharedConfig::openConfig();
            }
            manager->_watcher = KConfigWatcher::create(manager->_config);
            connect(manager->_watcher.data(), &KConfigWatcher::configChanged, manager, &ToolsAreaManager::configUpdated);
            manager->configUpdated();
        }
    }

    return false;
}

void ToolsAreaManager::registerWidget(QWidget *widget)
{
    Q_ASSERT(widget);
    auto ptr = QPointer<QWidget>(widget);

    auto parent = ptr;
    QPointer<QMainWindow> mainWindow = nullptr;
    while (parent != nullptr) {
        if (qobject_cast<QMdiArea *>(parent) || qobject_cast<QDockWidget *>(parent)) {
            break;
        }
        if (auto window = qobject_cast<QMainWindow *>(parent)) {
            mainWindow = window;
        }
        parent = parent->parentWidget();
    }
    if (mainWindow == nullptr) {
        return;
    }
    if (mainWindow != mainWindow->window()) {
        return;
    }
    tryRegister(mainWindow, widget);
}

void ToolsAreaManager::unregisterWidget(QWidget *widget)
{
    Q_ASSERT(widget);
    auto ptr = QPointer<QWidget>(widget);

    if (QPointer<QMainWindow> window = qobject_cast<QMainWindow *>(ptr)) {
        _windows.remove(window);
        return;
    } else if (widgetCouldBeEligible(widget)) {
        auto parent = ptr;
        QPointer<QMainWindow> mainWindow = nullptr;
        while (parent != nullptr) {
            if (qobject_cast<QMainWindow *>(parent)) {
                mainWindow = qobject_cast<QMainWindow *>(parent);
                break;
            }
            parent = parent->parentWidget();
        }
        if (mainWindow == nullptr) {
            return;
        }
        _windows[mainWindow].removeAll(widget);
    }
}

bool Breeze::ToolsAreaManager::hasHeaderColors()
{
    return _colorSchemeHasHeaderColor;
}
}
