/* This file is part of the KDE project
    SPDX-FileCopyrightText: 2016 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "breezestylechooser.h"

#ifdef Q_OS_WIN
#include <QSysInfo>
#endif
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QString>
#include <QStyle>
#include <QStyleFactory>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

WidgetStyleChooser::WidgetStyleChooser(QWidget *parent)
    : QPushButton(parent)
    , m_widgetStyle(QString())
{
}

KActionMenu *WidgetStyleChooser::createStyleSelectionMenu(const QString &text, const QString &selectedStyleName)
{
    QIcon icon = QIcon::fromTheme(QStringLiteral("preferences-desktop-theme"));
    KActionMenu *stylesAction = new KActionMenu(icon, text, this);
    setText(text);
    if (!icon.isNull()) {
        setIcon(icon);
    }
    stylesAction->setToolTip(i18n("Select the application widget style"));
    stylesAction->setStatusTip(stylesAction->toolTip());
    QActionGroup *stylesGroup = new QActionGroup(stylesAction);

    QStringList availableStyles = QStyleFactory::keys();
    QString desktopStyle = QApplication::style()->objectName();

    m_widgetStyle = selectedStyleName;
    bool setStyle = false;
    if (m_widgetStyle.isEmpty()) {
        m_widgetStyle = desktopStyle;
    } else if (selectedStyleName.compare(desktopStyle, Qt::CaseInsensitive)) {
        setStyle = true;
    }

    for (const QString &style : std::as_const(availableStyles)) {
        QAction *a = new QAction(style, stylesGroup);
        a->setCheckable(true);
        a->setData(style);
        if (m_widgetStyle.compare(style, Qt::CaseInsensitive) == 0) {
            a->setChecked(true);
            if (setStyle) {
                // selectedStyleName was not empty and the
                // the style exists: activate it.
                activateStyle(style);
            }
        }
        stylesAction->addAction(a);
    }
    connect(stylesGroup, &QActionGroup::triggered, this, [&](QAction *a) {
        activateStyle(a->data().toString());
    });

    setMenu(stylesAction->menu());

    return stylesAction;
}

QString WidgetStyleChooser::currentStyle() const
{
    return m_widgetStyle;
}

void WidgetStyleChooser::activateStyle(const QString &styleName)
{
    m_widgetStyle = styleName;
    QApplication::setStyle(QStyleFactory::create(m_widgetStyle));
}
