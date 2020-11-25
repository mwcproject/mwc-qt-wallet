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

#ifndef S_NEWSWAP3_W_H
#define S_NEWSWAP3_W_H

#include "../core_desktop/navwnd.h"

namespace Ui {
class NewSwap3;
}

namespace bridge {
class Swap;
class Util;
}

namespace wnd {

class NewSwap3 : public core::NavWnd {
Q_OBJECT

public:
    explicit NewSwap3(QWidget *parent);
    ~NewSwap3();

private slots:
    void onSgnCreateStartSwap(bool ok, QString errorMessage);

    void on_backButton_clicked();
    void on_sendOfferButton_clicked();

    void on_noteEdit_textEdited(const QString &arg1);

private:
    void updateSwapReview();

private:
    Ui::NewSwap3 *ui;
    bridge::Swap * swap = nullptr;
    bridge::Util * util = nullptr;
};

}

#endif // S_NEWSWAP3_W_H
