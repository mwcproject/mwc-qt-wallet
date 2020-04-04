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

#ifndef OUTPUTS_H
#define OUTPUTS_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../core/Notification.h"

namespace wnd {
class Outputs;
}

namespace state {

const QString OUTPUT_SPENT_STATUS    = "Spent";

struct CachedOutputInfo {
    QString currentAccount;
    bool showSpent = false;
    int64_t height = -1;

    QVector<wallet::WalletOutput> allAccountOutputs;
    QVector<wallet::WalletOutput> unspentOutputs;

    void resetCache(QString account, bool showSpent);
    void setCache(QString account, int64_t height, QVector<wallet::WalletOutput> outputs);
};


class Outputs : public QObject, public State
{
    Q_OBJECT
public:
    Outputs( StateContext * context);
    virtual ~Outputs() override;

    void deleteWnd(wnd::Outputs * w) { if(w==wnd) wnd = nullptr;}

    // request wallet for outputs
    void requestOutputs(QString account, bool show_spent, bool enforceSync);

    QString getCurrentAccountName() const;

    // IO for columns widhts
    QVector<int> getColumnsWidhts() const;
    void updateColumnsWidhts(const QVector<int> & widths);

    void switchCurrentAccount(const wallet::AccountInfo & account);
    QVector<wallet::AccountInfo> getWalletBalance();

private slots:
    void onOutputs( QString account, int64_t height, QVector<wallet::WalletOutput> outputs);

    void onWalletBalanceUpdated();

    void onNewNotificationMessage(notify::MESSAGE_LEVEL  level, QString message);

    void onLoginResult(bool ok);
    void onRootPublicKey( bool success, QString errMsg, QString rootPubKey, QString message, QString signature );

protected:
    virtual NextStateRespond execute() override;
    virtual QString getHelpDocName() override {return "outputs.html";}

private:
    wnd::Outputs * wnd = nullptr;
    CachedOutputInfo cachedOutputs;
};

}

#endif // OUTPUTS_H
