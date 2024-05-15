#ifndef breezetabwidget_h
#define breezetabwidget_h

//////////////////////////////////////////////////////////////////////////////
// breezetabwidget.h
// -------------------
//
// SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include <QIcon>
#include <QTabBar>
#include <QTabWidget>

namespace Breeze
{
class TabWidget : public QTabWidget
{
    Q_OBJECT

public:
    //! constructor
    explicit TabWidget(QWidget *parent)
        : QTabWidget(parent)
    {
        tabBar()->setMovable(true);
    }

    // adjust tabbar size
    void adjustTabBarSize(void)
    {
        if (tabBar())
            tabBar()->adjustSize();
    }

    //! show icons
    void showIcons(void)
    {
        // add icons to tabs
        tabBar()->setTabIcon(0, QIcon::fromTheme(QStringLiteral("document-open-folder")));
        tabBar()->setTabIcon(1, QIcon::fromTheme(QStringLiteral("document-open-folder")));
        tabBar()->setTabIcon(2, QIcon::fromTheme(QStringLiteral("document-open-folder")));
        tabBar()->setTabIcon(3, QIcon::fromTheme(QStringLiteral("document-open-folder")));
    }

    void hideIcons(void)
    {
        // add icons to tabs
        tabBar()->setTabIcon(0, QIcon());
        tabBar()->setTabIcon(1, QIcon());
        tabBar()->setTabIcon(2, QIcon());
        tabBar()->setTabIcon(3, QIcon());
    }

    void showText(void)
    {
        tabBar()->setTabText(0, QStringLiteral("First Tab"));
        tabBar()->setTabText(1, QStringLiteral("Second Tab"));
        tabBar()->setTabText(2, QStringLiteral("Third Tab"));
        tabBar()->setTabText(3, QStringLiteral("Fourth Tab"));
    }

    void hideText(void)
    {
        tabBar()->setTabText(0, QString());
        tabBar()->setTabText(1, QString());
        tabBar()->setTabText(2, QString());
        tabBar()->setTabText(3, QString());
    }

public Q_SLOTS:

    // toggle tabbar visibility
    void toggleTabBarVisibility(bool value)
    {
        if (tabBar())
            tabBar()->setVisible(!value);
    }
};
}

#endif
