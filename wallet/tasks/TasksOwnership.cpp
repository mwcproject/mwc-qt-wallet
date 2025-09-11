// Copyright 2024 The MWC Developers
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

#include "TasksOwnership.h"
#include "TaskWallet.h"
#include "../mwc713.h"
#include "tries/mwc713inputparser.h"

namespace wallet {

////////////////////////////////////////////////////
// TasksGenerateOwnership

bool TasksGenerateOwnership::processTask(const QVector<WEvent> &events) {
    QString errors;
    QString proof;

    for ( const auto & evt : events ) {
        if (evt.event != WALLET_EVENTS::S_LINE)
            continue;


        QString ln = evt.message;
        if (ln.contains("Error", Qt::CaseInsensitive)) {
            if (errors.isEmpty())
                errors += "\n";
            errors += tries::mapMwc713Message(ln);
        }

        if (ln.startsWith("Ownership Proof:")) {
            proof = ln.mid(strlen("Ownership Proof:")).trimmed();
        }
    }

    wallet713->setGenerateOwnershipProof(proof, errors );
    return true;
}

/////////////////////////////////////////////////////////////////////
/// TasksValidateOwnership

bool TasksValidateOwnership::processTask(const QVector<WEvent> & events) {
    QString errors;
    QString network;
    QString message;
    QString viewingKey;
    QString torAddress;
    QString mqsAddress;

    for ( const auto & evt : events ) {
        if (evt.event != WALLET_EVENTS::S_LINE)
            continue;

        QString ln = evt.message;
        if (ln.contains("Error", Qt::CaseInsensitive)) {
            if (errors.isEmpty())
                errors += "\n";
            errors += tries::mapMwc713Message(ln);
        }

        if (ln.startsWith("Network:"))
            network = ln.mid(strlen("Network:")).trimmed();
        else if (ln.startsWith("Message:"))
            message = ln.mid(strlen("Message:")).trimmed();
        else if (ln.startsWith("Viewing Key:"))
            viewingKey = ln.mid(strlen("Viewing Key:")).trimmed();
        else if (ln.startsWith("Tor Address:"))
            torAddress = ln.mid(strlen("Tor Address:")).trimmed();
        else if (ln.startsWith("MWCMQS Address:"))
            mqsAddress = ln.mid(strlen("MWCMQS Address:")).trimmed();
    }

    wallet713->setValidateOwnershipProof(network, message, viewingKey, torAddress, mqsAddress, errors );
    return true;
}

}
