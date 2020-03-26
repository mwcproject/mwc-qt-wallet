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

#ifndef H_HODLNODE_W_H
#define H_HODLNODE_W_H

#include "../core/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class HodlNode;
}

namespace state {
    class Hodl;
}

namespace wnd {

class HodlNode : public core::NavWnd
{
    Q_OBJECT

public:
    explicit HodlNode(QWidget *parent, state::Hodl * state );
    ~HodlNode();

    // Hodl object changed it's state, need to refresh
    void updateHodlState();

    void reportMessage(const QString & title, const QString & message);
    void hideWaitingStatus();
private slots:
    void on_signInButton_clicked();

    void on_claimMwcButton_clicked();

    void on_publicKey_textChanged(const QString &arg1);

    void on_viewOutputsButton_clicked();

private:
    Ui::HodlNode *ui;
    state::Hodl * state;
};

}

#endif // H_HODLNODE_W_H
