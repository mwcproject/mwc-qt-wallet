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

#ifndef MWC_QT_WALLET_TASKTRANSACTION_H
#define MWC_QT_WALLET_TASKTRANSACTION_H

#include "../mwc713task.h"
#include "../../util/stringutils.h"

namespace wallet {


class TaskOutputs : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*15;

    // Outputs run with no-refresh because wallet responsible to call sync first
    TaskOutputs( MWC713 * wallet713, bool show_spent ) :
        Mwc713Task("Outputs", QString("outputs") + (show_spent?" --show-spent":"") + " --no-refresh" , wallet713, "") {showSpent=show_spent;}

    virtual ~TaskOutputs() override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return { WALLET_EVENTS::S_READY };}
private:
    bool showSpent;
};

// Get outputs and deliver them directly to HODL status
class TaskOutputsForAccount : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*15;

    TaskOutputsForAccount( MWC713 * wallet713, QString _accountName) :
            Mwc713Task("Outputs", "outputs --no-refresh", wallet713, ""),
            accountName(_accountName) {}

    virtual ~TaskOutputsForAccount() override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return { WALLET_EVENTS::S_READY };}
private:
    QString accountName;
};

class TaskTransactions : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*60;

    // Transactions run with no-refresh because wallet responsible to call sync first
    TaskTransactions( MWC713 * wallet713) :
            Mwc713Task("Transactions", "txs --show-full --no-refresh", wallet713, "") {}

    virtual ~TaskTransactions() override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return { WALLET_EVENTS::S_READY };}
};

class TaskTransactionsById : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*60;

    // Transactions run with no-refresh because wallet responsible to call sync first
    TaskTransactionsById( MWC713 * wallet713, int txId) :
            Mwc713Task("Transactions", "txs --id " + QString::number(txId) +" --no-refresh", wallet713, "")  {}

    virtual ~TaskTransactionsById() override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return { WALLET_EVENTS::S_READY };}
};

// Just a callback, not a real task
class TaskAllTransactionsStart : public Mwc713Task {
public:
    TaskAllTransactionsStart( MWC713 * wallet713 ) :
            Mwc713Task("TaskAllTransactionsStart", "", wallet713,"") {}

    virtual bool processTask(const QVector<WEvent> &events) override;
    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>();}
};

class TaskAllTransactionsEnd : public Mwc713Task {
public:
    TaskAllTransactionsEnd( MWC713 * wallet713 ) :
            Mwc713Task("TaskAllTransactionsEnd", "", wallet713,"") {}

    virtual bool processTask(const QVector<WEvent> &events) override;
    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>();}
};

class TaskAllTransactions : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*180;

    TaskAllTransactions( MWC713 * wallet713) :
            Mwc713Task("Transactions", "txs", wallet713, "") {}

    virtual ~TaskAllTransactions() override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return { WALLET_EVENTS::S_READY };}
};



class TaskTransCancel : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*7;

    TaskTransCancel( MWC713 * wallet713, int64_t transId, QString _account ) :
            Mwc713Task("TaskTransCancel", "cancel --id " + QString::number(transId) , wallet713, ""), transactionId(transId), account(_account) {}

    virtual ~TaskTransCancel() override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return { WALLET_EVENTS::S_READY };}
private:
    int64_t transactionId;
    QString account;
};



class TaskTransExportProof : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*7;

    TaskTransExportProof( MWC713 * wallet713, QString fileName, int64_t transId ) :
            Mwc713Task("TransExportProof", "export-proof --file " + util::toMwc713input(fileName) + " --id " + QString::number(transId) , wallet713, ""), transactionId(transId), proofFileName(fileName) {}

    virtual ~TaskTransExportProof () override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return { WALLET_EVENTS::S_READY };}
private:
    int64_t transactionId;
    QString proofFileName;
};

class TaskTransVerifyProof : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*7;

    TaskTransVerifyProof( MWC713 * wallet713, QString fileName  ) :
            Mwc713Task("TaskTransVerifyProof", "verify-proof --file " + util::toMwc713input(fileName), wallet713, ""), proofFileName(fileName) {}

    virtual ~TaskTransVerifyProof () override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return { WALLET_EVENTS::S_READY };}
private:
    QString proofFileName;
};


}

#endif //MWC_QT_WALLET_TASKTRANSACTION_H
