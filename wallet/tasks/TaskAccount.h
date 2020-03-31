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

#ifndef MWC_QT_WALLET_TASKACCOUNT_H
#define MWC_QT_WALLET_TASKACCOUNT_H

#include "../mwc713task.h"
#include "../../util/stringutils.h"
#include "../../core/Config.h"

namespace wallet {

// List all account
class TaskAccountList : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*7;

    TaskAccountList( MWC713 * _wallet713 ) :
            Mwc713Task("TaskAccountList", "accounts", _wallet713,"") {}

    virtual ~TaskAccountList() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}

};

// Create acoount command. Note that after account creation wallet keeping the original default account
class TaskAccountCreate : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*7;

    // Expected that account is unique. Account must meet mwc713 expectations
    TaskAccountCreate( MWC713 * _wallet713, QString account ) :
            Mwc713Task("TaskAccountCreate", "account create " + util::toMwc713input(account), _wallet713,""), newAccountName(account) { Q_ASSERT(account.length()>0); }

    virtual ~TaskAccountCreate() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString newAccountName;
};

class TaskAccountSwitch : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*7;

    TaskAccountSwitch( MWC713 * _wallet713, QString accountName, QString password, bool _makeAccountCurrent ) :
            Mwc713Task("TaskAccountSwitch", "account switch " +  util::toMwc713input(accountName) +
                       (password.isEmpty() ? "" : " -p " +  util::toMwc713input(password)), _wallet713, "account switch " + accountName ),
            switchAccountName(accountName), makeAccountCurrent(_makeAccountCurrent)
    {
        Q_ASSERT(accountName.length()>0);
    }

    virtual ~TaskAccountSwitch() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString switchAccountName;
    bool makeAccountCurrent;
};


class TaskAccountRename : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*5;

    // createSimulation is true if user expect create account instead of rename deleted
    TaskAccountRename( MWC713 * _wallet713, QString oldAccountName, QString newAccountName, bool createSimulation ) :
            Mwc713Task("TaskAccountRename", "account rename " +  util::toMwc713input(oldAccountName) + " " + util::toMwc713input(newAccountName), _wallet713, ""),
            oldName(oldAccountName),
            newName(newAccountName),
            createAccountSimulation(createSimulation) { }

    virtual ~TaskAccountRename() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString oldName;
    QString newName;
    bool createAccountSimulation;
};


class TaskAccountInfo : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*10;

    // noRefresh can be used for sequenced calls
    TaskAccountInfo( MWC713 * _wallet713, int confimations ) :
                Mwc713Task("TaskAccountInfo", QString("info -c ") + QString::number(confimations) + " --no-refresh", _wallet713,"") {}

    virtual ~TaskAccountInfo() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
};

// Just a callback, not a real task
class TaskAccountProgress : public Mwc713Task {
public:
    TaskAccountProgress( MWC713 * wallet713, int _pos, int _total ) :
    Mwc713Task("TaskAccountProgress", "", wallet713,""), pos(_pos), total(_total) {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>();}
private:
    int pos;
    int total;
};

// Just a callback, not a real task
class TaskAccountListFinal : public Mwc713Task {
public:
    TaskAccountListFinal( MWC713 * _wallet713, QString _accountName2switch ) :
            Mwc713Task("TaskAccountListFinal", "", _wallet713,""), accountName2switch(_accountName2switch) {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>();}
private:
    QString accountName2switch;
};

}

#endif //MWC_QT_WALLET_TASKACCOUNT_H
