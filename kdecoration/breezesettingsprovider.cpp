/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "breezesettingsprovider.h"

#include "decorationexceptionlist.h"

#include <KWindowInfo>

#include <QRegularExpression>
#include <QTextStream>

namespace Breeze
{

SettingsProvider *SettingsProvider::s_self = nullptr;

//__________________________________________________________________
SettingsProvider::SettingsProvider()
    : m_config(KSharedConfig::openConfig(QStringLiteral("klassyrc")))
{
    reconfigure();
}

//__________________________________________________________________
SettingsProvider::~SettingsProvider()
{
    s_self = nullptr;
}

//__________________________________________________________________
SettingsProvider *SettingsProvider::self()
{
    // TODO: this is not thread safe!
    if (!s_self) {
        s_self = new SettingsProvider();
    }

    return s_self;
}

//__________________________________________________________________
void SettingsProvider::reconfigure()
{
    if (!m_defaultSettings) {
        m_defaultSettings = InternalSettingsPtr(new InternalSettings());
        m_defaultSettings->setCurrentGroup(QStringLiteral("Windeco"));
    }

    m_defaultSettings->load();

    DecorationExceptionList exceptions;
    exceptions.readConfig(m_config);
    m_exceptions = exceptions.getDefault();
    m_exceptions.append(exceptions.get());
}

//__________________________________________________________________
InternalSettingsPtr SettingsProvider::internalSettings(Decoration *decoration) const
{
    // get the client
    auto client = decoration->client().toStrongRef();
    Q_ASSERT(client);

    foreach (auto internalSettings, m_exceptions) {
        // discard disabled exceptions
        if (!internalSettings->enabled())
            continue;

        // discard exceptions with empty exception pattern
        if (internalSettings->exceptionWindowPropertyPattern().isEmpty())
            continue;

        /*
        decide which windowPropertyValue is to be compared
        to the regular expression, based on exception type
        */
        QString windowPropertyValue;
        switch (internalSettings->exceptionWindowPropertyType()) {
        case InternalSettings::ExceptionWindowTitle: {
            windowPropertyValue = client->caption();
            break;
        }

        default:
        case InternalSettings::ExceptionWindowClassName: {
            // retrieve class name
            KWindowInfo info(client->windowId(), {}, NET::WM2WindowClass);
            QString window_className(QString::fromUtf8(info.windowClassName()));
            QString window_class(QString::fromUtf8(info.windowClassClass()));
            windowPropertyValue = window_className + QStringLiteral(" ") + window_class;

            break;
        }
        }

        // check matching
        QRegularExpression rx(internalSettings->exceptionWindowPropertyPattern());
        if (rx.match(windowPropertyValue).hasMatch()) {
            return internalSettings;
        }
    }

    return m_defaultSettings;
}

}
