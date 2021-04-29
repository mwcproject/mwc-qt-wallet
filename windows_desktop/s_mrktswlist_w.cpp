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

#include "s_mrktswlist_w.h"
#include "ui_s_mrktswlist_w.h"
#include "../bridge/wnd/swapmkt_b.h"
#include "../bridge/wnd/swap_b.h"
#include "../bridge/config_b.h"
#include "../bridge/wallet_b.h"
#include "../bridge/util_b.h"
#include <QDebug>
#include "../control_desktop/messagebox.h"
#include "../util_desktop/timeoutlock.h"
#include <QTimer>
#include "../dialogs_desktop/s_mktofferdetailsdlg.h"
#include "../util/address.h"

const int BTN_FONT_SIZE = 13;
const int BTN_WIDTH = 80;

namespace wnd {

QString feeLevelValToStr(double feeLevel) {
    for (const auto &f : MKT_FEES) {
        if (feeLevel > f.fee - 0.0001)
            return f.shortName;
    }
    return "Dust";
}

MrktSwList::MrktSwList(QWidget *parent, bool selectMyOffers, bool selectFee) :
        core::NavWnd(parent, true),
        ui(new Ui::MrktSwList) {
    ui->setupUi(this);
    ui->progress->initLoader(false);

    config = new bridge::Config(this);
    swapMarketplace = new bridge::SwapMarketplace(this);
    swap = new bridge::Swap(this);
    wallet = new bridge::Wallet(this);
    util = new bridge::Util(this);

    connect(swapMarketplace, &bridge::SwapMarketplace::sgnRequestIntegrityFees, this, &MrktSwList::sgnRequestIntegrityFees,
            Qt::QueuedConnection);
    connect(swapMarketplace, &bridge::SwapMarketplace::sgnMarketPlaceOffersChanged, this, &MrktSwList::sgnMarketPlaceOffersChanged,
            Qt::QueuedConnection);
    connect(swapMarketplace, &bridge::SwapMarketplace::sgnMyOffersChanged, this, &MrktSwList::sgnMyOffersChanged,
            Qt::QueuedConnection);
    connect(swapMarketplace, &bridge::SwapMarketplace::sgnMessagingStatusChanged, this, &MrktSwList::sgnMessagingStatusChanged,
            Qt::QueuedConnection);
    connect(swapMarketplace, &bridge::SwapMarketplace::sgnWithdrawIntegrityFees, this, &MrktSwList::sgnWithdrawIntegrityFees,
            Qt::QueuedConnection);

    connect(ui->buyOffersTable, &control::RichVBox::onItemActivated, this, &MrktSwList::onBuyItemActivated,
            Qt::QueuedConnection);
    connect(ui->sellOffersTable, &control::RichVBox::onItemActivated, this, &MrktSwList::onSellItemActivated,
            Qt::QueuedConnection);

    double currentFee = swapMarketplace->getFeeLevel();
    int selected = MKT_FEES.size() / 2;
    for (const auto &f : MKT_FEES) {
        if (currentFee < f.fee + 0.0001)
            selected = ui->feeLevel->count();

        ui->feeLevel->addItem(f.longName, QVariant::fromValue(f.fee));
    }
    ui->feeLevel->setCurrentIndex(selected);

    wallet->requestNodeStatus();

    // Requesting only accounts names.
    QVector<QString> accounts = wallet->getWalletBalance(true, false, false);
    QString selectedAccount = swapMarketplace->getFeeDepositAccount();
    int selectedAcc = 0;
    for (int i = 0; i < accounts.size(); i++) {
        if (accounts[i]=="integrity")
            break; // It is the last account, we can exit

        if (accounts[i] == selectedAccount)
            selectedAcc = i;

        ui->depositAccountName->addItem(accounts[i]);
    }
    ui->depositAccountName->setCurrentIndex(selectedAcc);

    QVector<QString> scList = swap->secondaryCurrencyList();
    ui->secCurrencyCombo->addItem("All", "");
    for (int i=0; i<scList.size(); i++) {
        const auto & sc = scList[i];
        ui->secCurrencyCombo->addItem(sc, sc);
    }
    for (const auto &f : wnd::MKT_FEES) {
        ui->filterFeeLevel->addItem(f.longName, QVariant::fromValue(f.fee));
    }

    controlsReady = true;

    swapMarketplace->requestIntegrityFees();

    updateMktFilter();

    int btn = config->getMktPlaceSelectedBtn();
    if (selectMyOffers)
        btn = BTN_MY_OFFERS;

    if (selectFee)
        btn = BTN_FEES;

    updateModeButtons(btn);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateList()));
    timer->start(1000*60); // Every minute is fine. We want sometimes update expiration time
}

