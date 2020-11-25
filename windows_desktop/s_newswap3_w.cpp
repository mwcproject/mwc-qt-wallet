// Copyright 2020 The MWC Developers
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

#include "s_newswap3_w.h"
#include "ui_s_newswap3_w.h"
#include "../bridge/swap_b.h"
#include "../bridge/util_b.h"
#include "../control_desktop/messagebox.h"

namespace wnd {

NewSwap3::NewSwap3(QWidget *parent) :
        core::NavWnd(parent),
        ui(new Ui::NewSwap3) {
    ui->setupUi(this);
    swap = new bridge::Swap(this);
    util = new bridge::Util(this);

    connect(swap, &bridge::Swap::sgnApplyNewTrade2Params, this, &NewSwap3::onSgnCreateStartSwap,
            Qt::QueuedConnection);

    ui->progress->initLoader(false);

    updateSwapReview();
}

NewSwap3::~NewSwap3() {
    delete ui;
}

void NewSwap3::on_backButton_clicked() {
    swap->showNewTrade2();
}

void NewSwap3::on_sendOfferButton_clicked() {
    ui->progress->show();
    swap->createStartSwap();
}

void NewSwap3::onSgnCreateStartSwap(bool ok, QString errorMessage) {
    ui->progress->hide();

    if (ok)
        return;

    Q_ASSERT(!errorMessage.isEmpty());
    control::MessageBox::messageText(this, "Swap Creating Error", errorMessage);
}

static QString findInterval( const QVector<QString> & intervals, int interval ) {
    Q_ASSERT( intervals.size()%2 == 0 );

    QString intStr = QString::number(interval);

    for (int k=1; k<intervals.size(); k+=2) {
        if ( intervals[k] == intStr )
            return intervals[k-1];
    }
    Q_ASSERT(false);
    return intervals[0];
}

void NewSwap3::updateSwapReview() {
    QString secCur = swap->getCurrentSecCurrency();
    int mwcBlocks = swap->getMwcConfNumber();
    int secBlocks = swap->getSecConfNumber();
    int offerExpiration = swap->getOfferExpirationInterval();
    int redeemTime = swap->getSecRedeemTime();
    QVector<QString> lockTime = swap->getLockTime( secCur, offerExpiration, redeemTime, mwcBlocks, secBlocks );

    // <Interval is string> <Value in minutes>
    QVector<QString> expIntevals = swap->getExpirationIntervals();


    QString reportStr = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
                        "<html><body style=\"font-family:'Open Sans'; font-size:medium; font-weight:normal; font-style:normal; color:white; background-color:transparent;\">";

    reportStr += "<p>";
    reportStr += "Selling <b style=\"color:yellow;\">" + swap->getMwc2Trade() + " MWC</b> for <b style=\"color:yellow;\">" + swap->getSec2Trade() + " " + secCur + "</b>.</p>";

    reportStr += "<p>";
    reportStr += "Required lock confirmations: <b style=\"color:yellow;\">" + QString::number(mwcBlocks) +
            " for MWC</b> and <b style=\"color:yellow;\">" + QString::number(secBlocks) + " for " + secCur + "</b>.</p>";


    reportStr += "<p>";
    reportStr += "Time limits:<br>";
    reportStr += "&emsp; &#9702; Offer expiration time is <b style=\"color:yellow;\">" + findInterval(expIntevals, offerExpiration) + "</b>.<br>";
    reportStr += "&emsp; &#9702; "+secCur+" redeem time limit is <b style=\"color:yellow;\">" + findInterval(expIntevals, redeemTime) + "</b>.</p>";

    reportStr += "<p>";
    reportStr += "Coins lock order: <b style=\"color:yellow;\">" + (swap->isLockMwcFirst() ? QString("I am locking MWC first") : (QString("Another party locking ") + secCur + " first")) + "</b>.</p>";

    reportStr += "<p>";
    reportStr += "In case the trade is cancelled, the coins will be locked:<br>";
    reportStr += "&emsp; &#9702; My <b style=\"color:yellow;\">MWC</b> will be locked for <b style=\"color:yellow;\">" + lockTime[0] + "</b>.<br>";
    reportStr += "&emsp; &#9702; Other party's <b style=\"color:yellow;\">"+secCur+"</b> will be locked for <b style=\"color:yellow;\">" + lockTime[1] + "</b>.</p>";

    reportStr += "<p>";
    reportStr += secCur + " redeem address: <b style=\"color:yellow;\">" + swap->getSecAddress() + "</b></p>";
    reportStr += "<p>";
    reportStr += "My "+secCur+" redeem transaction fee is <b style=\"color:yellow;\">" + util->trimStrAsDouble( QString::number(swap->getSecTransactionFee(), 'f'), 10) + " " + swap->getCurrentSecCurrencyFeeUnits() + "</b></p>";
    reportStr += "<p>";
    reportStr += "Trading with : <b style=\"color:yellow;\">" + swap->getBuyerAddress() + "</b></p>";

    reportStr += "</body></html>";

    ui->offerDetailsEdit->setHtml(reportStr);

    ui->noteEdit->setText( swap->getNote() );
}

void NewSwap3::on_noteEdit_textEdited(const QString &text)
{
    swap->setNote(text);
}

}
