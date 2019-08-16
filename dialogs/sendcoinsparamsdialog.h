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

#ifndef SENDCOINSPARAMSDIALOG_H
#define SENDCOINSPARAMSDIALOG_H

#include "../control/mwcdialog.h"
#include "../core/appcontext.h"

namespace Ui {
class SendCoinsParamsDialog;
}

namespace wnd {

class SendCoinsParamsDialog : public control::MwcDialog
{
    Q_OBJECT

public:
    explicit SendCoinsParamsDialog(QWidget *parent, const core::SendCoinsParams & params);
    virtual ~SendCoinsParamsDialog() override;

    core::SendCoinsParams getSendCoinsParams() const {return params;}

private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::SendCoinsParamsDialog *ui;
    core::SendCoinsParams params;
};

}

#endif // SENDCOINSPARAMSDIALOG_H
