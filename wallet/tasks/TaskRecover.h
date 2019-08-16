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

#ifndef MWC_QT_WALLET_TASKRECOVER_H
#define MWC_QT_WALLET_TASKRECOVER_H

#include "../mwc713task.h"
#include <QThread>
#include "../../util/stringutils.h"

namespace wallet {

// It is listener task. No input can be defined
class TaskRecoverProgressListener : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 3600*1000*5; // 5 hours should be enough

    // Start one listen per request. mwc713 doesn't support both
    TaskRecoverProgressListener( MWC713 *wallet713 ) :
            Mwc713Task("TaskRecoverProgressListener", "", wallet713,"") {}

    virtual ~TaskRecoverProgressListener() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>();}

};

/////////////////////////////////////////////////////

class TaskRecoverFull : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000L*3600L*5L; // 5 hours should be enough

    // Expected that listening is already stopped
    TaskRecoverFull( MWC713 *wallet713) :
            Mwc713Task("TaskRecover", "", wallet713, "recover from mnemonic") {}

    virtual ~TaskRecoverFull() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}

};


///////////////////////////////////////////////////////////////
////////////////////////

// Just show mnenonic
class TaskRecoverShowMnenonic : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 5000; //

    // Expected that listening is already stopped
    TaskRecoverShowMnenonic( MWC713 *wallet713, QString password ) :
            Mwc713Task("TaskRecover", "recover -d -p " + util::toMwc713input(password), wallet713, "get mnemonic seed") {}

    virtual ~TaskRecoverShowMnenonic() override {}

    virtual void onStarted() override;

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}

};

////////////////////////////////////////////////////////////////
///////////////////

// 'check' command. Will recover from
class TaskCheck : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 3600*1000*5; // 5 hours should be enough

    // Expected that listening is already stopped
    TaskCheck( MWC713 *wallet713, bool wait4listeners ) :
            Mwc713Task("TaskRecover", "check", wallet713, ""), sleepBeforeStart(wait4listeners) {}

    virtual ~TaskCheck() override {}

    // Wait 3 seconds fro listeners to stop. Expected that stopping in the progress
    virtual void onStarted() override { if (sleepBeforeStart) { QThread::msleep(3000); } }

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}

private:
    bool sleepBeforeStart;
};




}



#endif //MWC_QT_WALLET_TASKRECOVER_H
