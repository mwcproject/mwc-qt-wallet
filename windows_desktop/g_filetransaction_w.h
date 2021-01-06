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

#ifndef FINALIZETRANSACTION_H
#define FINALIZETRANSACTION_H

#include "../core_desktop/navwnd.h"
#include "../util/Json.h"
#include "../wallet/wallet.h"

namespace Ui {
class FileTransaction;
}

namespace bridge {
class FileTransaction;
class Wallet;
class Config;
class Util;
}

namespace wnd {

class FileTransaction : public core::NavWnd {
Q_OBJECT

public:
    explicit FileTransaction(QWidget *parent,
             QString callerId,
             const QString & fileNameOrSlatepack, const util::FileTransactionInfo & transInfo,
             int nodeHeight,
             QString transactionType, QString processButtonName);

    ~FileTransaction();

private slots:
    void on_cancelButton_clicked();
    void on_processButton_clicked();
    void on_resultTransFileNameSelect_clicked();

    void onSgnHideProgress();
private:
    Ui::FileTransaction *ui;
    bridge::FileTransaction * fileTransaction = nullptr;
    bridge::Wallet * wallet = nullptr;
    bridge::Config * config = nullptr;
    bridge::Util * util = nullptr;
    QString callerId;
    QString fileNameOrSlatepack;
    QString txUuid;
};

}

#endif // FINALIZETRANSACTION_H