MrktSwList::~MrktSwList() {
    delete ui;
}

void MrktSwList::updateModeButtons(int btnId) {
    selectedTab = btnId;
    config->setMktPlaceSelectedBtn(btnId);

    ui->marketOffers->setChecked(btnId==BTN_MKT_OFFERS);
    ui->myOffers->setChecked(btnId==BTN_MY_OFFERS);
    ui->feesBtn->setChecked(btnId==BTN_FEES);

    ui->feesHolder->setVisible( btnId==BTN_FEES );

    ui->filterPlaceholder->setVisible( btnId==BTN_MKT_OFFERS );

    // 0 - buy, 1 - sell, 2 - all
    int buySell = config->getSwapMktSelling();

    ui->buyOffersTable->setVisible( btnId==BTN_MY_OFFERS || (btnId==BTN_MKT_OFFERS && (buySell==0 || buySell==2)) );
    ui->sellOffersTable->setVisible( btnId==BTN_MKT_OFFERS && (buySell==1 || buySell==2) );

    ui->refreshBtnsHolder->setVisible( btnId==BTN_MY_OFFERS );

    ui->progress->hide();

    // In case of incorrect input, we want to reset the data
    ui->mwcReservedFee->setText(QString::number(swapMarketplace->getFeeReservedAmount()));
    updateMwcReservedFeeValidity();

    updateTradeListData(true);
}

void MrktSwList::updateMktFilter() {
    int mktSelling = config->getSwapMktSelling();
    QString currency = config->getSwapMktCurrency();
    double fee = config->getSwapMktMinFeeLevel();

    ui->buySellCombo->setCurrentIndex(mktSelling);
    ui->secCurrencyCombo->setCurrentIndex(ui->secCurrencyCombo->findData(currency));
    int idx = wnd::MKT_FEES.size()-1;
    for( ;idx>=0; idx--) {
        if (fee < wnd::MKT_FEES[idx].fee + 0.0001) {
            ui->filterFeeLevel->setCurrentIndex(idx);
            break;
        }
    }

    QString status = "  " + swapMarketplace->getOffersListeningStatus();

    QVector<QString> offers = swapMarketplace->getTotalOffers();
    status += "   Offers: ";
    if (offers.isEmpty()) {
        status += "None";
    }
    else{
        for (int i=1; i<offers.size(); i+=2) {
            if (i>1)
                status += ", ";

            status += offers[i-1] + ": " + offers[i];
        }
    };

    ui->statusLabel->setText(status);
}

void MrktSwList::on_buySellCombo_currentIndexChanged(int index)
{
    if (!controlsReady)
        return;

    if (index>=0) {
        config->setSwapMktSelling(index);
        updateModeButtons(BTN_MKT_OFFERS);
        updateTradeListData(true);
    }
}

void MrktSwList::on_secCurrencyCombo_currentIndexChanged(int index)
{
    if (!controlsReady)
        return;

    if (index>=0) {
        config->setSwapMktCurrency( ui->secCurrencyCombo->currentData().toString() );
        updateTradeListData(true);
    }
}

void MrktSwList::on_filterFeeLevel_currentIndexChanged(int index)
{
    if (!controlsReady)
        return;

    if (index>=0) {
        double fee = ui->filterFeeLevel->currentData().toDouble();
        config->setSwapMktMinFeeLevel( fee );
        updateTradeListData(true);
    }
}

