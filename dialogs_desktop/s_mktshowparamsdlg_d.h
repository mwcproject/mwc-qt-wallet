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

#ifndef S_MKTSHOWPARAMSDLG_D_H
#define S_MKTSHOWPARAMSDLG_D_H

#include "../control_desktop/mwcdialog.h"

/*
namespace Ui {
class MktShowParamsDlg;
}

namespace bridge {
class Swap;
}

namespace dlg {

class MktShowParamsDlg : public control::MwcDialog {
Q_OBJECT

public:
    explicit MktShowParamsDlg(QWidget *parent, double feeLevel, bool selling, const QString & secondaryCurrency, double minMwcAmount, double maxMwcAmount);
    virtual ~MktShowParamsDlg();

    // Get the resulting settings
    void getData(double & feeLevel, bool & selling, QString & secondaryCurrency, double & minMwcAmount, double & maxMwcAmount) const;

private slots:
    void on_cancelButton_clicked();
    void on_applyButton_clicked();

private:
    Ui::MktShowParamsDlg *ui;
    bridge::Swap * swap = nullptr;

    double feeLevel;
    bool selling;
    QString secondaryCurrency;
    double minMwcAmount;
    double maxMwcAmount;
};

}*/

#endif // S_MKTSHOWPARAMSDLG_D_H
