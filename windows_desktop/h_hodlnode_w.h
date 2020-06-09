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

#include "../core_desktop/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class HodlNode;
}

namespace bridge {
class Hodl;
class HodlStatus;
class Util;
}

namespace wnd {

class HodlNode : public core::NavWnd
{
    Q_OBJECT

public:
    explicit HodlNode(QWidget *parent);
    ~HodlNode();

private slots:
    void on_signInButton_clicked();
    void on_publicKey_textChanged(const QString &arg1);
    void on_viewOutputsButton_clicked();

    void onSgnUpdateHodlState();
    void onSgnReportMessage(QString title, QString message);
    void onSgnHideWaitingStatus();
private:
    Ui::HodlNode *ui;
    bridge::Hodl * hodl = nullptr;
    bridge::HodlStatus * hodlStatus = nullptr;
    bridge::Util * util = nullptr;
};

}

#endif // H_HODLNODE_W_H
