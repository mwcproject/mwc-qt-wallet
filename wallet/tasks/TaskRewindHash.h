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

//
// Created by Konstantin Bay on 9/22/24.
//

#ifndef MWC_QT_WALLET_TASKREWINDHASH_H
#define MWC_QT_WALLET_TASKREWINDHASH_H

#include "../mwc713task.h"

namespace wallet {

// It is listener task. No input can be defined.
// Listening for MWC MQ & tor connection statuses
class TasksViewRewindHash : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 5000; // NA in any case

    TasksViewRewindHash( MWC713 *wallet713 ) :
            Mwc713Task("TaskRewindHash", "", "rewind_hash", wallet713,"") {}

    virtual ~TasksViewRewindHash() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
};

// It is listener task. No input can be defined.
// Listening for MWC MQ & tor connection statuses
class TasksScanRewindHash : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 3600*1000*5; // long task, put 5 hours to be sure

    TasksScanRewindHash( MWC713 * wallet713, const QString & rewindHash) :
            Mwc713Task("TasksScanRewindHash","Scanning by viewing key...", "scan_rewind_hash --rewind_hash " + rewindHash, wallet713, "") {}

    virtual ~TasksScanRewindHash() override {}

    virtual void onStarted() override;

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return { WALLET_EVENTS::S_READY };}
};


}

#endif //MWC_QT_WALLET_TASKREWINDHASH_H