void MrktSwList::updateTradeListData(bool resetScrollValue) {
    ui->buyOffersTable->clearAll(resetScrollValue);
    ui->sellOffersTable->clearAll(resetScrollValue);

    if (selectedTab == BTN_FEES)
        return;

    bool hasTor = wallet->getTorListenerStatus();
    int lastNodeHeight = swapMarketplace->getLastNodeHeight();

    if (selectedTab == BTN_MY_OFFERS) {
        QVector<QString> offersStr = swapMarketplace->getMyOffers();
        for (auto &s : offersStr) {
            state::MySwapOffer offer(s);

            control::RichItem *itm = control::createMarkedItem(offer.offer.id, ui->buyOffersTable, true, offer.offer.sell ? "#ff0000" : "#009127" );
            itm->hbox().setContentsMargins(0, 0, 0, 0).setSpacing(4);

            // Line 1
            itm->addWidget(
                        control::createIcon(itm, offer.offer.sell ? ":/img/iconSent@2x.svg" : ":/img/iconReceived@2x.svg", control::ROW_HEIGHT, control::ROW_HEIGHT));
            itm->addWidget(control::createLabel(itm, false, false,
                                                    (offer.offer.sell ? "Sell " : "Buy ") + QString::number(offer.offer.mwcAmount) + " MWC for " +
                                                    QString::number(offer.offer.secAmount) + " " + offer.offer.secondaryCurrency +
                                                    " ; Price "+ offer.offer.calcRateAsStr() + " (" + offer.offer.secondaryCurrency + "); " + (hasTor ? offer.getStatusStr(lastNodeHeight) : "Waiting for TOR...")   ));
//            itm->setMinWidth(275);
//            itm->addWidget(control::createLabel(itm, false, true, "Auto renew: Yes"));
            itm->addHSpacer().pop();

            // Line 2
            {
                itm->addFixedVSpacer(control::VBOX_SPACING); // add extra spacing
                itm->hbox().setContentsMargins(0, 0, 0, 0);
                itm->addWidget(
                        control::createLabel(itm, false, true,
                                             "Conf:  MWC " + QString::number(offer.offer.mwcLockBlocks) +
                                             ", " + offer.offer.secondaryCurrency + " " +
                                             QString::number(offer.offer.secLockBlocks),
                                             control::FONT_SMALL)).setMinWidth(300);
                itm->addWidget(
                        control::createLabel(itm, false, true,
                                             "Lock Time:  MWC " + offer.offer.calcMwcLockTime() +
                                             ", " + offer.offer.secondaryCurrency + " " +
                                                     offer.offer.calcSecLockTime(),
                                             control::FONT_SMALL));
                itm->addHSpacer().pop();
            }
            {
                itm->hbox().setContentsMargins(0, 0, 0, 0);
                QString feeStr;
                if (offer.integrityFee.uuid.isEmpty()) {
                    feeStr += "Not Paid";
                } else {
                    QString expirationStr;
                    if (lastNodeHeight > 0 && offer.integrityFee.expiration_height > 0) {
                        if (offer.integrityFee.expiration_height > lastNodeHeight)
                            expirationStr = ", expired in " +
                                            QString::number(offer.integrityFee.expiration_height - lastNodeHeight) +
                                            " blocks";
                        else
                            expirationStr = ", expired";
                    }

                    feeStr += feeLevelValToStr(offer.integrityFee.toDblFee() / offer.offer.mwcAmount) + " (" + util->nano2one(offer.integrityFee.fee) + " MWC)" +
                              expirationStr;
                }
                itm->addWidget(
                        control::createLabel(itm, false, true,
                                             "Fee:  " + feeStr,
                                             control::FONT_SMALL));
                itm->addHSpacer().pop();
            }
            if (!offer.note.isEmpty()) {
                itm->addFixedVSpacer(control::VBOX_SPACING); // add extra spacing
                itm->hbox().setContentsMargins(0, 0, 0, 0);
                itm->addWidget(control::createLabel(itm, false, false, offer.note, control::FONT_NORMAL));
                itm->addHSpacer().pop();
            }

            // Buttons need to go in full size.
            // So we need to finish the main vertical layout
            itm->pop();
            itm->vbox().setContentsMargins(0, control::VBOX_MARGIN, 0, 0).setSpacing(control::VBOX_SPACING);

            itm->addWidget(
                    (new control::RichButton(itm, "Cancel", BTN_WIDTH, control::ROW_HEIGHT * 3 / 2,
                                             "Cancel this offer", BTN_FONT_SIZE))->
                            setCallback(this, "Cancel:" + offer.offer.id));

            itm->addWidget(
                    (new control::RichButton(itm, "Edit", BTN_WIDTH, control::ROW_HEIGHT * 3 / 2,
                                             "Update this offer", BTN_FONT_SIZE))->
                            setCallback(this, "Update:" + offer.offer.id));

            itm->addVSpacer();

            ui->buyOffersTable->addItem(itm);
        }
        // Done with "My Offers"
        ui->buyOffersTable->apply();

        return;
    }

    if (selectedTab == BTN_MKT_OFFERS) {
        int sellingFlag = config->getSwapMktSelling();
        QVector<QString> offersStr = swapMarketplace->getMarketOffers(config->getSwapMktMinFeeLevel(),
                                                                      sellingFlag,
                                                                      config->getSwapMktCurrency());
        QString myTorAddress = util::extractPubKeyFromAddress(wallet->getTorAddress());

        for (auto &s : offersStr) {
            state::MktSwapOffer offer(s);

            QString mktOfferId = offer.walletAddress + "," + offer.id;
            bool ownOffer = offer.walletAddress==myTorAddress;

            control::RichVBox * parent = nullptr;
            if (offer.sell) {
                parent = ui->sellOffersTable;
            }
            else {
                parent = ui->buyOffersTable;
            }
            Q_ASSERT(parent);
            control::RichItem * itm = control::createMarkedItem(mktOfferId, parent, ownOffer, "", 3, 10, 3, 10);

            itm->hbox().setContentsMargins(0, 0, 0, 0).setSpacing(4);

            // Line 1
            itm->addWidget(
                    control::createIcon(itm, offer.sell ? ":/img/iconSent@2x.svg" : ":/img/iconReceived@2x.svg", control::ROW_HEIGHT, control::ROW_HEIGHT));
            itm->addWidget(control::createLabel(itm, false, false,
                                                (offer.sell ? "Sell " : "Buy ") + QString::number(offer.mwcAmount) + " MWC for " +
                                                QString::number(offer.secAmount) + " " + offer.secondaryCurrency +
                                                " ; Price "+ offer.calcRateAsStr() + " (" + offer.secondaryCurrency + ")"));

            itm->addFixedHSpacer(20);

            if (sellingFlag!=2) {
                QString text;
                if (!ownOffer || offer.mktFee > 0) {
                    text = "Fee : " + feeLevelValToStr(offer.mktFee / offer.mwcAmount) +
                           " (" + QString::number(offer.mktFee) + " MWC)";
                }
                itm->addWidget(control::createLabel(itm, false, true,
                                                    text));
                itm->addFixedHSpacer(20);
            }

            itm->addWidget(
                    control::createLabel(itm, false, true,
                                         "Conf:  MWC " + QString::number(offer.mwcLockBlocks) +
                                         ", " + offer.secondaryCurrency + " " +
                                         QString::number(offer.secLockBlocks),
                                         control::FONT_SMALL));
            itm->addHSpacer();

            if (offer.walletAddress!=myTorAddress) {
                itm->addWidget(
                        (new control::RichButton(itm, "Details", BTN_WIDTH, control::ROW_HEIGHT * 3 / 2,
                                                 "View details and accept this offer", BTN_FONT_SIZE))->
                                setCallback(this, "Details:" + mktOfferId));
            }
            itm->pop();

            // Line 2
/*            {
                itm->addFixedVSpacer(control::VBOX_SPACING); // add extra spacing
                itm->hbox().setContentsMargins(0, 0, 0, 0);
                itm->addWidget(
                        control::createLabel(itm, false, true,
                                             "Conf:  MWC " + QString::number(offer.mwcLockBlocks) +
                                             ", " + offer.secondaryCurrency + " " +
                                             QString::number(offer.secLockBlocks),
                                             control::FONT_SMALL)).setMinWidth(350);
                itm->addHSpacer().pop();
            }
            {
                itm->hbox().setContentsMargins(0, 0, 0, 0);
                itm->addWidget(
                        control::createLabel(itm, false, true,
                                             "Lock Time:  MWC " + offer.calcMwcLockTime() +
                                             ", " + offer.secondaryCurrency + " " +
                                             offer.calcSecLockTime(),
                                             control::FONT_SMALL));
                itm->addHSpacer().pop();
            }
            {
                itm->addFixedVSpacer(control::VBOX_SPACING); // add extra spacing
                itm->hbox().setContentsMargins(0, 0, 0, 0);
                itm->addWidget(
                        control::createLabel(itm, false, true,
                                             "From:  " + offer.walletAddress,
                                             control::FONT_SMALL));
                itm->addHSpacer().pop();

            }

            // Don't add button for self offers
            if (offer.walletAddress!=myTorAddress) {
                itm->pop();
                itm->vbox().setContentsMargins(0, control::VBOX_MARGIN, 0, 0).setSpacing(control::VBOX_SPACING);

                itm->addWidget(
                        (new control::RichButton(itm, "Accept", BTN_WIDTH, control::ROW_HEIGHT * 3 / 2,
                                                 "Accept this offer", BTN_FONT_SIZE))->
                                setCallback(this, "Accept:" + mktOfferId));
                itm->addVSpacer();
            }*/

            parent->addItem(itm);
        }
        ui->buyOffersTable->apply();
        ui->sellOffersTable->apply();
    }
}

