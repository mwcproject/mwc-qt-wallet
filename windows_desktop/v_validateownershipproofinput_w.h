// Copyright 2024 The MWC Developers
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

#ifndef V_VALIDATEOWNERSHIPPROOFINPUT_W_H
#define V_VALIDATEOWNERSHIPPROOFINPUT_W_H

#include "../core_desktop/navwnd.h"

namespace Ui {
class ValidateOwnershipProofInput;
}

namespace bridge {
class Wallet;
class ViewOutputs;
class Util;
}

namespace wnd {

class ValidateOwnershipProofInput : public core::NavWnd {
Q_OBJECT

public:
    explicit ValidateOwnershipProofInput(QWidget *parent = nullptr);

    ~ValidateOwnershipProofInput();

private:
    void updateButtons();

private slots:
    void on_proof_text_textChanged();
    void on_back_clicked();
    void on_load_from_file_clicked();
    void on_validate_clicked();
private:
    Ui::ValidateOwnershipProofInput *ui;
    bridge::Wallet * wallet = nullptr;
    bridge::ViewOutputs * viewAcounts = nullptr;
    bridge::Util * util = nullptr;
};

}

#endif // V_VALIDATEOWNERSHIPPROOFINPUT_W_H
