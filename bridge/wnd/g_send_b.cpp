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

namespace bridge {

static state::Send * getState() {return (state::Send *) state::getState(state::STATE::SEND);}

Send::Send(QObject * parent) :
    QObject(parent){
    getBridgeManager()->addSend(this);
}

Send::~Send() {
    getBridgeManager()->removeSend(this);
}

// Process sept 1 send request.  sendAmount is a value as user input it
// return code:
//   0 - ok
//   1 - account error
//   2 - amount error
int Send::initialSendSelection( bool isOnlineSelected, QString account, QString sendAmount ) {
    return getState()->initialSendSelection(isOnlineSelected, account, sendAmount);
}


void Send::showSendResult( bool success, QString message ) {
    emit sgnShowSendResult(success, message);
}


bool Send::isNodeHealthy() {
    return getState()->isNodeHealthy();
}

// Handle whole workflow to send offline
// return true if some long process was started.
bool Send::sendMwcOffline( QString account, QString amountNano, QString message) {
    return getState()->sendMwcOffline( account, amountNano.toLongLong(), message);
}

// Handle whole workflow to send online
// return true if some long process was started.
bool Send::sendMwcOnline( QString account, QString amountNano, QString address, QString apiSecret, QString message) {
    return getState()->sendMwcOnline( account, amountNano.toLongLong(), address, apiSecret, message);
}


}
