// Copyright 2019 The MWC Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef MWC_QT_WALLET_NAVCONTROLS_H
#define MWC_QT_WALLET_NAVCONTROLS_H

#include <QPushButton>
#include <QToolButton>

// Navigation controls: Tool Button (NavControls) and menu button (NavMenuButton)
namespace control {

class NavPushButton : public QToolButton {
    Q_OBJECT
public:
    explicit NavPushButton(QWidget *parent = Q_NULLPTR);
    explicit NavPushButton(const QString &text, QWidget *parent = Q_NULLPTR);
    NavPushButton(const QIcon &icon, const QString &text, QWidget *parent = Q_NULLPTR);
    virtual ~NavPushButton() override;
};

class NavMenuButton : public QPushButton {
Q_OBJECT
public:
    explicit NavMenuButton(QWidget *parent = Q_NULLPTR);
    explicit NavMenuButton(const QString &text, QWidget *parent = Q_NULLPTR);
    NavMenuButton(const QIcon &icon, const QString &text, QWidget *parent = Q_NULLPTR);
    virtual ~NavMenuButton() override;
};

}

#endif //MWC_QT_WALLET_NAVCONTROLS_H
