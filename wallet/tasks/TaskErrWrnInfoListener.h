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

#ifndef GUI_WALLET_TASKERRWRNINFOLISTENER_H
#define GUI_WALLET_TASKERRWRNINFOLISTENER_H

#include "../mwc713task.h"

namespace wallet {

// istener: Listening for all Errors, Warnings and Infos
class TaskErrWrnInfoListener : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 3600*1000*5; // NA in any case

    // Start one listen per request. mwc713 doesn't support both
    TaskErrWrnInfoListener( MWC713 *wallet713 ) :
            Mwc713Task("TaskErrWrnInfoListener", "", wallet713,"") {}

    virtual ~TaskErrWrnInfoListener() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>();}
private:
    bool walletIsReady = false;
};

}


#endif //GUI_WALLET_TASKERRWRNINFOLISTENER_H