void MrktSwList::richButtonPressed(control::RichButton *button, QString cookie) {
    Q_UNUSED(button)
    if (cookie.startsWith("Cancel:")) {
        QString myMsgId = cookie.mid(qstrlen("Cancel:"));
        qDebug() << "Cancelling my offer message " << myMsgId;

        if (core::WndManager::RETURN_CODE::BTN2 ==
            control::MessageBox::questionText(this, "Cancel Swap Offer", "Are you sure you want to cancel this offer?",
                                              "No", "Yes",
                                              "I want to keep publishing this offer", "Cancel this offer")) {
            swapMarketplace->withdrawMyOffer(myMsgId);
        }
    } else if (cookie.startsWith("Update:")) {
        QString myMsgId = cookie.mid(qstrlen("Update:"));
        qDebug() << "Updating my offer message " << myMsgId;
        swapMarketplace->pageCreateNewOffer(myMsgId);
    } else if (cookie.startsWith("Accept:")) {
        // Accept offer need to be implemented
        QString msgId = cookie.mid(qstrlen("Accept:"));
        auto ids = msgId.split(",");
        Q_ASSERT(ids.length() == 2);
        QString torAddress = ids[0];
        QString offerId = ids[1];
        qDebug() << "Accepting the offer from " << torAddress << " with id " << offerId;
        if (swapMarketplace->acceptMarketplaceOffer(offerId, torAddress))
            ui->progress->show();
    } else if (cookie.startsWith("Details:")) {
        util::TimeoutLockObject to("MktOfferDetails");

        QString msgId = cookie.mid(qstrlen("Details:"));
        auto ids = msgId.split(",");
        Q_ASSERT(ids.length() == 2);
        QString torAddress = ids[0];
        QString offerId = ids[1];

        QString offerJson = swapMarketplace->getMarketOffer(offerId, torAddress);
        Q_ASSERT(!offerJson.isEmpty());
        if (!offerJson.isEmpty()) {
            dlg::MktOfferDetailsDlg dlg(this, state::MktSwapOffer(offerJson) );
            if ( dlg.exec()  == QDialog::Accepted ) {
                qDebug() << "Accepting the offer from " << torAddress << " with id " << offerId;
                if (swapMarketplace->acceptMarketplaceOffer(offerId, torAddress))
                    ui->progress->show();
            }
        }
    }
}

