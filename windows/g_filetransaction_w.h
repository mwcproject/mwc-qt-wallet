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

#include "../core/navwnd.h"
#include "../util/Json.h"
#include "../wallet/wallet.h"

namespace Ui {
class FileTransaction;
}

namespace state {
class Finalize;
}

namespace wnd {

class FileTransaction;

class FileTransactionWndHandler {
public:
    virtual void ftBack() = 0;
    virtual void deleteFileTransactionWnd(FileTransaction * wnd) = 0;
    virtual void ftContinue(QString fileName, QString resultTxFileName) = 0;

    virtual bool needResultTxFileName() = 0;

    virtual QString getResultTxPath() = 0;
    virtual void updateResultTxPath(QString path) = 0;

    virtual state::StateContext * getContext() = 0;
};



class FileTransaction : public core::NavWnd {
Q_OBJECT

public:
    explicit FileTransaction(QWidget *parent, FileTransactionWndHandler * handler,
             const QString & fileName, const util::FileTransactionInfo & transInfo,
             const wallet::WalletTransaction & transaction,
             int nodeHeight,
             QString transactionType, QString processButtonName);

    ~FileTransaction();

    void hideProgress();

private slots:
    void on_cancelButton_clicked();
    void on_processButton_clicked();

    void on_resultTransFileNameSelect_clicked();

private:
    Ui::FileTransaction *ui;
    FileTransactionWndHandler * handler = nullptr;
    QString transactionFileName;
};

}

#endif // FINALIZETRANSACTION_H
