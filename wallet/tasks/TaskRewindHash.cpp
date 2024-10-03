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

#include "TaskRewindHash.h"
#include "TaskWallet.h"
#include "../mwc713.h"

namespace wallet {

bool TasksViewRewindHash::processTask(const QVector<WEvent> &events) {
    QString errors;
    QString rewindHash;

    for ( const auto & evt : events ) {
        if (evt.event != WALLET_EVENTS::S_LINE)
            continue;


        QString ln = evt.message;
        if (ln.contains("Error", Qt::CaseInsensitive)) {
            if (errors.isEmpty())
                errors += "\n";
            errors += ln;
        }

        if (ln.startsWith("Wallet Rewind Hash:")) {
            rewindHash = ln.mid(strlen("Wallet Rewind Hash:")).trimmed();
        }
    }

    wallet713->setViewRewindHash( rewindHash, errors );
    return true;
}

// It is listener task. No input can be defined.
// Listening for MWC MQ & tor connection statuses
/////////////////////////////////////////////////////////////////////
/// TasksScanRewindHash

void TasksScanRewindHash::onStarted() {
}

bool TasksScanRewindHash::processTask(const QVector<WEvent> & events) {
    QString errors;
    QVector< WalletOutput > outputResult;
    int64_t total = -1;

    for ( const auto & evt : events ) {
        if (evt.event != WALLET_EVENTS::S_LINE)
            continue;

        QString ln = evt.message;
        if (ln.contains("Error", Qt::CaseInsensitive)) {
            if (errors.isEmpty())
                errors += "\n";
            errors += ln;
        }

        if (ln.startsWith("ViewOutput:")) {
            QString outputInfo = ln.mid(strlen("ViewOutput:")).trimmed();
            QStringList data = outputInfo.split(',');
            if (data.length()==7) {
                outputResult.push_back( WalletOutput::create(data[0],
                                                           data[1], data[2], data[3],
                                                           "", // status
                                                           data[4]=="true",
                                                           data[5],
                                                           data[6].toLongLong(),
                                                           -1));
            }
        }

        if (ln.startsWith("ViewTotal:")) {
            QString totalStr = ln.mid(strlen("ViewTotal:")).trimmed();
            total = totalStr.toLongLong();
        }
    }

    wallet713->setScanRewindHash( outputResult, total, errors );
    return true;
}

}
