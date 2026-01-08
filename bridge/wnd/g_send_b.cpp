// Copyright 2020 The MWC Developers
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

#include "g_send_b.h"
#include "../BridgeManager.h"
#include "../../state/state.h"
#include "../../state/g_Send.h"
#include "../../state/u_nodeinfo.h"
#include "../../util/Log.h"

namespace bridge {

static state::Send * getState() {return (state::Send *) state::getState(state::STATE::SEND);}
static state::NodeInfo * getNodeInfo() { return (state::NodeInfo *) state::getState(state::STATE::NODE_INFO); }

Send::Send(QObject * parent) :
    QObject(parent){
    getBridgeManager()->addSend(this);
}

Send::~Send() {
    getBridgeManager()->removeSend(this);
}

// Process sept 1 send request.  sendAmount is a value as user input it
// sendSelectedMethod values:
//   ONLINE_ID = 1, FILE_ID = 2, SLATEPACK_ID = 2
// return code:
//   0 - ok
//   1 - account error
//   2 - amount error
int Send::initialSendSelection( int sendSelectedMethod, QString accountPath, QString sendAmount, bool gotoNextPage ) {
    logger::logInfo(logger::BRIDGE, "Call Send::initialSendSelection with sendSelectedMethod=" + QString::number(sendSelectedMethod) +
                " accountPath=" + accountPath + " sendAmount=" + sendAmount + " gotoNextPage=" + QString(gotoNextPage ? "true" : "false"));
    return getState()->initialSendSelection( bridge::SEND_SELECTED_METHOD(sendSelectedMethod), accountPath, sendAmount, gotoNextPage);
}

int64_t Send::getTmpAmount() {
    logger::logInfo(logger::BRIDGE, "Call Send::getTmpAmount");
    return getState()->getTmpAmount();
}

QString Send::getTmpAccountPath() {
    logger::logInfo(logger::BRIDGE, "Call Send::getTmpAccountPath");
    return getState()->getTmpAccountPath();
}


void Send::showSendResult( bool success, QString message ) {
    logger::logInfo(logger::BRIDGE, QString("Call Send::showSendResult success=") + (success?"true":"false") + " <message>" );
    emit sgnShowSendResult(success, message);
}

bool Send::isNodeHealthy() {
    logger::logInfo(logger::BRIDGE, "Call Send::isNodeHealthy");
    return getNodeInfo()->isNodeHealthy();
}

// Handle whole workflow to send offline
// return true if some long process was started.
bool Send::sendMwcOffline( QString account, QString accountPath, QString amountNano, QString message, bool isLockLater, QString slatepackRecipientAddress) {
    logger::logInfo(logger::BRIDGE, "Call Send::sendMwcOffline with accountPath=" + accountPath + " amountNano=" + amountNano +
        " <message> " + " isLockLater=" + QString(isLockLater ? "true" : "false") + " slatepackRecipientAddress=" + (slatepackRecipientAddress.isEmpty() ? "<empty>" : "<hidden>"));
    return getState()->sendMwcOffline( account, accountPath, amountNano.toLongLong(), message, isLockLater, slatepackRecipientAddress);
}

// Handle whole workflow to send online
// return true if some long process was started.
bool Send::sendMwcOnline( QString account, QString accountPath, QString amountNano, QString address, QString message) {
    logger::logInfo(logger::BRIDGE, "Call Send::sendMwcOnline with accountPath=" + accountPath + " amountNano=" + amountNano +
            " address=" + (address.isEmpty() ? "<empty>" : "<hidden>") + " <message>");
    return getState()->sendMwcOnline( account, accountPath, amountNano.toLongLong(), address, message);
}

QString Send::getSpendAllAmount( QString accountPath) {
    logger::logInfo(logger::BRIDGE, "Call Send::getSpendAllAmount with account=" + accountPath);
    return getState()->getSpendAllAmount(accountPath);
}


}
