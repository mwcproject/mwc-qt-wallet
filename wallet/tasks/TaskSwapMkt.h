// Copyright 2021 The MWC Developers
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

#ifndef MWC_QT_WALLET_TASKSWAPMKT_H
#define MWC_QT_WALLET_TASKSWAPMKT_H

#include "../mwc713task.h"
#include "../../util/stringutils.h"

namespace wallet {

// It is listener task. No input can be defined.
// Listening for a  accept_offer and fail_bidding messages
class TaskSwapMktNewMessage : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 3600*1000*5; // NA in any case

    TaskSwapMktNewMessage( MWC713 *wallet713 ) :
            Mwc713Task("TaskSwapMktNewMessage", "", "", wallet713,"") {}

    virtual ~TaskSwapMktNewMessage() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>();}
};


class TaskCreateIntegrityFee : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*120; // 120 seconds can be possible because of the status update and connection errors

    TaskCreateIntegrityFee( MWC713 *wallet713, const QString & account, double mwcReserve, const QVector<double> & fees ) :
        Mwc713Task("TaskCreateIntegrityFee", "Paying integrity fees...",
                   generateCommandLine( account, mwcReserve, fees),
                    wallet713, "")
        {}

    virtual ~TaskCreateIntegrityFee() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString generateCommandLine(const QString & account, double mwcReserve, const QVector<double> & fees) const;
};


class TaskRequestIntegrityFee : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*120; // 120 seconds can be possible because of the status update and connection errors

    TaskRequestIntegrityFee( MWC713 * wallet713) :
            Mwc713Task("TaskCreateIntegrityFee", "Requesting integrity fees...",
                       "integrity --check --json",
                       wallet713, "")
    {}

    virtual ~TaskRequestIntegrityFee() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
};


class TaskWithdrawIntegrityFees : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*120; // 120 seconds can be possible because of the status update and connection errors

    TaskWithdrawIntegrityFees( MWC713 * wallet713, const QString & account) :
            Mwc713Task("TaskWithdrawIntegrityFees", "Withdraw integrity fees...",
                       "integrity --withdraw --account " + util::toMwc713input(account),
                       wallet713, "")
    {}

    virtual ~TaskWithdrawIntegrityFees() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
};

class TaskRequestMessagingStatus : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*3; // Should be very fast operation

    TaskRequestMessagingStatus( MWC713 * wallet713) :
            Mwc713Task("TaskRequestMessagingStatus", "",
                       "messaging --status --json",
                       wallet713, "")
    {}

    virtual ~TaskRequestMessagingStatus() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
};


class TaskMessagingPublish : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*180; // Might need an update

    TaskMessagingPublish( MWC713 * wallet713, QString messageJsonStr, QString feeTxUuid, QString _id, int publishInterval, QString topic) :
            Mwc713Task("TaskMessagingPublish", "",
                       "messaging -i " + QString::number(publishInterval) + " -p " + util::toMwc713input(messageJsonStr) + " -u " + feeTxUuid + " -t " + topic,
                       wallet713, ""), id(_id)
    {}

    virtual ~TaskMessagingPublish() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString id;
};

class TaskCheckIntegrity : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*120; // might need sync

    TaskCheckIntegrity( MWC713 * wallet713) :
            Mwc713Task("TaskCheckIntegrity", "",
                       "messaging --check_integrity --json",
                       wallet713, "")
    {}

    virtual ~TaskCheckIntegrity() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
};


class TaskMessageWithdraw : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*5; // should be fast

    TaskMessageWithdraw( MWC713 * wallet713, QString msgUuid) :
            Mwc713Task("TaskMessageWithdraw", "",
                       "messaging --withdraw_message " + util::toMwc713input(msgUuid) + " --json",
                       wallet713, "")
    {}

    virtual ~TaskMessageWithdraw() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
};


class TaskRequestReceiveMessages : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*10; // should be fast

    TaskRequestReceiveMessages( MWC713 * wallet713, bool cleanBuffer) :
        Mwc713Task("TaskRequestReceiveMessages", "",
            QString("messaging --receive_messages ") + (cleanBuffer ? "yes" : "no") + " --json",
            wallet713, "")
    {}

    virtual ~TaskRequestReceiveMessages() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
};

class TaskStartListenOnTopic : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*5;

    TaskStartListenOnTopic( MWC713 * wallet713, const QString & topic) :
            Mwc713Task("TaskStartListenOnTopic", "",
                       "messaging --add_topic " + topic,
                       wallet713, "")
    {}

    virtual ~TaskStartListenOnTopic() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
};

class TaskStopListenOnTopic : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*5;

    TaskStopListenOnTopic( MWC713 * wallet713, const QString & topic) :
            Mwc713Task("TaskStopListenOnTopic", "",
                       "messaging --remove_topic " + topic,
                       wallet713, "")
    {}

    virtual ~TaskStopListenOnTopic() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
};


class TasksSendMarketplaceMessage : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*60*3; // might take time because of connection timeouts

    TasksSendMarketplaceMessage( MWC713 * wallet713, QString command, QString _wallet_tor_address, QString _offer_id, QString _cookie) :
            Mwc713Task("TasksSendMarketplaceMessage", "",
                       "send_marketplace_message --command " + command + " --offer_id " + _offer_id + " --tor_address " + _wallet_tor_address,
                       wallet713, ""),
            wallet_tor_address(_wallet_tor_address),
            offer_id(_offer_id),
            cookie(_cookie)
    {}

    virtual ~TasksSendMarketplaceMessage() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString wallet_tor_address;
    QString offer_id;
    QString cookie;
};


}

#endif //MWC_QT_WALLET_TASKSWAPMKT_H
