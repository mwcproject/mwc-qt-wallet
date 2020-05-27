// Copyright 2020 The MWC Developers
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

#ifndef MWC_QT_WALLET_PANELBASEWND_H
#define MWC_QT_WALLET_PANELBASEWND_H

#include <QWidget>

namespace state {
class StateMachine;
}

#ifdef Q_OS_DARWIN
namespace Cocoa
{
    void changeTitleBarColor(WId winId, double red, double green, double blue);
}
#endif

namespace core {

// Base calss for all panels. those classes cover:
// Title update for MacOS. Need to be done because of QT bugs
// Activity monitoring
class PanelBaseWnd : public QWidget {
    Q_OBJECT
public:
    PanelBaseWnd(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

private slots:

    void updateAppTitle();

};

}

#endif //MWC_QT_WALLET_PANELBASEWND_H
