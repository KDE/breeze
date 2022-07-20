//////////////////////////////////////////////////////////////////////////////
// breezeanimationconfigitem.h
// animation configuration item
// -------------------
//
// SPDX-FileCopyrightText: 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include <QAbstractScrollArea>
#include <QApplication>
#include <QIcon>

#include <KCMultiDialog>
#include <KLocalizedString>

//__________________________________________
int main(int argc, char *argv[])
{
    KLocalizedString::setApplicationDomain("klassy_style_config");

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("klassy-settings"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("klassy-settings")));

    KCMultiDialog dialog;
    dialog.setWindowTitle(i18n("Klassy Settings"));
    dialog.addModule(QStringLiteral("klassystyleconfig"));
    dialog.addModule(QStringLiteral("klassydecorationconfig"));
    dialog.show();

    foreach (auto child, dialog.findChildren<QAbstractScrollArea *>()) {
        child->adjustSize();
        child->viewport()->adjustSize();
    }

    return app.exec();
}
