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

#include "e_outputs_w.h"
#include "ui_e_outputs.h"
#include <QDebug>
#include "../control_desktop/messagebox.h"
#include "../dialogs_desktop/e_showoutputdlg.h"
#include "../util_desktop/timeoutlock.h"
#include "../bridge/config_b.h"
#include "../bridge/hodlstatus_b.h"
#include "../bridge/wallet_b.h"
#include "../bridge/wnd/e_outputs_b.h"
#include "../control_desktop/richbutton.h"

namespace wnd {

// static
bool Outputs::lockMessageWasShown = false;

//////////////////////////////////////////////////////////////////////////////////////////////////////

void OutputData::setBtns(control::RichItem * _ritem,
                         QWidget * _markWnd,
                         control::RichButton * _lockBtn,
                         QLabel * _lockLabel,
                         QLabel * _comment) {
    ritem = _ritem;
    markWnd = _markWnd;
    lockBtn = _lockBtn;
    lockLabel = _lockLabel;
    comment = _comment;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

Outputs::Outputs(QWidget *parent) :
        core::NavWnd(parent),
        ui(new Ui::Outputs) {
    ui->setupUi(this);

    config = new bridge::Config(this);
    hodlStatus = new bridge::HodlStatus(this);
    wallet = new bridge::Wallet(this);
    outputs = new bridge::Outputs(this);

    QObject::connect( wallet, &bridge::Wallet::sgnOutputs,
                      this, &Outputs::onSgnOutputs, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnWalletBalanceUpdated,
                      this, &Outputs::onSgnWalletBalanceUpdated, Qt::QueuedConnection);
    QObject::connect( wallet, &bridge::Wallet::sgnNewNotificationMessage,
                      this, &Outputs::onSgnNewNotificationMessage, Qt::QueuedConnection);

    QObject::connect(ui->outputsTable, &control::RichVBox::onItemActivated,
                     this, &Outputs::onItemActivated, Qt::QueuedConnection);

    ui->progress->initLoader(true);
    ui->progressFrame->hide();

    bool showAll = config->isShowOutputAll();
    ui->showUnspent->setText( QString("Show Spent: ") + (showAll ? "Yes" : "No") );

    QString accName = updateAccountsData();

    inHodl = hodlStatus->isInHodl();
    canLockOutputs = config->isLockOutputEnabled();

    requestOutputs(accName);
}

void Outputs::panelWndStarted() {
    ui->outputsTable->setFocus();
}


Outputs::~Outputs() {
    delete ui;
}

bool Outputs::calcMarkFlag(const wallet::WalletOutput & out) {
    QString lockState = calcLockedState(out);
    return out.status == "Unconfirmed" || out.status == "Locked" || lockState == "YES";
}

void Outputs::updateShownData() {
    ui->outputsTable->clearAll();

    int total = allData.size();

    qDebug() << "updating output table for " << total << " outputs";
    // Printing first 200 outputs. For normal usage 200 is enough. The re
    for (int i = allData.size()-1; i >= std::max(0, allData.size()-5000); i--) {
        auto &out = allData[i].output;

        // Data filtering was done on outputs request level. No need to filter out anything

        // return "N/A, Yes, "No"
        QString lockState = calcLockedState(out);
        bool mark = calcMarkFlag(out);

        control::RichItem * itm = control::createMarkedItem(QString::number(i), ui->outputsTable, mark );

        QWidget * markWnd = (QWidget *) itm->getCurrentWidget();

        // First row with Info about the commit
        {
            itm->hbox().setContentsMargins(0, 0, 0, 0).setSpacing(4);
            // Adding Icon and a text
            if (out.status == "Unconfirmed") {
                itm->addWidget(control::createIcon(itm, ":/img/iconUnconfirmed@2x.svg", control::ROW_HEIGHT, control::ROW_HEIGHT));
            } else if (out.status == "Unspent") {
                if (out.coinbase)
                    itm->addWidget(control::createIcon(itm, ":/img/iconCoinbase@2x.svg", control::ROW_HEIGHT, control::ROW_HEIGHT));
                else
                    itm->addWidget(control::createIcon(itm, ":/img/iconReceived@2x.svg", control::ROW_HEIGHT, control::ROW_HEIGHT));
            } else if (out.status == "Locked") {
                itm->addWidget( control::createIcon(itm, ":/img/iconLock@2x.svg", control::ROW_HEIGHT, control::ROW_HEIGHT));
            } else if (out.status == "Spent") {
                itm->addWidget( control::createIcon(itm, ":/img/iconSent@2x.svg", control::ROW_HEIGHT, control::ROW_HEIGHT));
            } else {
                Q_ASSERT(false);
            }

            itm->addWidget(control::crateLabel(itm, false, false, out.status));
            itm->addSpacer();

            bool hok = false;
            int height = out.blockHeight.toInt(&hok);
            bool lok = false;
            int lockH = out.lockedUntil.toInt(&lok);

            if (hok) {
                itm->addWidget(control::crateLabel(itm, false, true, "Block: " + out.blockHeight));
            }
            if (lok && hok && lockH > height) {
                itm->addFixedHSpacer(control::LEFT_MARK_SPACING).addWidget(
                        control::crateLabel(itm, false, true, "Lock Height: " + out.lockedUntil));
            }
            itm->pop();
        } // First line

        itm->addWidget( control::createHorzLine(itm) );

        QLabel * lockL = nullptr;
        control::RichButton * lockBtn = nullptr;

        { // Line with amount
            itm->hbox().setContentsMargins(0, 0, 0, 0).setSpacing(4);

            itm->addWidget(control::createIcon(itm, ":/img/iconLock@2x", control::ROW_HEIGHT, control::ROW_HEIGHT));
            lockL = (QLabel*)itm->getCurrentWidget();

            if (lockState != "YES") {
                lockL->hide();
            }

            itm->addWidget( control::crateLabel(itm, false, false, util::nano2one(out.valueNano) + " MWC", control::FONT_LARGE));
            itm->addSpacer();

            // Add lock button if it is applicable
            if ( lockState == "YES" ) {
                itm->addWidget(new control::RichButton(itm, "Unlock", 60, control::ROW_HEIGHT, "Unlock this output and make it spendable"));
                lockBtn = (control::RichButton*) itm->getCurrentWidget();
            }
            else if (lockState == "NO") {
                itm->addWidget(new control::RichButton(itm, "Lock", 60, control::ROW_HEIGHT, "Lock this output and make it non spendable"));
                lockBtn = (control::RichButton*) itm->getCurrentWidget();
            }

            if (lockBtn) {
                lockBtn->setCallback(this, QString::number(i) );
                itm->addFixedHSpacer(control::ROW_HEIGHT);
            }

            itm->addWidget( control::crateLabel(itm, false, true, "Conf: " + out.numOfConfirms));

            itm->pop();
        }

        // line with commit
        itm->addWidget( control::crateLabel(itm, false, true, out.outputCommitment, control::FONT_SMALL) );

        // And the last optional line is comment
        QString outputNote = config->getOutputNote(out.outputCommitment);
        itm->addWidget( control::crateLabel(itm, true, false,outputNote));
        QLabel * noteL = (QLabel *) itm->getCurrentWidget();
        if (outputNote.isEmpty())
            noteL->hide();

        itm->apply();

        allData[i].setBtns(itm, markWnd, lockBtn, lockL, noteL);
        ui->outputsTable->addItem(itm);
    }

    ui->outputsTable->apply();
}

void Outputs::richButtonPressed(control::RichButton * button, QString coockie) {
    // cookie is index.
    int idx = coockie.toInt();

    if (idx>=0 && idx<allData.size()) {
        wallet::WalletOutput & selected = allData[idx].output;
        bool locked = !config->isLockedOutput(selected.outputCommitment);
        config->setLockedOutput(locked, selected.outputCommitment);

        updateOutputState( idx, locked );
    }
}

bool Outputs::updateOutputState(int idx, bool lock) {
    if ( idx>=0 && idx<allData.size() && showLockMessage() ) {

        wallet::WalletOutput & selected = allData[idx].output;
        QLabel * lockL = allData[idx].lockLabel;
        control::RichButton * lockBtn = allData[idx].lockBtn;

        config->setLockedOutput(lock, selected.outputCommitment);
        if (lock) {
            if (lockBtn)
                lockBtn->setText("Unlock");
            if (lockL)
                lockL->show();
        }
        else {
            if (lockBtn)
                lockBtn->setText("Lock");
            if (lockL)
                lockL->hide();
        }

        bool mark = calcMarkFlag(selected);
        allData[idx].markWnd->setStyleSheet( mark ? control::LEFT_MARK_ON : control::LEFT_MARK_OFF );

        return true;
    }
    return false;
}

QString Outputs::currentSelectedAccount() {
    return ui->accountComboBox->currentData().toString();
}

void Outputs::onSgnOutputs( QString account, bool showSpent, QString height, QVector<QString> outputs) {
    Q_UNUSED(height);

    if (account != currentSelectedAccount() || showSpent != config->isShowOutputAll() )
        return;

    ui->progressFrame->hide();
    ui->tableFrame->show();

    allData.clear();

    for (const QString & s : outputs) {
        OutputData out;
        out.output = wallet::WalletOutput::fromJson(s);
        allData.push_back( out );
    }

    updateShownData();
}

void Outputs::on_refreshButton_clicked() {
    requestOutputs(currentSelectedAccount());
}

// Request and reset page counter
void Outputs::requestOutputs(QString account) {
    allData.clear();

    ui->progressFrame->show();
    ui->tableFrame->hide();

    updateShownData();

    wallet->requestOutputs(account, config->isShowOutputAll(), true);
}

void Outputs::on_accountComboBox_activated(int index) {
    Q_UNUSED(index)
    // Selecting the active account
    QString selectedAccount = currentSelectedAccount();
    if (!selectedAccount.isEmpty()) {
        wallet->switchAccount(selectedAccount);
        requestOutputs(selectedAccount);
    }
}

void Outputs::onSgnWalletBalanceUpdated() {
    updateAccountsData();
}

QString Outputs::updateAccountsData() {

    QVector<QString> accounts = wallet->getWalletBalance(true,false,true);
    QString selectedAccount = wallet->getCurrentAccountName();

    int selectedAccIdx = 0;

    ui->accountComboBox->clear();

    int idx = 0;
    for (int i=1; i<accounts.size(); i+=2) {
        if (accounts[i-1] == selectedAccount)
            selectedAccIdx = idx;

        ui->accountComboBox->addItem(accounts[i], accounts[i-1]);
        idx++;
    }
    ui->accountComboBox->setCurrentIndex(selectedAccIdx);
    return currentSelectedAccount();
}


void Outputs::on_showUnspent_clicked() {
    bool showAll = ! config->isShowOutputAll();
    config->setShowOutputAll( showAll );
    ui->showUnspent->setText( QString("Show Spent: ") + (showAll ? "Yes" : "No") );

    on_refreshButton_clicked();
}

// return "N/A, YES, "NO"
QString Outputs::calcLockedState(const wallet::WalletOutput & output) {
    if (!canLockOutputs)
        return "N/A";

    QString lockState = "N/A";
    if (output.isUnspent()) {
            lockState = config->isLockedOutput(output.outputCommitment) ? "YES" : "NO";
    }
    return lockState;
}

// return true if user fine with lock changes
bool Outputs::showLockMessage() {
    if (lockMessageWasShown)
        return true;

    if ( core::WndManager::RETURN_CODE::BTN2 != control::MessageBox::questionText(this, "Locking Output",
            "By manually locking output you are preventing it from spending by QT wallet.\nLocked outputs amount will be shown as Locked balance until you change this.",
            "Cancel", "Continue",
            "Cancel, I don't want to change lock status my output",
            "Continue and change lock status",
            false, true) ) {
        return false;
    }

    lockMessageWasShown = true;
    return lockMessageWasShown;
}

void Outputs::onSgnNewNotificationMessage(int level, QString message) {
    Q_UNUSED(level)
    if (message.contains("Changing status for output")) {
        on_refreshButton_clicked();
    }
}

void Outputs::onItemActivated(QString itemId) {
    util::TimeoutLockObject to("Events");

    int idx = itemId.toInt();
    if (idx>=0 && idx<allData.size()) {
        wallet::WalletOutput out = allData[idx].output;
        util::TimeoutLockObject to("Outputs");

        bool locked = config->isLockedOutput(out.outputCommitment);

        QString account = currentSelectedAccount();
        QString outputNote = config->getOutputNote(out.outputCommitment);
        dlg::ShowOutputDlg showOutputDlg(this, out,
                                         outputNote,
                                         config->isLockOutputEnabled(), locked);

        if (showOutputDlg.exec() == QDialog::Accepted) {
            if (locked != showOutputDlg.isLocked()) {
                if (updateOutputState(idx, showOutputDlg.isLocked())) {
                    config->setLockedOutput(showOutputDlg.isLocked(), out.outputCommitment);
                }
            }

            QString resNote = showOutputDlg.getResultOutputNote();
            if (resNote != outputNote ) {
                if (resNote.isEmpty()) {
                    config->deleteOutputNote( out.outputCommitment );
                    allData[idx].comment->hide();
                }
                else {
                    // add new note or update existing note for this commitment
                    config->updateOutputNote(out.outputCommitment, resNote);
                    allData[idx].comment->setText(resNote);
                    allData[idx].comment->show();
                }
            }
        }
    }
}


}  // end namespace wnd

