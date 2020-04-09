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

#include "TaskRecover.h"
#include <QDebug>
#include "../mwc713.h"
#include "../../util/Log.h"
#include <QThread>
#include "TaskWallet.h"

namespace wallet {


static bool ProcessRecoverTask(const QVector<WEvent> &events,MWC713 *wallet713) {
    qDebug() << "ProcessRecoverTask with events: " << printEvents(events);

    QVector< WEvent > staring = filterEvents(events, WALLET_EVENTS::S_RECOVERY_STARTING );
    QVector< WEvent > done = filterEvents(events, WALLET_EVENTS::S_RECOVERY_DONE );
    QVector< WEvent > mqAddr = filterEvents(events, WALLET_EVENTS::S_YOUR_MWC_ADDRESS );
    QVector< WEvent > error = filterEvents(events, WALLET_EVENTS::S_ERROR );

    QStringList errorMessages;
    for (auto & evt : error) {
        if (!evt.message.isEmpty())
            errorMessages.append(evt.message);
    }

    wallet713->setRecoveryResults( staring.size() > 0, done.size()>0,
                                   mqAddr.size()<1 ? "" : mqAddr[0].message,
                                   errorMessages );
    return true;
}


// ----------------------------------- TaskRecoverProgressListener ---------------------------------

bool TaskRecoverProgressListener::processTask(const QVector<WEvent> &events) {
    // It is listener, one by one processing only
    Q_ASSERT(events.size()==1);

    const WEvent & evt = events[0];
    if (evt.event != S_RECOVERY_PROGRESS)
        return false;

    qDebug() << "TaskRecoverProgressListener::processTask with events: " << printEvents(events);

    QStringList lst = evt.message.split('|');
    Q_ASSERT(lst.size()==2);
    if (lst.size()!=2)
        return false;

    bool ok0 = false;
    int64_t limit = lst[0].toLongLong(&ok0); // Limit
    bool ok1 = false;
    int64_t pos = lst[1].toLongLong(&ok1); // position

    Q_ASSERT(ok0 && ok1);
    if (ok0 && ok1) {
        Q_ASSERT(pos<=limit);
        wallet713->setRecoveryProgress( pos, limit );
        return true;
    }

    return false;
}

// --------------------------- TaskRecoverFull ---------------------------------

void TaskRecoverFull::onStarted() {
    logger::blockLogMwc713out( true );
}


bool TaskRecoverFull::processTask(const QVector<WEvent> &events) {
    logger::blockLogMwc713out( false );
    return ProcessRecoverTask(events, wallet713);
}

// -------------------- TaskRecover3Password ----------------------

void TaskRecoverShowMnenonic::onStarted() {
    logger::blockLogMwc713out( true );

    // Let's sleep some time. Needed to get a chance to stop listeners
    QThread::msleep(5000); // 5 seconds normally is enough to stop all listeners
}

bool TaskRecoverShowMnenonic::processTask(const QVector<WEvent> &events) {
    logger::blockLogMwc713out( false );


    QVector<QString> seed = calcSeedFromEvents(events);

    if (seed.size()>0) {
        wallet713->setGettedSeed(seed);
        return true;
    }

    // Check for error
    QVector< WEvent > err = filterEvents(events, WALLET_EVENTS::S_ERROR );
    if (err.size()>0) {
        wallet713->setGettedSeed( QVector<QString>{err[0].message} );
    }
    else {
        wallet713->setGettedSeed( QVector<QString>() );
    }

    return true;
}

// ----------------------- TaskCheck --------------------------

bool TaskCheck::processTask(const QVector<WEvent> &events) {

    QVector< WEvent > lns = filterEvents(events, WALLET_EVENTS::S_LINE );
    bool ok = false;
    for ( auto & l : lns ) {
        if (l.message.contains("check and repair done!"))
            ok = true;
    }

    if (ok) {
        wallet713->setCheckResult(true, "");
        return true;
    }

    QStringList messages;
    QVector< WEvent > errs = filterEvents(events, WALLET_EVENTS::S_ERROR );
    for (auto & e : errs)
        messages.push_back(e.message);

    wallet713->setCheckResult(false, util::formatErrorMessages(messages));

    return true;
}


}

