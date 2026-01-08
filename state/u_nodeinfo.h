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

#ifndef MWC_QT_WALLET_NODEINFO_H
#define MWC_QT_WALLET_NODEINFO_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../node/MwcNodeConfig.h"

namespace state {

class NodeInfo : public QObject, public State {
Q_OBJECT
public:
    NodeInfo(StateContext * context);
    virtual ~NodeInfo() override;

    void requestWalletResync();

    //node::MwcNode * getMwcNode() const;

    bool isNodeHealthy() const;

    void exportBlockchainData(QString fileName);
    void importBlockchainData(QString fileName);
    void publishTransaction(QString fileName, bool fluff);
    void resetEmbeddedNodeData();
protected:
    virtual NextStateRespond execute() override;
    virtual bool mobileBack() override {return false;}
    virtual QString getHelpDocName() override {return "node_overview.html";}

private slots:
private:
private:
    bool  justLogin = false;
};

}

#endif //MWC_QT_WALLET_NODEINFO_H
