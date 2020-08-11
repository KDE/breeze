/*************************************************************************
 * Copyright (C) 2020 by Carson Black <uhhadd@gmail.com>                 *
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program; if not, write to the                         *
 * Free Software Foundation, Inc.,                                       *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 *************************************************************************/

/// Qt Utilities
#include <QDebug>
#include <QKeyEvent>
#include <QQuickWindow>
#include <QStringBuilder>
#include <QToolTip>

/// The widgets that we support the key hold for
#include <QLineEdit>
#include <QTextEdit>
#include <QQuickItem>

#include "breezekeyholdmanager.h"
#include "config-breeze.h"

#define qCoreApp QCoreApplication::instance()

namespace Breeze {

const QMap<QChar,QList<QString>> s_keyMappings = {
    //
    // Latin
    //
    {'e', {"è", "é", "ê", "ë", "ē", "ė", "ę"}},
    {'y', {"ÿ", "ұ", "ү", "ӯ", "ў"}},
    {'u', {"û", "ü", "ù", "ú", "ū"}},
    {'i', {"î", "ï", "í", "ī", "į", "ì"}},
    {'o', {"ô", "ö", "ò", "ó", "œ", "ø", "ō", "õ"}},
    {'a', {"à", "á", "â", "ä", "æ", "ã", "å", "ā"}},
    {'s', {"ß", "ś", "š"}},
    {'l', {"ł"}},
    {'z', {"ž", "ź", "ż"}},
    {'c', {"ç", "ć", "č"}},
    {'n', {"ñ", "ń"}},
    {'d', {"ð"}},
    //
    // Cyrilic
    //
    {'й', {"ј"}}, // ј is not j
    {'к', {"қ", "ҝ",}},
    {'е', {"ё"}}, // this in fact NOT the same E as before
    {'н', {"ң", "һ"}}, // һ is not h
    {'г', {"ғ"}},
    {'о', {"ә", "ө"}},
    {'ч', {"ҷ", "ҹ"}},
    {'и', {"ӣ", "і"}}, // і is not i
    {'ь', {"ъ"}},
    //
    // Arabic
    //
    // This renders weirdly in text editors, but is valid code.
    {'و', {"ؤ"}},
    {'ه', {"ه"}},
    {'ف', {"ڤ"}},
    {'ج', {"چ"}},
    {'ك', {"گ"}},
    {'ل', {"لا"}},
    {'ز', {"ژ"}},
    {'ب', {"پ"}},
    {'ا', {"أ", "إ", "آ", "ء"}},
};

static QString tooltipForIndex(QChar ch, bool upperCase)
{
    const QString tableFormat = QStringLiteral(R"RJIENRLWEY(
        <table>
            <tr>%1</tr>
            <tr>%2</tr>
        </table>
    )RJIENRLWEY");
    const QString charFormat = QStringLiteral("<td style=\"padding: 4px; font-weight: bold;\">%1</td>");
    const QString numFormat = QStringLiteral("<td style=\"padding: 4px; opacity: 0.7;\">%1</td>");
    QString rowOne;
    QString rowTwo;
    int i = 1;
    for (auto item : s_keyMappings[ch]) {
        rowOne.append(charFormat.arg(upperCase ? item.toUpper() : item));
        rowTwo.append(numFormat.arg(i));
        i++;
    }
    return tableFormat.arg(rowOne, rowTwo);
}

const char* isPreHold = "__breeze_isPreHold";
const char* preHoldText = "__breeze_preHoldText";

KeyHoldManager::KeyHoldManager(QObject *parent) : QObject(parent) {}

void KeyHoldManager::registerApplication(QApplication* app)
{
    app->installEventFilter(this);
}

void KeyHoldManager::unregisterApplication(QApplication* app)
{
    cleanUpState();
    app->removeEventFilter(this);
}

void KeyHoldManager::cleanUpState()
{
    auto app = qobject_cast<QApplication*>(qApp);
    if (!app) return;

    QToolTip::hideText();
    app->setProperty(isPreHold, false);
    app->setProperty(preHoldText, QVariant());
}

bool KeyHoldManager::objectIsTextField(QObject *watched)
{
    if (qobject_cast<QLineEdit*>(watched)
     || qobject_cast<QTextEdit*>(watched)) return true;

    if (watched->property("cursorRectangle").isValid()) return true;

    return false;
}

void KeyHoldManager::applyReplacement(QObject *on, const QString& data)
{
    if (auto le = qobject_cast<QLineEdit*>(on)) {
        le->backspace();
        le->insert(data);
    } else if (auto te = qobject_cast<QTextEdit*>(on)) {
        te->textCursor().deletePreviousChar();
        te->textCursor().insertText(data);
    } else if (on->property("cursorRectangle").isValid()) {
        QMetaObject::invokeMethod(
            on, "remove", Qt::DirectConnection,
            Q_ARG(int, on->property("cursorPosition").toInt()-1),
            Q_ARG(int, on->property("cursorPosition").toInt())
        );
        QMetaObject::invokeMethod(
            on, "insert", Qt::DirectConnection,
            Q_ARG(int, on->property("cursorPosition").toInt()),
            Q_ARG(QString, data)
        );
    }
}

void KeyHoldManager::showPopup(QObject *on, const QString& text)
{
    if (auto le = qobject_cast<QLineEdit*>(on)) {
        auto rect = le->rect();
        auto mapped = QRect(le->mapToGlobal(rect.topLeft()), rect.size());
        mapped.setBottom(mapped.top() - mapped.height());
        QToolTip::showText(
            mapped.bottomRight(),
            text,
            le,
            QRect()
        );
    } else if (auto textEdit = qobject_cast<QTextEdit*>(on)) {
        QToolTip::showText(
            textEdit->mapToGlobal(textEdit->cursorRect().topRight()),
            text,
            textEdit,
            QRect()
        );
    } else if (on->property("cursorRectangle").isValid()) {
        auto item = qobject_cast<QQuickItem*>(on);
        if (!item) return;
        auto rect = on->property("cursorRectangle").toRectF();
        QToolTip::showText(
            item->mapToGlobal(rect.topRight()).toPoint(),
            text,
            nullptr,
            QRect()
        );
    }
}


bool KeyHoldManager::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        if (!objectIsTextField(watched)) return false;

