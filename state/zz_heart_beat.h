// Copyright 2025 The MWC Developers
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

#ifndef MWC_QT_WALLET_ZZ_HEART_BEAT_H
#define MWC_QT_WALLET_ZZ_HEART_BEAT_H

#include "state.h"
#include "../wallet/wallet.h"

namespace state {

struct ReceivedTxInfo {
    int64_t mwc;
    QString fromAddr;
    QString message;

    ReceivedTxInfo() = default;
    ReceivedTxInfo(const ReceivedTxInfo &) = default;
    ReceivedTxInfo(int64_t _mwc, const QString &_fromAddr, const QString &_message)
        : mwc(_mwc),
          fromAddr(_fromAddr),
          message(_message) {
    }

    ReceivedTxInfo& operator=(const ReceivedTxInfo&) = default;
};


// Special start that only Job is collect some status data and show messages/warnings about the events
// Note, this state doesn't have any associated windows.
class HeartBeat : public QObject, public State {
    Q_OBJECT
public:
    HeartBeat(StateContext * context);
    virtual ~HeartBeat() override;

    void updateNodeStatus();

protected:
    virtual NextStateRespond execute() override;
    virtual bool mobileBack() override {return false;}
    virtual QString getHelpDocName() override {return "";}

private slots:
    void onLogin();
    void onLogout();

    void onSlateReceivedFrom(QString slate, int64_t mwc, QString fromAddr, QString message );
    void onScanProgress( QString responseId, QJsonObject statusMessage );
    void onScanDone( QString responseId, bool fullScan, int height, QString errorMessage );
private:
    virtual void timerEvent(QTimerEvent *event) override;

    void emitTorMqsStateChange();
private:
    bool lastNodeIsHealty = true;
    bool lastUsePubNode = true;
    bool justLogin = true;
    int64_t lastNodeDifficulty = 0;
    int64_t lastNodeHeight = 0;
    bool mqsOnline = false;
    bool torOnline = false;

    // Transactions that we are processing for congrats request
    QMap<QString, ReceivedTxInfo> recievedTxs;

    state::STATE coldWalletSyncState = state::STATE::NONE;
};


}

#endif //MWC_QT_WALLET_ZZ_HEART_BEAT_H