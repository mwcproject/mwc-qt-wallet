// Copyright 2021 The MWC Developers
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

#ifndef G_RESULTEDSLATEPACK_W_H
#define G_RESULTEDSLATEPACK_W_H

#include "../core_desktop/navwnd.h"

namespace Ui {
class ResultedSlatepack;
}

namespace bridge {
class StateMachine;
class Config;
class Util;
}

namespace control {
class QrCodeWidget;
}

namespace wnd {

class ResultedSlatepack : public core::NavWnd {
Q_OBJECT

public:
    // txExtention - extention for the slatepack file. Let's keep the same as a regular slate
    explicit ResultedSlatepack(QWidget *parent, QString slatepack, int backStateId, QString txExtension );

    ~ResultedSlatepack();

private slots:
    void on_backButton_clicked();
    void on_saveQrImageBtn_clicked();
    void on_saveSlatepackBtn_clicked();
    void on_backButton_2_clicked();

private:
    Ui::ResultedSlatepack *ui;
    bridge::StateMachine * stateMachine = nullptr;
    bridge::Config * config = nullptr;
    bridge::Util * util = nullptr;

    int backStateId = -1;
    QString slatepack;
    QString txExtension;
};

}

#endif // G_RESULTEDSLATEPACK_W_H
