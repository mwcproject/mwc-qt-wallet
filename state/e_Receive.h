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

#ifndef MWC_QT_WALLET_RECEIVE_H
#define MWC_QT_WALLET_RECEIVE_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../windows/g_filetransaction_w.h"

namespace wnd {
class Receive;
}

namespace state {

class Receive : public QObject, public State, public wnd::FileTransactionWndHandler {
    Q_OBJECT
public:
    Receive( StateContext * context );
    virtual ~Receive() override;

    QString  getReceiveAccount();
    void  setReceiveAccount(QString accountName );
    QVector<wallet::AccountInfo>  getWalletBalance();

    QString getFileGenerationPath();
    void updateFileGenerationPath(QString path);

    void signTransaction( QString fileName );

    void deletedWnd(wnd::Receive * w) { if(w==wnd) wnd = nullptr;}

public:
    // wnd::FileTransactionWndHandler

    virtual void ftBack() override;
    virtual void deleteFileTransactionWnd(wnd::FileTransaction * wnd) override {  if (wnd==fileTransWnd) fileTransWnd=nullptr; }
    virtual void ftContinue(QString fileName, QString resultTxFileName) override;
    virtual state::StateContext * getContext() override;

    virtual bool needResultTxFileName() override {return false;}

    virtual QString getResultTxPath() override {return "";}
    virtual void updateResultTxPath(QString path)  override {Q_UNUSED(path)}

    virtual bool isNodeHealthy() const override {return true;}
protected:
    virtual NextStateRespond execute() override;
    virtual QString getHelpDocName() override {return "receive.html";}

    void respReceiveFile( bool success, QStringList errors, QString inFileName );

private slots:
    void onMwcMqListenerStatus(bool online);
    void onKeybaseListenerStatus(bool online);
    void onMwcAddressWithIndex(QString mwcAddress, int idx);
    void onNodeStatus( bool online, QString errMsg, int nodeHeight, int peerHeight, int64_t totalDifficulty, int connections );
    void onWalletBalanceUpdated();
private:
    wnd::Receive * wnd = nullptr;
    wnd::FileTransaction * fileTransWnd = nullptr;
    int lastNodeHeight = 0;
};


}

#endif //MWC_QT_WALLET_RECEIVE_H
