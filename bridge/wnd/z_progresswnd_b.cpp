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

#include "z_progresswnd_b.h"
#include "../BridgeManager.h"
#include "../../state/a_initaccount.h"
#include "../../state/x_Resync.h"
#include "../../state/state.h"

namespace bridge {

ProgressWnd::ProgressWnd(QObject * parent) : QObject(parent) {
    bridge::getBridgeManager()->addProgressWnd(this);
}
ProgressWnd::~ProgressWnd() {
    bridge::getBridgeManager()->removeProgressWnd(this);
}

void ProgressWnd::setHeader(QString _callerId, QString header) {
    if (_callerId == callerId)
        emit sgnSetHeader(header);
}
void ProgressWnd::setMsgPlus(QString _callerId, QString msgPlus) {
    if (_callerId == callerId)
        emit sgnSetMsgPlus(msgPlus);
}

void ProgressWnd::initProgress(QString _callerId, int min, int max) {
    if (_callerId == callerId)
        emit sgnInitProgress(min,max);
}

void ProgressWnd::updateProgress(QString _callerId, int pos, QString msgProgress) {
    if (_callerId == callerId)
        emit sgnUpdateProgress(pos, msgProgress);
}

void ProgressWnd::cancelProgress() {
    if (callerId == state::INIT_ACCOUNT_CALLER_ID) {
        Q_ASSERT(false);
        return;
    }

    if (callerId == state::RESYNC_CALLER_ID) {
        Q_ASSERT(false);
        return;
    }

    // So far nobody expect cancel to be available, but it exist. Let's keep it
    Q_ASSERT(false);
}


}
