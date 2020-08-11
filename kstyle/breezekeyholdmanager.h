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

#ifndef breezekeyholdmanager_h
#define breezekeyholdmanager_h

#include <QApplication>
#include <QObject>

namespace Breeze {

class KeyHoldManager : public QObject
{
    Q_OBJECT

private:
    void cleanUpState();
    bool objectIsTextField(QObject *watched);
    void applyReplacement(QObject *on, const QString& data);
    void showPopup(QObject *on, const QString& text);

public:
    KeyHoldManager(QObject *parent = nullptr);
    void registerApplication(QApplication *application);
    void unregisterApplication(QApplication *application);
    bool eventFilter(QObject *watched, QEvent *event) override;

};

}

#endif