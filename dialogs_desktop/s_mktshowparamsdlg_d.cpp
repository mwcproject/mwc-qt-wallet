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


#include "s_mktshowparamsdlg_d.h"
#include "ui_s_mktshowparamsdlg_d.h"
#include "../windows_desktop/s_mrktswlist_w.h"
#include "../bridge/wnd/swap_b.h"
#include "../control_desktop/messagebox.h"

namespace dlg {

MktShowParamsDlg::MktShowParamsDlg(QWidget *parent, double _feeLevel, bool _selling, const QString & _secondaryCurrency, double _minMwcAmount, double _maxMwcAmount) :
        control::MwcDialog(parent),
        ui(new Ui::MktShowParamsDlg),
        feeLevel(_feeLevel),
        selling(_selling),
        secondaryCurrency(_secondaryCurrency),
        minMwcAmount(_minMwcAmount),
        maxMwcAmount(_maxMwcAmount)
{
    ui->setupUi(this);

    bridge::Swap * swap = new bridge::Swap(this);

    int selected = wnd::MKT_FEES.size() / 2;
    for (const auto &f : wnd::MKT_FEES) {
        if (feeLevel < f.second + 0.0001)
            selected = ui->feeLevel->count();

        ui->feeLevel->addItem(f.first, QVariant::fromValue(f.second));
    }
    ui->feeLevel->setCurrentIndex(selected);

    ui->buySellCombo->setCurrentIndex( selling ? 1 : 0 );

    QVector<QString> scList = swap->secondaryCurrencyList();
    int selIdx = 0;
    for (int i=0; i<scList.size(); i++) {
        const auto & sc = scList[i];
        ui->secCurrencyCombo->addItem(sc);
        if (sc == secondaryCurrency) {
            selIdx = i;
        }
    }
    ui->secCurrencyCombo->setCurrentIndex(selIdx);

    if (minMwcAmount>0)
        ui->rangeMin->setText( QString::number(minMwcAmount) );

    if (maxMwcAmount>0)
        ui->rangeMax->setText( QString::number(maxMwcAmount) );
}

MktShowParamsDlg::~MktShowParamsDlg() {
    delete ui;
}

// Get the resulting settings
void MktShowParamsDlg::getData(double & _feeLevel, bool & _selling, QString & _secondaryCurrency, double & _minMwcAmount, double & _maxMwcAmount) const {
    _feeLevel = feeLevel;
    _selling = selling;
    _secondaryCurrency = secondaryCurrency;
    _minMwcAmount = minMwcAmount;
    _maxMwcAmount = maxMwcAmount;
}


void MktShowParamsDlg::on_cancelButton_clicked() {
    reject();
}

void MktShowParamsDlg::on_applyButton_clicked() {
    bool minOk = false;
    bool maxOk = false;

    QString minStr = ui->rangeMin->text();
    double minVal = minStr.toDouble(&minOk);
    QString maxStr = ui->rangeMax->text();
    double maxVal = maxStr.toDouble(&maxOk);

    if (minStr.isEmpty()) {
        minVal = 0.0;
    }
    else {
        if (!minOk || minVal<0) {
            control::MessageBox::messageText(this, "Incorrect value", "Please minimal range value, or keep it empty.");
            ui->rangeMin->setFocus();
            return;
        }
    }

    if (maxStr.isEmpty()) {
        maxVal = 0.0;
    }
    else {
        if (!maxOk || maxVal<0) {
            control::MessageBox::messageText(this, "Incorrect value", "Please maximum range value, or keep it empty.");
            ui->rangeMax->setFocus();
            return;
        }
    }

    if (maxVal<=minVal && maxVal>0.0 && minVal>0.0) {
        control::MessageBox::messageText(this, "Incorrect value", "Please the range correctly. Minimum value should be smaller than maximum value.");
        ui->rangeMax->setFocus();
        return;
    }

    feeLevel = ui->feeLevel->currentData().toDouble();
    selling = ui->buySellCombo->currentIndex() == 1;
    secondaryCurrency = ui->secCurrencyCombo->currentText();
    minMwcAmount = minVal;
    maxMwcAmount = maxVal;

    accept();
}

}