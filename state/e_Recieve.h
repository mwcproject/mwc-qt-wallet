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

#ifndef MWC_QT_WALLET_RECIEVE_H
#define MWC_QT_WALLET_RECIEVE_H

#include "state.h"
#include "../wallet/wallet.h"

namespace wnd {
class Recieve;
}

namespace state {

class Recieve : public QObject, public State {
    Q_OBJECT
public:
    Recieve( StateContext * context );
    virtual ~Recieve() override;

    QString  getReceiveAccount();
    void  setReceiveAccount(QString accountName );
    QVector<wallet::AccountInfo>  getWalletBalance();

    QString getFileGenerationPath();
    void updateFileGenerationPath(QString path);

    void signTransaction( QString fileName );

    void deletedWnd(wnd::Recieve * w) { if(w==wnd) wnd = nullptr;}
protected:
    virtual NextStateRespond execute() override;
    virtual QString getHelpDocName() override {return "receive.html";}

    void respReceiveFile( bool success, QStringList errors, QString inFileName );

private slots:
    void onMwcMqListenerStatus(bool online);
    void onKeybaseListenerStatus(bool online);
    void onMwcAddressWithIndex(QString mwcAddress, int idx);

private:
    wnd::Recieve * wnd = nullptr;
};


}

#endif //MWC_QT_WALLET_RECIEVE_H
