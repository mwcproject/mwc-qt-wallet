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

#ifndef MWC_QT_WALLET_FINALIZE_H
#define MWC_QT_WALLET_FINALIZE_H

#include "state.h"
#include <QObject>
#include <QMap>
#include "../util/Json.h"
#include "../wallet/wallet.h"

namespace state {

const QString FINALIZE_CALLER_ID = "Finalize";

// Finalize transaction. Probably the only state that Doesn't have a special page(window)
// Workflow is supported by navigation window
class Finalize : public QObject, public State {
    Q_OBJECT
public:
    Finalize( StateContext * context);
    virtual ~Finalize() override;

    void uploadFileTransaction(QString fileName);

public:
    // wnd::FileTransactionWndHandler
    void ftBack();
    // Expected that user already made all possible appruvals
    void ftContinue(QString fileName, QString resultTxFileName, bool fluff);

    bool needResultTxFileName();

    QString getResultTxPath();
    void updateResultTxPath(QString path);

    bool isNodeHealthy() const {return nodeIsHealthy;}

protected:
    virtual NextStateRespond execute() override;
    virtual QString getHelpDocName() override {return "finalize.html";}

private slots:
    void onFinalizeFile( bool success, QStringList errors, QString fileName );
    void onAllTransactions( QVector<wallet::WalletTransaction> Transactions);
    void onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections );
private:
    // We can use transactions to obtain additional data about send to address, transaction Date
    QVector<wallet::WalletTransaction> allTransactions;
    int lastNodeHeight = 0;

    // History of submitted file transactions
    QMap<QString, util::FileTransactionInfo> file2TransactionsInfo;

    bool nodeIsHealthy = false;
};


}

#endif //MWC_QT_WALLET_FINALIZE_H
