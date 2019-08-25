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

namespace wnd {
class NodeInfo;
}

namespace state {

struct NodeStatus {
    bool online = false;
    QString errMsg;
    int nodeHeight = 0;
    int peerHeight = 0;
    int64_t totalDifficulty = 0;
    int connections = 0;

    void setData(bool online,
            const QString & errMsg,
            int nodeHeight,
            int peerHeight,
            int64_t totalDifficulty,
            int connections);
};


class NodeInfo : public QObject, public State {
Q_OBJECT
public:
    NodeInfo(StateContext * context);
    virtual ~NodeInfo() override;

    void wndIsGone(wnd::NodeInfo * w) { if(w==wnd) wnd = nullptr;} // window is closed

    void requestNodeInfo();

    void requestWalletResync();

    wallet::WalletConfig getWalletConfig() const;
    void updateWalletConfig( const wallet::WalletConfig & config );

protected:
    virtual NextStateRespond execute() override;
    virtual QString getHelpDocName() override {return "node_overview.html";}

private slots:
    void onLoginResult(bool ok);

    void onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections );

private:
    virtual void timerEvent(QTimerEvent *event) override;

private:
    wnd::NodeInfo * wnd = nullptr;
    bool  justLogin = false;
    NodeStatus lastNodeStatus;
};

}

#endif //MWC_QT_WALLET_NODEINFO_H
