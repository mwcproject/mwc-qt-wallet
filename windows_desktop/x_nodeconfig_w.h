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

#ifndef X_NODECONFIG_W_H
#define X_NODECONFIG_W_H

#include "../core_desktop/navwnd.h"

namespace Ui {
class NodeConfig;
}

namespace bridge {
class Config;
class WalletConfig;
}

namespace wnd {

// Config panel for the Online_Node running mode
class NodeConfig : public core::NavWnd
{
    Q_OBJECT

public:
    explicit NodeConfig(QWidget *parent);
    virtual ~NodeConfig() override;

    bool askUserForChanges();
private slots:
    void on_radioMainNet_clicked();
    void on_radioFloonet_clicked();
    void on_applyButton_clicked();

private:
    void updateApplyBtn();
    QString getSelectedNetwork() const;

    bool applyChanges();
private:
    Ui::NodeConfig          *ui = nullptr;
    bridge::Config * config = nullptr;
    bridge::WalletConfig * walletConfig = nullptr;
};

}

#endif // X_NODECONFIG_W_H
