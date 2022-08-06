#include "breezetoolsareamanager.h"
#include "breezedecorationsettingsprovider.h"
#include "breezepropertynames.h"

#include <QMainWindow>
#include <QMdiArea>
#include <QMenuBar>
#include <QObject>
#include <QToolBar>
#include <QWidget>
#include <QWindow>

#include <KColorUtils>

const char *colorProperty = "KDE_COLOR_SCHEME_PATH";

namespace Breeze
{
ToolsAreaManager::ToolsAreaManager(Helper *helper, QObject *parent)
    : QObject(parent)
    , _helper(helper)
{
    if (qApp && qApp->property(colorProperty).isValid()) {
        auto path = qApp->property(colorProperty).toString();
        _config = KSharedConfig::openConfig(path);
    } else {
        _config = KSharedConfig::openConfig();
    }
    _translucent = false;
    _watcher = KConfigWatcher::create(_config);
    connect(_watcher.data(), &KConfigWatcher::configChanged, this, &ToolsAreaManager::configUpdated);
    configUpdated();
}

ToolsAreaManager::~ToolsAreaManager()
{
}

template<class T1, class T2>
void appendIfNotAlreadyExists(T1 *list, T2 item)
{
    for (auto listItem : *list) {
        if (listItem == item) {
            return;
        }
    }
    list->append(item);
}

inline void doTranslucency(QMainWindow *win, bool on)
{
    if (on) { // deal with opaqueTitleBar on window decoration exceptions list
        auto windowDecorationSettings = SettingsProvider::self()->internalSettings(win);
        if (windowDecorationSettings->preventApplyOpacityToHeader())
            on = false;
    }

    if (on) {
        if (win->property("_klassy_was_translucent_set").toBool()) // if translucency has already been set don't set it again
            return;

        win->setProperty("_klassy_was_translucent", win->testAttribute(Qt::WA_TranslucentBackground));
        win->setProperty("_klassy_was_translucent_set", true);
        win->setAttribute(Qt::WA_TranslucentBackground, true);
    } else {
        if (!win->property("_klassy_was_translucent_set").toBool()) // do not turn off translucency if it was initially set by a third party
            return;

        win->setAttribute(Qt::WA_TranslucentBackground,
                          win->property("_klassy_was_translucent").toBool()); // set the translucency back to its initial value if altered here
        win->setProperty("_klassy_was_translucent", QVariant());
        win->setProperty("_klassy_was_translucent_set", false);
    }
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

    int itemHeight = window->menuWidget() ? window->menuWidget()->height() : 0;
    for (auto item : _windows[window]) {
        if (!item.isNull() && item->isVisible() && window->toolBarArea(item) == Qt::TopToolBarArea) {
            itemHeight = qMax(item->mapTo(window, item->rect().bottomLeft()).y(), itemHeight);
        }
    }
    if (itemHeight > 0) {
        itemHeight += window->devicePixelRatio();
    }

    return QRect(0, 0, window->width(), itemHeight);
}

bool ToolsAreaManager::tryRegisterToolBar(QPointer<QMainWindow> window, QPointer<QWidget> widget)
{
    Q_ASSERT(!widget.isNull());
    doTranslucency(window, _translucent);

    QPointer<QToolBar> toolbar;
    if (!(toolbar = qobject_cast<QToolBar *>(widget)))
        return false;

    if (window->toolBarArea(toolbar) == Qt::TopToolBarArea) {
        widget->setPalette(palette());
        appendIfNotAlreadyExists(&_windows[window], toolbar);
        return true;
    }

    return false;
}

void ToolsAreaManager::tryUnregisterToolBar(QPointer<QMainWindow> window, QPointer<QWidget> widget)
{
    Q_ASSERT(!widget.isNull());

    doTranslucency(window, false);

    QPointer<QToolBar> toolbar;
    if (!(toolbar = qobject_cast<QToolBar *>(widget)))
        return;

    if (window->toolBarArea(toolbar) != Qt::TopToolBarArea) {
        widget->setPalette(window->palette());
        _windows[window].removeAll(toolbar);
    }
}

void ToolsAreaManager::configUpdated()
{
    _colorSchemeHasHeaderColor = KColorScheme::isColorSetSupported(_config, KColorScheme::Header);

    auto translucent = false;
    auto active = KColorScheme(QPalette::Active, KColorScheme::Header, _config);
    auto inactive = KColorScheme(QPalette::Inactive, KColorScheme::Header, _config);
    auto disabled = KColorScheme(QPalette::Disabled, KColorScheme::Header, _config);

    if (_colorSchemeHasHeaderColor && _helper->decorationConfig()->applyOpacityToHeader()) {
        translucent = translucent || !active.background().isOpaque();
        translucent = translucent || !inactive.background().isOpaque();
        translucent = translucent || !disabled.background().isOpaque();
        translucent = translucent || _helper->decorationConfig()->activeTitlebarOpacity() < 100;
        translucent = translucent || _helper->decorationConfig()->inactiveTitlebarOpacity() < 100;
    }

    if (translucent != _translucent) {
        if (translucent)
            becomeTransparent();
        else
            becomeOpaque();
    }

    _translucent = translucent;

    _palette = KColorScheme::createApplicationPalette(_config);

    _palette.setBrush(QPalette::Active, QPalette::Window, active.background());
    _palette.setBrush(QPalette::Active, QPalette::WindowText, active.foreground());
    _palette.setBrush(QPalette::Disabled, QPalette::Window, disabled.background());
    _palette.setBrush(QPalette::Disabled, QPalette::WindowText, disabled.foreground());
    _palette.setBrush(QPalette::Inactive, QPalette::Window, inactive.background());
    _palette.setBrush(QPalette::Inactive, QPalette::WindowText, inactive.foreground());

    if (_helper->decorationConfig()->applyOpacityToHeader()) {
        // override active with opacity from decoration if needed
        if (active.background().isOpaque() && _helper->decorationConfig()->activeTitlebarOpacity() < 100) {
            QColor activeReplacedAlpha = active.background().color();
            activeReplacedAlpha.setAlphaF(qreal(_helper->decorationConfig()->activeTitlebarOpacity()) / 100);
            _palette.setColor(QPalette::Active, QPalette::Window, activeReplacedAlpha);
        }

        // override inactive with opacity from decoration if needed
        if (inactive.background().isOpaque() && _helper->decorationConfig()->inactiveTitlebarOpacity() < 100) {
            QColor inactiveReplacedAlpha = inactive.background().color();
            inactiveReplacedAlpha.setAlphaF(qreal(_helper->decorationConfig()->inactiveTitlebarOpacity()) / 100);
            _palette.setColor(QPalette::Inactive, QPalette::Window, inactiveReplacedAlpha);
        }
    }

    for (auto window : _windows) {
        for (auto toolbar : window) {
            if (!toolbar.isNull()) {
                toolbar->setPalette(_palette);
            }
        }
    }
}

void ToolsAreaManager::becomeOpaque()
{
    for (auto window : _windows.keys()) {
        doTranslucency(const_cast<QMainWindow *>(window), false);
    }
}

void ToolsAreaManager::becomeTransparent()
{
    for (auto window : _windows.keys()) {
        doTranslucency(const_cast<QMainWindow *>(window), true);
    }
}

bool AppListener::eventFilter(QObject *watched, QEvent *event)
{
    Q_ASSERT(watched);
    Q_ASSERT(event);

    if (watched != qApp) {
        return false;
    }

    if (event->type() == QEvent::DynamicPropertyChange) {
        if (watched != qApp) {
            return false;
        }
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

bool ToolsAreaManager::eventFilter(QObject *watched, QEvent *event)
{
    Q_ASSERT(watched);
    Q_ASSERT(event);

    QPointer<QObject> parent = watched;
    QPointer<QMainWindow> mainWindow = nullptr;
    while (parent != nullptr) {
        if (qobject_cast<QMainWindow *>(parent)) {
            mainWindow = qobject_cast<QMainWindow *>(parent);
            break;
        }
        parent = parent->parent();
    }

    if (QPointer<QMainWindow> mw = qobject_cast<QMainWindow *>(watched)) {
        QChildEvent *ev = nullptr;
        if (event->type() == QEvent::ChildAdded || event->type() == QEvent::ChildRemoved)
            ev = static_cast<QChildEvent *>(event);

        QPointer<QToolBar> tb = qobject_cast<QToolBar *>(ev->child());
        if (tb.isNull())
            return false;

        if (ev->added()) {
            if (mw->toolBarArea(tb) == Qt::TopToolBarArea)
                appendIfNotAlreadyExists(&_windows[mw], tb);
        } else if (ev->removed()) {
            _windows[mw].removeAll(tb);
        }
    } else if (qobject_cast<QToolBar *>(watched)) {
        if (!mainWindow.isNull()) {
            tryUnregisterToolBar(mainWindow, qobject_cast<QWidget *>(watched));
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
        if (qobject_cast<QMainWindow *>(parent)) {
            mainWindow = qobject_cast<QMainWindow *>(parent);
        }
        parent = parent->parentWidget();
    }
    if (mainWindow == nullptr) {
        return;
    }
    if (mainWindow != mainWindow->window()) {
        return;
    }
    tryRegisterToolBar(mainWindow, widget);
}

void ToolsAreaManager::unregisterWidget(QWidget *widget)
{
    Q_ASSERT(widget);
    auto ptr = QPointer<QWidget>(widget);

    if (QPointer<QMainWindow> window = qobject_cast<QMainWindow *>(ptr)) {
        _windows.remove(window);
        return;
    } else if (QPointer<QToolBar> toolbar = qobject_cast<QToolBar *>(ptr)) {
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
        _windows[mainWindow].removeAll(toolbar);
    }
}

bool Breeze::ToolsAreaManager::hasHeaderColors()
{
    return _colorSchemeHasHeaderColor;
}
}
