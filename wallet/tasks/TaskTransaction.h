#ifndef MWC_QT_WALLET_TASKTRANSACTION_H
#define MWC_QT_WALLET_TASKTRANSACTION_H

#include "../mwc713task.h"
#include "../../util/stringutils.h"

namespace wallet {

class TaskOutputCount : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*10;

    TaskOutputCount( MWC713 * wallet713, QString _account ) :
            Mwc713Task("Outputs", "output_count", wallet713, ""), account(_account) {}

    virtual ~TaskOutputCount() override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return { WALLET_EVENTS::S_READY };}
private:
    QString account;
};


class TaskOutputs : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*15;

    TaskOutputs( MWC713 * wallet713, int offset, int number ) :
            Mwc713Task("Outputs", "outputs -o " + QString::number(offset) + " -l " + QString::number(number), wallet713, "")
            { Q_ASSERT(offset>=0); Q_ASSERT(number>0);}

    virtual ~TaskOutputs() override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return { WALLET_EVENTS::S_READY };}
};

class TaskTransactionCount : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*10;

    TaskTransactionCount( MWC713 * wallet713, QString _account ) :
            Mwc713Task("Outputs", "txs_count", wallet713, ""), account(_account) {}

    virtual ~TaskTransactionCount() override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return { WALLET_EVENTS::S_READY };}
private:
    QString account;
};

class TaskTransactions : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*60;

    TaskTransactions( MWC713 * wallet713, int offset, int number) :
            Mwc713Task("Transactions", "txs -o " + QString::number(offset) + " -l " + QString::number(number), wallet713, "")
            { Q_ASSERT(offset>=0); Q_ASSERT(number>0);}

    virtual ~TaskTransactions() override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return { WALLET_EVENTS::S_READY };}
};



class TaskTransCancel : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*7;

    TaskTransCancel( MWC713 * wallet713, int64_t transId ) :
            Mwc713Task("TaskTransCancel", "cancel --id " + QString::number(transId) , wallet713, ""), transactionId(transId) {}

    virtual ~TaskTransCancel() override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return { WALLET_EVENTS::S_READY };}
private:
    int64_t transactionId;
    QString proofFileName;
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
