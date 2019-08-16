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

#ifndef GUI_WALLET_TASKSTARTING_H
#define GUI_WALLET_TASKSTARTING_H

#include "../mwc713task.h"

namespace wallet {

    class TaskStarting : public Mwc713Task {
    public:
        const static int64_t TIMEOUT = 8000;

        TaskStarting( MWC713 * wallet713 ) : Mwc713Task("Starting", "", wallet713,"") {}
        virtual ~TaskStarting() override {}

        virtual bool processTask(const QVector<WEvent> & events) override;

        virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_INIT, WALLET_EVENTS::S_READY };}

    private:
    };

}

#endif //GUI_WALLET_TASKSTARTING_H
