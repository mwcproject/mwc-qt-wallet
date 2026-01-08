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


#ifndef SENDCONFIRMATIONSPDLG_H
#define SENDCONFIRMATIONSPDLG_H

#include <QDialog>

#include "../control_desktop/mwcdialog.h"

namespace Ui {
class SendConfirmationSlatePackDlg;
}

namespace bridge {
    class Config;
    class Wallet;
}


namespace dlg {

class SendConfirmationSlatePackDlg : public control::MwcDialog
{
    Q_OBJECT

public:
    explicit SendConfirmationSlatePackDlg(QWidget *parent, QString title, QString messageBody, double widthScale,
                int inputsNum, int ttl);
    ~SendConfirmationSlatePackDlg();

    int getTTlBlocks() const {return ttl_blocks;}
private slots:
    void on_confirmBtn_clicked();

    void on_declineBtn_clicked();

    void on_outputsEdit_textChanged(const QString &arg1);

    void on_passwordEdit_textChanged(const QString &arg1);

private:
    void updateMessageText();
    void checkPasswordStatus();

private:
    Ui::SendConfirmationSlatePackDlg *ui;
    bridge::Config * config = nullptr;
    bridge::Wallet * wallet = nullptr;

    int ttl_blocks = -1;
    QString messageBody;
    int inputsNum;
};

}

#endif // SENDCONFIRMATIONSPDLG_H
