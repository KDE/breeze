#ifndef breezetoolsareamanager_h
#define breezetoolsareamanager_h

#include "breezehelper.h"
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

class ToolsAreaManager : public QObject
{
    Q_OBJECT

private:
    Helper *_helper;
    QHash<const QMainWindow *, QVector<QPointer<QToolBar>>> _windows;
    KSharedConfigPtr _config;
    KConfigWatcher::Ptr _watcher;
    QPalette _palette = QPalette();
    AppListener *_listener;
    bool _colorSchemeHasHeaderColor;
    bool _translucent;

    friend class AppListener;

protected:
    bool tryRegisterToolBar(QPointer<QMainWindow> window, QPointer<QWidget> widget);
    void tryUnregisterToolBar(QPointer<QMainWindow> window, QPointer<QWidget> widget);
    void configUpdated();

public:
    explicit ToolsAreaManager(Helper *helper, QObject *parent = nullptr);
    ~ToolsAreaManager();

    bool eventFilter(QObject *watched, QEvent *event) override;

    const QPalette &palette() const
    {
        return _palette;
    }

    void registerApplication(QApplication *application);
    void registerWidget(QWidget *widget);
    void unregisterWidget(QWidget *widget);

    QRect toolsAreaRect(const QMainWindow *window);

    bool hasHeaderColors();

    // sets the translucency of a window for translucent tools area purposes
    void becomeTransparent();
    void becomeOpaque();
};
}

#endif
