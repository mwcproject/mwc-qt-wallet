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

#ifndef V_VIEWHASH_W_H
#define V_VIEWHASH_W_H

#include "../core_desktop/navwnd.h"

namespace Ui {
class ViewHash;
}

namespace bridge {
class Wallet;
class ViewOutputs;
}


namespace wnd {

class ViewHash : public core::NavWnd
{
    Q_OBJECT
public:
    explicit ViewHash(QWidget *parent = nullptr);
    ~ViewHash();

private slots:
    void on_showWalletKeyButton_clicked();
    void on_startScanning_clicked();
    void on_viewingKey_textChanged(const QString &arg1);
    void on_generateOwnershipProof_clicked();
    void on_viewOwnershipProof_clicked();
private:
    Ui::ViewHash *ui;

    bridge::Wallet * wallet = nullptr;
    bridge::ViewOutputs * viewAcounts = nullptr;
};

}

#endif // V_VIEWHASH_W_H
