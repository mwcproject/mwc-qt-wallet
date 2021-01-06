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

#ifndef GUI_WALLET_TASKADDRESS_H
#define GUI_WALLET_TASKADDRESS_H


#include "../mwc713task.h"

namespace wallet {

// Geting and chnanging
class TaskMwcMqAddress : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 3000;

    // genNext:true - generate next address, address by index
    // idx     - index for generated address (genNext is true)
    // genNext:false - get current address with associated index
    TaskMwcMqAddress( MWC713 * wallet713, bool genNext, int idx ) :
            Mwc713Task("TaskMwcMqAddress", "Requesting MQS address...", calcCommandLine(genNext, idx), wallet713,"") {}

    virtual ~TaskMwcMqAddress() override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}

private:
    QString calcCommandLine( bool genNext, int idx ) const;
};

class TaskFileProofAddress : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 2000;

    // genNext:true - generate next address, address by index
    // idx     - index for generated address (genNext is true)
    // genNext:false - get current address with associated index
    TaskFileProofAddress( MWC713 * wallet713) :
            Mwc713Task("FileProofAddress", "Requesting Wallet Public address...", "address --provable-address" , wallet713,"") {}

    virtual ~TaskFileProofAddress() override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
};



}


#endif //GUI_WALLET_TASKADDRESS_H
