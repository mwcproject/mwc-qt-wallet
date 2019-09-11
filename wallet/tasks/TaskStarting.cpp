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

#include "TaskStarting.h"
#include "../mwc713.h"
#include <QDebug>
#include "TaskWallet.h"

namespace wallet {

bool TaskStarting::processTask(const QVector<WEvent> & events) {
    // Expected:  sWelcome with version= "2.0.0"
    // sNeed2Unlock + sPasswordError + sGenericError  OR  sInit

    qDebug() << "TaskStarting::processTask with events: " << printEvents(events);

    while (true) {
        // happy path

        QVector< WEvent > welcome = filterEvents(events, WALLET_EVENTS::S_WELCOME );
        QVector< WEvent > need2unlock = filterEvents(events, WALLET_EVENTS::S_NEED2UNLOCK );
        QVector< WEvent > init = filterEvents(events, WALLET_EVENTS::S_INIT );

        if (welcome.empty())
            break;

        if (need2unlock.empty() && init.empty())
            break;

        // check version of the wallet
        /*if (welcome[0].message != mwc::MWC_713_VERSION) {
            wallet713->appendNotificationMessage(MWC713::MESSAGE_LEVEL::FATAL_ERROR, MWC713::MESSAGE_ID::GENERIC,
                    "Backed wallet713 has incorrect error. Expected v" + mwc::MWC_713_VERSION + ", but found v" + welcome[0].message);
            return true;
        }*/

        if ( !init.empty() ) {
            // wallet need to be provisioned
            wallet713->appendNotificationMessage(MWC713::MESSAGE_LEVEL::FATAL_ERROR, MWC713::MESSAGE_ID::GENERIC,
                                                 "Wallet mwc713 in non initialized state. Internal error.");
            return true; // Done. Now it is UI problem to provision the wallet
        }

        if (need2unlock.empty()) {
            wallet713->appendNotificationMessage(MWC713::MESSAGE_LEVEL::FATAL_ERROR, MWC713::MESSAGE_ID::GENERIC,
                                                 "Wallet mwc713 has a seed without a password. Did you initialize it manually? "
                                                 "To fix this problem you will need to reinitialize your wallet with a passphrase and a password.");
            return true;
        }

        // It is a normal workflow, password does required
        return true;
    }

    // Failure path. Just report a error
    wallet713->appendNotificationMessage(MWC713::MESSAGE_LEVEL::FATAL_ERROR, MWC713::MESSAGE_ID::GENERIC,
            "Unable to start backed wallet713. Please reinstall this app or clean up its data");
    return true;
}

}
