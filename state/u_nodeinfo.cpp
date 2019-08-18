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

#include "u_nodeinfo.h"
#include "../windows/u_nodeinfo_w.h"
#include "../wallet/wallet.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include <QDebug>
#include "../util/Log.h"

namespace state {

NodeInfo::NodeInfo(StateContext * context) :
        State(context, STATE::NODE_INFO )
{
    QObject::connect(context->wallet, &wallet::Wallet::onNodeStatus,
                     this, &NodeInfo::onNodeStatus, Qt::QueuedConnection);
}

NodeInfo::~NodeInfo() {

}


NextStateRespond NodeInfo::execute() {
    if ( context->appContext->getActiveWndState() != STATE::NODE_INFO )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    if (wnd==nullptr) {
        wnd = (wnd::NodeInfo*) context->wndManager->switchToWindowEx(new wnd::NodeInfo( context->wndManager->getInWndParent(), this));
    }

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}


void NodeInfo::requestWalletResync() {
    context->appContext->pushCookie("PrevState", (int)context->appContext->getActiveWndState() );
    context->stateMachine->setActionWindow( state::STATE::RESYNC );
}

void NodeInfo::requestNodeInfo() {
    context->wallet->getNodeStatus();
}

wallet::WalletConfig NodeInfo::getWalletConfig() const {
    return context->wallet->getWalletConfig();
}

void NodeInfo::updateWalletConfig( const wallet::WalletConfig & config ) {
    if ( context->wallet->setWalletConfig(config) ) {
        // config require to restart
        context->stateMachine->executeFrom( STATE::NONE );
    }
}

void NodeInfo::onNodeStatus( bool online, QString errMsg, int height, int64_t totalDifficulty, int connections ) {
    if (wnd== nullptr)
        return;

    wnd->setNodeStatus( online, errMsg, height, totalDifficulty, connections );
}


}