void MrktSwList::onBuyItemActivated(QString id) {
    if (selectedTab == BTN_MY_OFFERS) {
        swapMarketplace->pageCreateNewOffer(id);
        return;
    }
    if (selectedTab == BTN_MKT_OFFERS) {
        Q_ASSERT(false); // fix me
    }

}

void MrktSwList::onSellItemActivated(QString id) {
    if (selectedTab == BTN_MKT_OFFERS) {
        Q_ASSERT(false); // fix me
    }
}


void MrktSwList::on_marketOffers_clicked() {
    updateModeButtons(BTN_MKT_OFFERS);
}

void MrktSwList::on_myOffers_clicked() {
    updateModeButtons(BTN_MY_OFFERS);
}

void MrktSwList::on_feesBtn_clicked() {
    updateModeButtons(BTN_FEES);
    swapMarketplace->requestIntegrityFees();
    ui->activeFees->setText("Requesting integrity fee status...");
}

const double MIN_RESERVE_FEE = 1.0;

void MrktSwList::pushIntegritySettings() {
    bool ok = false;
    double mwcAmount = ui->mwcReservedFee->text().toDouble(&ok);
    int idx = ui->feeLevel->currentIndex();
    if (ok && idx >= 0 && idx < MKT_FEES.length() && mwcAmount>=MIN_RESERVE_FEE ) {
        swapMarketplace->setIntegritySettings(MKT_FEES[idx].fee, ui->depositAccountName->currentText(), mwcAmount);
    }
}


