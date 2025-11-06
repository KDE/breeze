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

#ifdef Q_OS_WIN
#include "windows.h"
#endif

const char *colorProperty = "KDE_COLOR_SCHEME_PATH";

#ifdef Q_OS_WIN
static bool isHighContrastModeActive()
{
    HIGHCONTRAST result;
    result.cbSize = sizeof(HIGHCONTRAST);
    if (SystemParametersInfo(SPI_GETHIGHCONTRAST, result.cbSize, &result, 0)) {
        return (result.dwFlags & HCF_HIGHCONTRASTON);
    }
    return false;
}
#endif

namespace Breeze
{
ToolsAreaManager::ToolsAreaManager()
    : QObject()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QString path;
    if (qApp && qApp->property(colorProperty).isValid()) {
        path = qApp->property(colorProperty).toString();
    }
    recreateConfigWatcher(path);
#endif
    configUpdated();
}

ToolsAreaManager::~ToolsAreaManager()
{
}

void ToolsAreaManager::loadSchemeConfig(const QString &path)
{
    const auto openFlags = path.isEmpty() ? KConfig::OpenFlag::FullConfig : KConfig::OpenFlag::NoGlobals;
    _config = KSharedConfig::openConfig(path, openFlags);
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void ToolsAreaManager::recreateConfigWatcher(const QString &path)
{
    loadSchemeConfig(path);
    if (!path.startsWith(QLatin1Char('/'))) {
        _watcher = KConfigWatcher::create(_config);
        connect(_watcher.data(), &KConfigWatcher::configChanged, this, &ToolsAreaManager::configUpdated);
    } else {
        _watcher.reset();
    }
}
#endif

void ToolsAreaManager::appendIfNotAlreadyExists(const QMainWindow *window, const QPointer<QToolBar> &toolBar)
{
    const auto windowIt = std::find_if(_windows.begin(), _windows.end(), [window](const WindowToolBars &windowToolBars) {
        return window == windowToolBars.window;
    });
    if (windowIt != _windows.end()) {
        if (!windowIt->toolBars.contains(toolBar)) {
            windowIt->toolBars.append(toolBar);
        }
    } else {
        _windows.emplace_back(WindowToolBars{window, {toolBar}});
        connect(window, &QObject::destroyed, this, [this, window] {
            removeWindow(window);
        });
    }
}

void ToolsAreaManager::removeWindowToolBar(const QMainWindow *window, const QPointer<QToolBar> &toolBar)
{
    const auto windowIt = std::find_if(_windows.begin(), _windows.end(), [window](const WindowToolBars &windowToolBars) {
        return window == windowToolBars.window;
    });
    if (windowIt != _windows.end()) {
        windowIt->toolBars.removeAll(toolBar);
    }
}

void ToolsAreaManager::removeWindow(const QMainWindow *window)
{
    std::erase_if(_windows, [window](const WindowToolBars &windowToolBars) {
        return window == windowToolBars.window;
    });
}

void ToolsAreaManager::registerApplication(QApplication *application)
{
    _listener = new AppListener(this);
    _listener->manager = this;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (application->property(colorProperty).isValid()) {
        auto path = application->property(colorProperty).toString();
        recreateConfigWatcher(path);
    }
#endif
    application->installEventFilter(_listener);
    configUpdated();
}

QRect ToolsAreaManager::toolsAreaRect(const QMainWindow &window) const
{
    int itemHeight = window.menuWidget() ? window.menuWidget()->height() : 0;
    const auto windowIt = std::find_if(_windows.begin(), _windows.end(), [&window](const WindowToolBars &windowToolBars) {
        return &window == windowToolBars.window;
    });
    if (windowIt != _windows.end()) {
        for (auto item : windowIt->toolBars) {
            if (!item.isNull() && item->isVisible() && window.toolBarArea(item) == Qt::TopToolBarArea) {
                itemHeight = qMax(item->mapTo(&window, item->rect().bottomLeft()).y(), itemHeight);
            }
        }
    }
    if (itemHeight > 0) {
        itemHeight += 1;
    }

    return QRect(0, 0, window.width(), itemHeight);
}

bool ToolsAreaManager::tryRegisterToolBar(QPointer<const QMainWindow> window, QPointer<QWidget> widget)
{
    Q_ASSERT(!widget.isNull());

    QPointer<QToolBar> toolbar;
    if (!(toolbar = qobject_cast<QToolBar *>(widget))) {
        return false;
    }

    if (window->toolBarArea(toolbar) == Qt::TopToolBarArea) {
        widget->setProperty("breeze_has_toolsarea_palette", true);
        widget->setPalette(palette());
        appendIfNotAlreadyExists(window, toolbar);
        return true;
    }

    return false;
}

void ToolsAreaManager::tryUnregisterToolBar(QPointer<const QMainWindow> window, QPointer<QWidget> widget)
{
    Q_ASSERT(!widget.isNull());

    QPointer<QToolBar> toolbar;
    if (!(toolbar = qobject_cast<QToolBar *>(widget))) {
        return;
    }

    if (window->toolBarArea(toolbar) != Qt::TopToolBarArea) {
        widget->setProperty("breeze_has_toolsarea_palette", true);
        widget->setPalette(window->palette());
        removeWindowToolBar(window, toolbar);
    }
}

void ToolsAreaManager::configUpdated()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (qApp->property(colorProperty).isValid()) {
        const auto colorSchemePath = qApp->property(colorProperty).toString();
#ifdef Q_OS_WIN
        // If no color scheme is set and high-contrast is active then use the system colors
        if (colorSchemePath.isEmpty() && isHighContrastModeActive()) {
            _config.reset();
        } else
#endif
        {
            if (!_config || _config->name() != colorSchemePath) {
                loadSchemeConfig(colorSchemePath);
            }
        }
    } else {
#ifdef Q_OS_WIN
        // If high-contrast is active then use the system colors
        if (isHighContrastModeActive()) {
            _config.reset();
        } else
#endif
        {
            loadSchemeConfig(QString{});
        }
    }
#endif
    auto active = KColorScheme(QPalette::Active, KColorScheme::Header, _config);
    auto inactive = KColorScheme(QPalette::Inactive, KColorScheme::Header, _config);
    auto disabled = KColorScheme(QPalette::Disabled, KColorScheme::Header, _config);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    _palette = qApp->palette();
#else
    _palette = KColorScheme::createApplicationPalette(_config);
#endif

    _palette.setBrush(QPalette::Active, QPalette::Window, active.background());
    _palette.setBrush(QPalette::Active, QPalette::WindowText, active.foreground());
    _palette.setBrush(QPalette::Disabled, QPalette::Window, disabled.background());
    _palette.setBrush(QPalette::Disabled, QPalette::WindowText, disabled.foreground());
    _palette.setBrush(QPalette::Inactive, QPalette::Window, inactive.background());
    _palette.setBrush(QPalette::Inactive, QPalette::WindowText, inactive.foreground());

    for (const WindowToolBars &windowToolBars : _windows) {
        for (const auto &toolbar : windowToolBars.toolBars) {
            if (!toolbar.isNull()) {
                toolbar->setProperty("breeze_has_toolsarea_palette", true);
                toolbar->setPalette(_palette);
            }
        }

        if (QMenuBar *menuBar = windowToolBars.window->menuBar()) {
            menuBar->setProperty("breeze_has_toolsarea_palette", true);
            menuBar->setPalette(_palette);
        }
    }

    _colorSchemeHasHeaderColor = _config ? KColorScheme::isColorSetSupported(_config, KColorScheme::Header) : false;
}

