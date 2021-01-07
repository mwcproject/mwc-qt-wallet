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

#ifndef MWC_QT_WALLET_TASKSEND_H
#define MWC_QT_WALLET_TASKSEND_H

#include "../mwc713task.h"
#include "../../util/stringutils.h"

namespace wallet {

// Listening for transaction task
class TaskSlatesListener : public Mwc713Task {
public:
    TaskSlatesListener( MWC713 * wallet713) :
            Mwc713Task("TaskSlatesListener","", "", wallet713, "") {}

    virtual ~TaskSlatesListener() override {}

    virtual bool processTask(const QVector<WEvent> & events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>();}
};

////////////////////////////////////     set-recv       ///////////////

class TaskSetReceiveAccount : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*2;

    TaskSetReceiveAccount( MWC713 *wallet713, QString account ) :
            Mwc713Task("TaskSetReceiveAccount","",
                       QString("set-recv ") + util::toMwc713input(account),
                       wallet713, "set-recv for " + account) {}

    virtual ~TaskSetReceiveAccount() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
};


//////////////////////////////////////////////////////////////////

class TaskSendMwc : public QObject, public Mwc713Task {
    Q_OBJECT
public:
    const static int64_t TIMEOUT = 1000*150; // Send timeout is 2 minutes, we are making it a little longer to cover start/stop overhead

    // coinNano == -1  - mean All
    TaskSendMwc( MWC713 *wallet713, int64_t coinNano, const QString & address, const QString & apiSecret, QString message,
                 int inputConfirmationNumber, int changeOutputs, const QStringList & outputs, bool fluff, int ttl_blocks,
                 bool generateProof, const QString & expectedproofAddress ) :
            Mwc713Task("TaskSendMwc", "Sending coins online...",
                    buildCommand( coinNano, address, apiSecret, message, inputConfirmationNumber, changeOutputs, outputs, fluff, ttl_blocks, generateProof, expectedproofAddress),
                    wallet713, ""), sendMwcNano(coinNano) {}

    virtual ~TaskSendMwc() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    // coinNano == -1  - mean All
    QString buildCommand(int64_t coinNano, const QString & address, const QString & apiSecret, QString message, int inputConfirmationNumber, int changeOutputs, const QStringList & outputs, bool fluff, int ttl_blocks, bool generateProof, const QString & expectedproofAddress) const;

    int64_t sendMwcNano;
};


class TaskSendFile : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*20;

    // Most are optional (empty values - ignore)
    TaskSendFile( MWC713 *wallet713, int64_t coinNano, QString message, QString fileTx, int inputConfirmationNumber, int changeOutputs, const QStringList & outputs, int ttl_blocks, bool generateProof ) :
            Mwc713Task("TaskSendFile", "Sending coins offline...",
                       buildCommand(coinNano, message, fileTx, inputConfirmationNumber, changeOutputs, outputs, ttl_blocks, generateProof),
                       wallet713, "") {}

    virtual ~TaskSendFile() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString buildCommand( int64_t coinNano, QString message, QString fileTx, int inputConfirmationNumber, int changeOutputs, const QStringList & outputs, int ttl_blocks, bool generateProof ) const;
};

class TaskReceiveFile : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*20;

    TaskReceiveFile( MWC713 *wallet713, QString fileName, QString identifier = "" ) :
            Mwc713Task("TaskReceiveFile", "Recieving file transaction...", buildCommand(fileName, identifier), wallet713,""), inFileName(fileName) {}

    virtual ~TaskReceiveFile() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString buildCommand(QString fileName, QString identifier) const;
    QString inFileName;
};

class TaskFinalizeFile : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*20;

    TaskFinalizeFile( MWC713 *wallet713, QString fileTxResponse, bool fluff ) :
            Mwc713Task("TaskReceiveFile", "Finalizing file transaction...", buildCommand(fileTxResponse, fluff), wallet713, "") {}

    virtual ~TaskFinalizeFile() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString buildCommand(QString filename, bool fluff) const;
};

class TaskSendSlatepack : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*20;

    // Most are optional (empty values - ignore)
    TaskSendSlatepack( MWC713 *wallet713, int64_t coinNano, QString message, int inputConfirmationNumber, int changeOutputs,
                  const QStringList & outputs, int ttl_blocks, bool generateProof, QString slatepackRecipientAddress, bool isLockLater, QString _tag ) :
            Mwc713Task("TaskSendSlatepack", "Sending slatepack transaction...",
                       buildCommand(coinNano, message, inputConfirmationNumber, changeOutputs, outputs, ttl_blocks, generateProof, slatepackRecipientAddress, isLockLater),
                       wallet713, ""),
                       tag(_tag) {}

    virtual ~TaskSendSlatepack() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString buildCommand( int64_t coinNano, QString message, int inputConfirmationNumber, int changeOutputs,
                          const QStringList & outputs, int ttl_blocks, bool generateProof, QString slatepackRecipientAddress,
                          bool isLockLater ) const;
    QString tag;
};

class TaskReceiveSlatepack : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*20;

    TaskReceiveSlatepack( MWC713 *wallet713, QString slatepack, QString _tag ) :
            Mwc713Task("TaskReceiveFile", "Receiving slatepack transaction...", "receive --content \"" + slatepack + "\"", wallet713,""),
            tag(_tag) {}

    virtual ~TaskReceiveSlatepack() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString tag;
};

class TaskFinalizeSlatepack : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*20;

    TaskFinalizeSlatepack( MWC713 *wallet713, QString slatepack, bool fluff, QString _tag ) :
            Mwc713Task("TaskReceiveFile", "Finalizing slatepack transaction...", buildCommand(slatepack, fluff), wallet713, ""),
            tag(_tag) {}

    virtual ~TaskFinalizeSlatepack() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString buildCommand(QString slatepack, bool fluff) const;
    QString tag;
};


class TaskRequestRecieverWalletAddress : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*40; // up to 30 seconds in case of network issues. We have retry logic there and it takes time

    TaskRequestRecieverWalletAddress( MWC713 *wallet713, QString _url, QString apiSecret ) :
        Mwc713Task("TaskRequestRecieverWalletAddress", "Validating proof...", "check-proof --to " + _url +
                (apiSecret.isEmpty()? "" : " --apisecret \"" + apiSecret + "\"" ), wallet713, ""),
            url(_url) {}

    virtual ~TaskRequestRecieverWalletAddress() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString url;
};

class TaskDecodeSlatepack : public Mwc713Task {
public:
    const static int64_t TIMEOUT = 1000*5; // Should be very quick operation

    TaskDecodeSlatepack( MWC713 *wallet713, QString _slatepack, QString _tag ) :
            Mwc713Task("TaskDecodeSlatepack", "Decoding slatepack...", "decode_slatepack --slatepack \"" + _slatepack + "\"", wallet713, ""),
            slatepack(_slatepack),
            tag(_tag) {}

    virtual ~TaskDecodeSlatepack() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString slatepack;
    QString tag;
};


}

#endif //MWC_QT_WALLET_TASKSEND_H