void MrktSwList::updateMwcReservedFeeValidity() {
    bool ok = false;
    double mwcAmount = ui->mwcReservedFee->text().toDouble(&ok);

    bool valid = ok && mwcAmount >= MIN_RESERVE_FEE;
    if (valid)
        ui->mwcReservedFee->setStyleSheet("color: white; font-weight: normal");
    else
        ui->mwcReservedFee->setStyleSheet("color: #FFBB2E; font-weight: bold");
}


void MrktSwList::on_feeLevel_currentIndexChanged(int index) {
    Q_UNUSED(index);
    pushIntegritySettings();
}

void MrktSwList::on_depositAccountName_currentIndexChanged(int index) {
    Q_UNUSED(index);
    pushIntegritySettings();
}

void MrktSwList::on_mwcReservedFee_textEdited(const QString &arg1) {
    Q_UNUSED(arg1);
    updateMwcReservedFeeValidity();
    pushIntegritySettings();
}

void MrktSwList::on_withdrawFeesBtn_clicked() {
    QString account = swapMarketplace->getFeeDepositAccount();
    if (core::WndManager::RETURN_CODE::BTN1 == control::MessageBox::questionText(this, "Waring",
                                                                                 "We recommend to pay integrity fee from a separate amount because they are used for proofs and leak privacy.\n\n"
                                                                                 "Please withdraw the reserved balance if you are not planning to post atomic swap offers for a long time.\n\n"
                                                                                 "Continue and move reserved coins to account '" +
                                                                                 account + "'?",
                                                                                 "Yes", "No",
                                                                                 "move reserved coins to the account",
                                                                                 "Keep reserved balance to pay swap marketplace fees")) {
        ui->progress->show();
        swapMarketplace->withdrawIntegrityFees();
        ui->withdrawFeesBtn->setText("Withdraw Fees");
        ui->withdrawFeesBtn->setEnabled(false);
    }
}

void MrktSwList::on_feeLevelHelpBtn_clicked() {
    control::MessageBox::messageText(this, "Help",
                                     "Specify the fees for your swap offers.\n\nLow: 1 bps or 0.01% of the trade amount.\n"
                                     "Normal: 10 bps or 0.1% of the trade amount.\n"
                                     "High: 50 bps or 0.5% of the trade amount.\n\n"
                                     "The fee level should depend on spamming activity. It make sense to specify the fees to be at higher level than spammers.");
}

void MrktSwList::on_depositAccountHelpBtn_clicked() {
    control::MessageBox::messageText(this, "Help", "Account that will be used as a source of funds to pay integrity fees.");
}

void MrktSwList::on_reservedAmountHelpBtn_clicked() {
    control::MessageBox::messageText(this, "Help",
                                     "To mitigate integrity fee transaction privacy leaks, your mwc wallet separate some coins and use them to pay integrity fees.");
}

void MrktSwList::on_activeFeesHelpBtn_clicked() {
    control::MessageBox::messageText(this, "Help",
                                     "Here you can see available reserved coins and valid paid fees that can be used to publish swap offers.");
}

void MrktSwList::on_withdrawHelpBtn_clicked() {
    control::MessageBox::messageText(this, "Help",
                                     "If you are not planning to use atomic swap marketplace for a long time, you can move reserved funds back to your regular account.");
}

