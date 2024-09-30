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

#ifndef MWC_QT_WALLET_TASKSOWNERSHIP_H
#define MWC_QT_WALLET_TASKSOWNERSHIP_H

#include "../mwc713task.h"
#include "../../util/stringutils.h"

namespace wallet {

// It is listener task. No input can be defined.
// Listening for MWC MQ & tor connection statuses
class TasksGenerateOwnership : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 3000;

    TasksGenerateOwnership( MWC713 *wallet713, const QString & message, bool includePublicRootKey, bool includeTorAddress, bool includeMqsAddress ) :
            Mwc713Task("TasksGenerateOwnership", "", "generate_ownership_proof --message " + util::toMwc713input(message) +
                               (includePublicRootKey ? " -p" : "") + (includeTorAddress ? " -t" : "") + (includeMqsAddress ? " -m" : ""),
                       wallet713,"") {}

    virtual ~TasksGenerateOwnership() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
};

// It is listener task. No input can be defined.
// Listening for MWC MQ & tor connection statuses
class TasksValidateOwnership : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 3000; // long task, put 5 hours to be sure

    TasksValidateOwnership( MWC713 * wallet713, const QString & proof) :
            Mwc713Task("TasksScanRewindHash","Scanning by viewing key...", "validate_ownership_proof --proof " + util::toMwc713input(proof), wallet713, "") {}

    virtual ~TasksValidateOwnership() override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return { WALLET_EVENTS::S_READY };}
};


}


#endif //MWC_QT_WALLET_TASKSOWNERSHIP_H