        auto ev = static_cast<QKeyEvent*>(event);

        // this is the state when we have a held key
        if (qCoreApp->property(isPreHold).toBool()) {
            if (ev->isAutoRepeat() && ev->text() == qCoreApp->property(preHoldText).toString()) return true;

            if (ev->key() < 0x31 || 0x39 < ev->key()) { cleanUpState(); return false; }

            auto str = qCoreApp->property(preHoldText).toString();
            QChar first = str.at(0);
            bool isUpper = first.isUpper();
            first = first.toLower();

            int key = ev->key() - 0x30;
            if (s_keyMappings[first].count() < key) { cleanUpState(); return false; }

            auto data = s_keyMappings[first][key-1];
            applyReplacement(watched, isUpper ? data.toUpper() : data);
            qCoreApp->setProperty(isPreHold, false);
            qCoreApp->setProperty(preHoldText, QString());
            QToolTip::hideText();
            return true;
        }

        // this is the state before we have a held key
        if (ev->key() < 0x41 || 0x5a < ev->key()) { cleanUpState(); return false; }
        if (ev->isAutoRepeat()) {
            if (!qCoreApp->property(isPreHold).toBool()) {
                if (ev->text().isEmpty()) return false;
                if (!s_keyMappings.contains(ev->text().at(0).toLower())) return false;

                auto tooltipText = tooltipForIndex(ev->text().at(0).toLower(), ev->text().at(0).isUpper());
                showPopup(watched, tooltipText);

                qCoreApp->setProperty(preHoldText, ev->text());
                qCoreApp->setProperty(isPreHold, true);
                return true;
            }
        }

        cleanUpState();
    }
    return false;
}

}