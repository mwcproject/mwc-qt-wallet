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
#include "../util/Json.h"
#include "../wallet/wallet.h"
#include "../windows/g_filetransaction_w.h"

namespace wnd {
class FinalizeUpload;
class FileTransaction;
}

namespace state {

// Finalize transaction. Probably the only state that Doesn't have a special page(window)
// Workflow is supported by navigation window
class Finalize : public QObject, public State, public wnd::FileTransactionWndHandler {
    Q_OBJECT
public:
    Finalize( StateContext * context);
    virtual ~Finalize() override;

    void deleteFinalizeWnd(wnd::FinalizeUpload * wnd) { if (wnd==uploadWnd) uploadWnd = nullptr; }

    QString getFileGenerationPath();
    void updateFileGenerationPath(QString path);

    // Process to the next Step, show transaction details
    void fileTransactionUploaded( const QString & fileName, const util::FileTransactionInfo & transInfo );

public:
    // wnd::FileTransactionWndHandler
    virtual void ftBack() override;
    virtual void deleteFileTransactionWnd(wnd::FileTransaction * wnd) override { if (wnd==fileTransWnd) fileTransWnd = nullptr; }
    // Expected that user already made all possible appruvals
    virtual void ftContinue(QString fileName) override;

    virtual state::StateContext * getContext() override;


protected:
    virtual NextStateRespond execute() override;

private slots:
    void onFinalizeFile( bool success, QStringList errors, QString fileName );
    void onAllTransactions( QVector<wallet::WalletTransaction> Transactions);
    void onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections );
private:
    wnd::FinalizeUpload * uploadWnd = nullptr;
    wnd::FileTransaction * fileTransWnd = nullptr;

    // We can use transactions to obtain additional data about send to address, transaction Date
    QVector<wallet::WalletTransaction> allTransactions;
    int lastNodeHeight = 0;
};


}

#endif //MWC_QT_WALLET_FINALIZE_H
