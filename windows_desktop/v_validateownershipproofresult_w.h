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


#ifndef V_VALIDATEOWNERSHIPPROOFRESULT_W_H
#define V_VALIDATEOWNERSHIPPROOFRESULT_W_H

#include "../core_desktop/navwnd.h"

namespace Ui {
class ValidateOwnershipProofResult;
}

namespace bridge {
class Wallet;
class ViewOutputs;
}


namespace wnd {

class ValidateOwnershipProofResult : public core::NavWnd
{
    Q_OBJECT
public:
    explicit ValidateOwnershipProofResult(QWidget *parent = nullptr);
    ~ValidateOwnershipProofResult();

private slots:
    void on_back_clicked();
    void sgnValidateOwnershipProof(QString network, QString message, QString viewingKey, QString torAddress, QString mqsAddress, QString error);

private:
    Ui::ValidateOwnershipProofResult *ui;
    bridge::Wallet * wallet = nullptr;
    bridge::ViewOutputs * viewAcounts = nullptr;
};

}

#endif // V_VALIDATEOWNERSHIPPROOFRESULT_W_H
