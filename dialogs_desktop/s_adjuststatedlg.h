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

#ifndef S_ADJUSTSTATEDLG_H
#define S_ADJUSTSTATEDLG_H

#include "../control_desktop/mwcdialog.h"

namespace Ui {
class AdjustStateDlg;
}

namespace bridge {
class Swap;
}

namespace dlg {

class AdjustStateDlg : public control::MwcDialog {
    Q_OBJECT

public:
    explicit AdjustStateDlg(QWidget *parent, QString tradeId);

    ~AdjustStateDlg();

private
slots:
    void on_applyButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::AdjustStateDlg *ui;
    bridge::Swap * swap = nullptr;
    QString tradeId;
};

}

#endif // S_ADJUSTSTATEDLG_H
