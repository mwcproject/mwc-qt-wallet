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
class FileTransactionFinalize;
}

namespace bridge {
class Finalize;
class Wallet;
class Config;
class Util;
}

namespace wnd {

class FileTransactionFinalize : public core::NavWnd {
Q_OBJECT

public:
    explicit FileTransactionFinalize(QWidget *parent,
             const QString & fileNameOrSlatepack, const util::FileTransactionInfo & transInfo,
             int nodeHeight);

    ~FileTransactionFinalize();

private slots:
    void on_cancelButton_clicked();
    void on_processButton_clicked();
    void on_resultTransFileNameSelect_clicked();

    void onSgnHideProgress();

    void sgnTransactionById( bool success, QString account, QString height, QString transaction,
                             QVector<QString> outputs, QVector<QString> messages );
private:
    Ui::FileTransactionFinalize *ui;
    bridge::Wallet * wallet = nullptr;
    bridge::Config * config = nullptr;
    bridge::Util * util = nullptr;
    bridge::Finalize * finalize = nullptr;
    QString callerId;
    QString fileNameOrSlatepack;
    QString txUuid;
    bool    receivePage = false;
};

}

#endif // FINALIZETRANSACTION_H
