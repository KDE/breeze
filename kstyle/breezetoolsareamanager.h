#pragma once

#include "breezestyle.h"
#include <KConfigWatcher>
#include <KSharedConfig>
#include <QApplication>
#include <QObject>

namespace Breeze
{
class ToolsAreaManager;

// Trying to discriminate QApplication events from events from all QObjects
// belonging to it is impractical with everything going through a single
// eventFilter, so we have this class which provides a second one that allows
// us to filter for the events we want.
class AppListener : public QObject
{
    Q_OBJECT
    using QObject::QObject;

    bool eventFilter(QObject *watched, QEvent *event) override;

    ToolsAreaManager *manager;
    friend class ToolsAreaManager;
};

//* signal manager for the tools area
class ToolsAreaManager : public QObject
{
    Q_OBJECT

private:
    QHash<const QMainWindow *, QVector<QPointer<QWidget>>> _windows;
    KSharedConfigPtr _config;
    KConfigWatcher::Ptr _watcher;
    QPalette _palette = QPalette();
    AppListener *_listener;
    bool _colorSchemeHasHeaderColor;

    friend class AppListener;

protected:
    bool tryRegister(QPointer<QMainWindow> window, QPointer<QWidget> widget);
    void tryUnregister(QPointer<QMainWindow> window, QPointer<QWidget> widget);
    void configUpdated();

public:
    explicit ToolsAreaManager();
    ~ToolsAreaManager();

    const QPalette &palette() const
    {
        return _palette;
    }

    void registerApplication(QApplication *application);
    void registerWidget(QWidget *widget);
    void unregisterWidget(QWidget *widget);

    QRect toolsAreaRect(const QMainWindow *window);

    bool hasHeaderColors();

    QHash<const QMainWindow *, QRect> previousRects;
};
}
