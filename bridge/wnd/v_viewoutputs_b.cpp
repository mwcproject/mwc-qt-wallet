// Copyright 2021 The MWC Developers
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

//
// Created by Konstantin Bay on 9/23/24.
//

#include "v_viewoutputs_b.h"
#include "../BridgeManager.h"
#include "../../state/v_ViewOutputs.h"
#include "../../util/stringutils.h"

namespace bridge {

static state::ViewOutputs * getState() {return (state::ViewOutputs *) state::getState(state::STATE::VIEW_ACCOUNTS);}

QString ViewOutputs::lastViewingKey;


ViewOutputs::ViewOutputs(QObject *parent) :
        QObject(parent) {
    getBridgeManager()->addViewAccounts(this);
}

ViewOutputs::~ViewOutputs() {
    getBridgeManager()->removeViewAccounts(this);
}

// Switching to the progress window
void ViewOutputs::startScanning(QString hashKey) {
    lastViewingKey = hashKey;
    getState()->startScanning(hashKey);
}

void ViewOutputs::backFromOutputsView() {
    getState()->backFromOutputsView();
}

void ViewOutputs::setSgnViewOutputs(const QVector<wallet::WalletOutput> & outputResult, int64_t total ) {
    QVector<QString> outputs;
    for (const wallet::WalletOutput & out : outputResult) {
        outputs.push_back( out.toJson() );
    }

    emit onSgnViewOutputs( lastViewingKey, outputs, util::zeroDbl2Dbl(util::nano2one(total)));
}


}
