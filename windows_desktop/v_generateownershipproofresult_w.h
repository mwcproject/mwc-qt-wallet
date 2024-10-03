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

#ifndef V_GENERATEOWNERSHIPPROOFRESULT_W_H
#define V_GENERATEOWNERSHIPPROOFRESULT_W_H

#include "../core_desktop/navwnd.h"

namespace Ui {
class GenerateOwnershipProofResult;
}

namespace bridge {
class Wallet;
class ViewOutputs;
class Util;
}

namespace wnd {

class GenerateOwnershipProofResult : public core::NavWnd {
    Q_OBJECT
public:
    explicit GenerateOwnershipProofResult(QWidget *parent = nullptr);

    ~GenerateOwnershipProofResult();

private slots:
    void on_back_clicked();
    void on_save_clicked();
    void onGenerateOwnershipProof(QString proof, QString error);

private:
    Ui::GenerateOwnershipProofResult *ui;
    bridge::Wallet * wallet = nullptr;
    bridge::ViewOutputs * viewAcounts = nullptr;
    bridge::Util * util = nullptr;
};

}

#endif // V_GENERATEOWNERSHIPPROOFRESULT_W_H
