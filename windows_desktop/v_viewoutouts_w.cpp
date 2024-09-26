// Copyright 2024 The MWC Developers
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

#include "v_viewoutputs_w.h"
#include "ui_v_viewoutputs_w.h"
#include "../bridge/wnd/v_viewoutputs_b.h"
#include "../util_desktop/timeoutlock.h"
#include "../dialogs_desktop/v_showviewoutputdlg.h"
#include <QDebug>

namespace wnd {

ViewOutputs::ViewOutputs(QWidget *parent) :
    core::NavWnd(parent),
    ui(new Ui::ViewOutputs)
{
    ui->setupUi(this);
    viewAcounts = new bridge::ViewOutputs(this);

    QObject::connect(viewAcounts, &bridge::ViewOutputs::onSgnViewOutputs,
                     this, &ViewOutputs::onSgnViewOutputs, Qt::QueuedConnection);

    QObject::connect(ui->outputsTable, &control::RichVBox::onItemActivated,
                     this, &ViewOutputs::onItemActivated, Qt::QueuedConnection);

}

ViewOutputs::~ViewOutputs()
{
    delete ui;
}

void ViewOutputs::on_backButton_clicked()
{
    viewAcounts->backFromOutputsView();
}

void ViewOutputs::onSgnViewOutputs(QString viewingKey, QVector<QString> outputs, QString totalAmount) {
    allData.clear();

    for (const QString & s : outputs) {
        OutputData out;
        out.output = wallet::WalletOutput::fromJson(s);
        allData.push_back( out );
    }

    ui->viewingKey->setText( "Total: " + totalAmount + " MWC  " + viewingKey );

    updateShownData(false);
}

void ViewOutputs::updateShownData(bool resetScrollData) {
    ui->outputsTable->clearAll(resetScrollData);

    int total = allData.size();

    qDebug() << "updating output table for " << total << " outputs";
    // Printing first 200 outputs. For normal usage 200 is enough. The re
    for (int i = allData.size()-1; i >= std::max(0, allData.size()-5000); i--) {
        auto &out = allData[i].output;

        // Data filtering was done on outputs request level. No need to filter out anything

        control::RichItem * itm = control::createMarkedItem(QString::number(i), ui->outputsTable, false, "" );

        QWidget * markWnd = (QWidget *) itm->getCurrentWidget();

        // First row with Info about the commit
        {
            itm->hbox().setContentsMargins(0, 0, 0, 0).setSpacing(4);
            if (out.coinbase)
                itm->addWidget(control::createIcon(itm, ":/img/iconCoinbase@2x.svg", control::ROW_HEIGHT, control::ROW_HEIGHT));
            else
                itm->addWidget(control::createIcon(itm, ":/img/iconReceived@2x.svg", control::ROW_HEIGHT, control::ROW_HEIGHT));

            itm->addWidget(control::createLabel(itm, false, false, out.status));
            itm->addHSpacer();

            bool hok = false;
            int height = out.blockHeight.toInt(&hok);
            bool lok = false;
            int lockH = out.lockedUntil.toInt(&lok);

            if (hok) {
                itm->addWidget(control::createLabel(itm, false, true, "Block: " + out.blockHeight));
            }
            if (lok && hok && lockH > height) {
                itm->addFixedHSpacer(control::LEFT_MARK_SPACING).addWidget(
                        control::createLabel(itm, false, true, "Lock Height: " + out.lockedUntil));
            }
            itm->pop();
        } // First line

        itm->addWidget( control::createHorzLine(itm) );

        { // Line with amount
            itm->hbox().setContentsMargins(0, 0, 0, 0).setSpacing(4);

            itm->addWidget( control::createLabel(itm, false, false, util::nano2one(out.valueNano) + " MWC", control::FONT_LARGE));
            itm->addHSpacer();

            itm->addWidget( control::createLabel(itm, false, true, "Conf: " + out.numOfConfirms));

            itm->pop();
        }

        // line with commit
        {
            itm->hbox().setContentsMargins(0, 0, 0, 0);
            itm->addWidget(control::createLabel(itm, false, true, out.outputCommitment, control::FONT_SMALL));
            itm->addHSpacer();
            itm->pop();
        }

        itm->apply();

        allData[i].setBtns(itm, markWnd, nullptr, nullptr, nullptr);
        ui->outputsTable->addItem(itm);
    }

    ui->outputsTable->apply();
}

void ViewOutputs::onItemActivated(QString itemId) {
    util::TimeoutLockObject to("Events");

    int idx = itemId.toInt();
    if (idx>=0 && idx<allData.size()) {
        wallet::WalletOutput out = allData[idx].output;
        util::TimeoutLockObject to("Outputs");

        dlg::ShowViewOutputDlg showOutputDlg(this, out);

        if (showOutputDlg.exec() == QDialog::Accepted) {
            // It is just show, nothing happens here
        }
    }
}


}