bool AppListener::eventFilter(QObject *watched, QEvent *event)
{
    Q_ASSERT(watched);
    Q_ASSERT(event);

    if (watched != qApp) {
        return false;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (event->type() == QEvent::ApplicationPaletteChange) {
        manager->configUpdated();
    }
#else
    if (event->type() == QEvent::DynamicPropertyChange) {
        auto ev = static_cast<QDynamicPropertyChangeEvent *>(event);
        if (ev->propertyName() == colorProperty) {
            QString path;
            if (qApp && qApp->property(colorProperty).isValid()) {
                path = qApp->property(colorProperty).toString();
            }
            manager->recreateConfigWatcher(path);
            manager->configUpdated();
        }
    }
#endif

    return false;
}

bool ToolsAreaManager::eventFilter(QObject *watched, QEvent *event)
{
    Q_ASSERT(watched);
    Q_ASSERT(event);

    QPointer<QObject> parent = watched;
    QPointer<const QMainWindow> mainWindow = nullptr;
    while (parent != nullptr) {
        if (qobject_cast<QMainWindow *>(parent)) {
            mainWindow = qobject_cast<QMainWindow *>(parent);
            break;
        }
        parent = parent->parent();
    }

    if (const QMainWindow *mw = qobject_cast<QMainWindow *>(watched)) {
        QChildEvent *ev = nullptr;
        if (event->type() == QEvent::ChildAdded || event->type() == QEvent::ChildRemoved) {
            ev = static_cast<QChildEvent *>(event);

            if (event->type() == QEvent::ChildAdded) {
                QChildEvent *childEvent = static_cast<QChildEvent *>(event);
                if (QMenuBar *menuBar = qobject_cast<QMenuBar *>(childEvent->child())) {
                    menuBar->setProperty("breeze_has_toolsarea_palette", true);
                    menuBar->setPalette(_palette);
                }
            }
        } else {
            return false;
        }

        QPointer<QToolBar> tb = qobject_cast<QToolBar *>(ev->child());
        if (tb.isNull()) {
            return false;
        }

        if (ev->added()) {
            if (mw->toolBarArea(tb) == Qt::TopToolBarArea) {
                appendIfNotAlreadyExists(mw, tb);
            }
        } else if (ev->removed()) {
            removeWindowToolBar(mw, tb);
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

    QPointer<const QMainWindow> mainWindow = qobject_cast<QMainWindow *>(ptr);

    if (mainWindow && mainWindow.data() == mainWindow->window()) {
        const auto toolBars = mainWindow->findChildren<QToolBar *>(QString(), Qt::FindDirectChildrenOnly);
        for (auto *toolBar : toolBars) {
            tryRegisterToolBar(mainWindow, toolBar);
        }

        if (QMenuBar *menuBar = mainWindow->menuBar()) {
            menuBar->setProperty("breeze_has_toolsarea_palette", true);
            menuBar->setPalette(_palette);
        }

        return;
    }

    auto parent = ptr;

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
    if (mainWindow.data() != mainWindow->window()) {
        return;
    }
    tryRegisterToolBar(mainWindow, widget);
}

void ToolsAreaManager::unregisterWidget(QWidget *widget)
{
    Q_ASSERT(widget);

    if (widget->property("breeze_has_toolsarea_palette").toBool()) {
        widget->setPalette({});
    }

    auto ptr = QPointer<QWidget>(widget);

    if (QPointer<const QMainWindow> window = qobject_cast<QMainWindow *>(ptr)) {
        removeWindow(window);
        return;
    } else if (QPointer<QToolBar> toolbar = qobject_cast<QToolBar *>(ptr)) {
        auto parent = ptr;
        QPointer<const QMainWindow> mainWindow = nullptr;
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
        removeWindowToolBar(mainWindow, toolbar);
    }
}

bool Breeze::ToolsAreaManager::hasHeaderColors()
{
    return _colorSchemeHasHeaderColor;
}
}
