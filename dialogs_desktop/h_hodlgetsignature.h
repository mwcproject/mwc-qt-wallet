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

#ifndef H_HODLGETSIGNATURE_H
#define H_HODLGETSIGNATURE_H

#include "../control_desktop/mwcdialog.h"
#include "../wallet/wallet.h"

namespace Ui {
class HodlGetSignature;
}

namespace bridge {
class Util;
}

namespace dlg {

class HodlGetSignature : public control::MwcDialog
{
    Q_OBJECT

public:
    explicit HodlGetSignature(QWidget *parent, QString message);
    ~HodlGetSignature();

    const QString & getSignature() const {return signature;}

private slots:
    void on_cancelButton_clicked();
    void on_continueButton_clicked();
    void on_signature_textChanged();
private:
    Ui::HodlGetSignature *ui;
    bridge::Util * util = nullptr;
    QString signature;
};

}

#endif // H_HODLGETSIGNATURE_H
