// Copyright 2021 The MWC Developers
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

#ifndef G_INPUTSLATEPACKDLG_H
#define G_INPUTSLATEPACKDLG_H

#include "../control_desktop/mwcdialog.h"
#include "../util/Json.h"

namespace bridge {
class Wallet;
class Util;
}

namespace Ui {
class InputSlatepackDlg;
}

namespace dlg {

class InputSlatepackDlg : public control::MwcDialog {
Q_OBJECT
public:
    explicit InputSlatepackDlg(QString expectedContent, QString expectedContentDescription, util::FileTransactionType txType,  QWidget *parent);

    ~InputSlatepackDlg();

    const QString &getSlatepack() const { return slatepack; }
    const QString &getSlateJson() const { return slateJson; }
    const QString &getSenderAddress() const { return sender; }

protected:
    void updateButtons();

    void initiateSlateVerification(const QString &slate2check);

private slots:

    void on_slatepackEdit_textChanged();
    void on_cancelButton_clicked();
    void on_continueButton_clicked();

    void onSgnDecodeSlatepack(QString error, QString slatepack, QString slateJSon, QString content, QString sender, QString recipient);

private:
    Ui::InputSlatepackDlg *ui;
    bridge::Wallet  *wallet = nullptr;
    bridge::Util    *util = nullptr;
    QString expectedContent;
    QString expectedContentDescription;
    util::FileTransactionType txType;

    QString slatepack; // The content
    QString slateJson;
    QString sender;
    bool isSpValid = false;
    QString spInProgress;
};

}

#endif // G_INPUTSLATEPACKDLG_H