void MrktSwList::on_newOfferButton_clicked() {
    if (!wallet->getTorListenerStatus()) {
        control::MessageBox::messageText(this, "TOR is offline", "Atomic swap marketplace requires connection to the TOR network. Please tart TOR listener for the wallet.");
        return;
    }

    swapMarketplace->pageCreateNewOffer("");
}

/*void MrktSwList::on_offersListenSettingsBtn_clicked() {
    util::TimeoutLockObject to("SwapBackupDlg");

    dlg::MktShowParamsDlg params(this, config->getSwapMktMinFeeLevel(), config->getSwapMktSelling(),
                                 config->getSwapMktCurrency(), config->getSwapMktMinMwcAmount(),
                                 config->getSwapMktMaxMwcAmount());
    if (params.exec() == QDialog::Accepted) {
        double feeLevel = 0.0;
        bool selling = true;
        QString secondaryCurrency;
        double minMwcAmount = 0.0;
        double maxMwcAmount = 0.0;
        params.getData(feeLevel, selling, secondaryCurrency, minMwcAmount, maxMwcAmount);

        config->setSwapMktFilter(feeLevel, selling, secondaryCurrency, minMwcAmount, maxMwcAmount);
        updateMktFilter();
        updateTradeListData(true);
    }
}*/

void MrktSwList::sgnMarketPlaceOffersChanged() {
    ui->progress->hide();
    if (selectedTab == BTN_MKT_OFFERS)
        updateTradeListData(false);
}

void MrktSwList::sgnMyOffersChanged() {
    ui->progress->hide();
    if (selectedTab == BTN_MY_OFFERS)
        updateTradeListData(false);
}

void MrktSwList::sgnMessagingStatusChanged() {
    updateMktFilter();
}

void MrktSwList::updateList() {
    updateTradeListData(false);
}

void MrktSwList::sgnRequestIntegrityFees(QString error, int64_t balance, QVector<QString> integrityFeesJsonStr) {
    if (error.isEmpty()) {
        // get the data
        QString report = "Reserved " + util->nano2one(balance) + " MWC\n";
        if (integrityFeesJsonStr.isEmpty()) {
            report += "No paid integrity fees are available";
        }
        else {
            report += "Paid active fees:\n";
            for (auto & s : integrityFeesJsonStr) {
                // parsing Json to the fees
                wallet::IntegrityFees feeInfo(s);
                int lastNodeHeight = swapMarketplace->getLastNodeHeight();

                if (lastNodeHeight>0 && lastNodeHeight >= feeInfo.expiration_height)
                    continue; // expired fee, don't show

                report += "   " + util->nano2one(feeInfo.fee) + " MWC, " + (feeInfo.confirmed ? "Confirmed" : "Not confirmed");
                if (lastNodeHeight>0 && lastNodeHeight < feeInfo.expiration_height) {
                    report += ", expired after " + QString::number(feeInfo.expiration_height - lastNodeHeight) + " blocks";
                }
                report += "\n";
            }
        }
        ui->activeFees->setText(report);

        if (balance>0) {
            ui->withdrawFeesBtn->setText("Withdraw " + util->nano2one(balance) + " MWC");
            ui->withdrawFeesBtn->setEnabled(true);
        }
        else {
            ui->withdrawFeesBtn->setText("Withdraw Fees");
            ui->withdrawFeesBtn->setEnabled(false);
        }
    }
    else {
        ui->activeFees->setText("Unable to request integrity fee status:\n+" + error);
        ui->withdrawFeesBtn->setText("Withdraw Fees");
        ui->withdrawFeesBtn->setEnabled(false);
    }
}

void MrktSwList::sgnWithdrawIntegrityFees(QString error, double mwc, QString account) {
    Q_UNUSED(mwc)
    Q_UNUSED(account)

    ui->progress->hide();
    if (!error.isEmpty()) {
        control::MessageBox::messageText(this, "Withdraw error", error);
    }
    swapMarketplace->requestIntegrityFees();
}

void MrktSwList::on_viewIntegrityTransactionsBtn_clicked() {
    swapMarketplace->pageTransactionFee();
}


}
