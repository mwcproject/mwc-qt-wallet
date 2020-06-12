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

#ifndef MWC_QT_WALLET_TASKLISTENING_H
#define MWC_QT_WALLET_TASKLISTENING_H

#include "../mwc713task.h"

namespace wallet {

// It is listener task. No input can be defined.
// Listening for MWC MQ & keybase connection statuses
class TaskListeningListener : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 3600*1000*5; // NA in any case

    TaskListeningListener( MWC713 *wallet713 ) :
            Mwc713Task("TaskListeningListener", "", wallet713,"") {}

    virtual ~TaskListeningListener() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>();}
};


class TaskListeningStart : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 7000;

    // Start one listen per request. mwc713 doesn't support both
    TaskListeningStart(MWC713 *wallet713, bool startMq, bool startKeybase, bool startTor, bool _initialStart ) :
            Mwc713Task("TaskListeningStart", calcCommand(startMq, startKeybase, startTor), wallet713,""),
            initialStart(_initialStart)
    {}

    virtual ~TaskListeningStart() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString calcCommand(bool startMq, bool startKeybase, bool startTor) const;

    bool initialStart;
};



class TaskListeningStop : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 8000;

    // Start one listen per request. mwc713 doesn't support both
    TaskListeningStop(MWC713 *wallet713, bool stopMq, bool stopKeybase, bool stopTor) :
            Mwc713Task("TaskListeningStop", calcCommand(stopMq, stopKeybase, stopTor), wallet713,"") {}

    virtual ~TaskListeningStop() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString calcCommand(bool stopMq, bool stopKeybase, bool stopTor) const;
};


}

#endif //MWC_QT_WALLET_TASKLISTENING_H
