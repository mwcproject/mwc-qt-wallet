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

#include "c_enterseed.h"
#include "ui_c_enterseed.h"
#include "../util/stringutils.h"
#include "../control/messagebox.h"
#include "../state/a_initaccount.h"
#include "../util/widgetutils.h"
#include "../util/Dictionary.h"
#include "../state/timeoutlock.h"

namespace wnd {

EnterSeed::EnterSeed(QWidget *parent, state::InitAccount * _state) :
    QWidget(parent),
    ui(new Ui::EnterSeed),
    state(_state)
{
    //state->setWindowTitle("Recover wallet from a Passphrase");

    ui->setupUi(this);

    ui->progress->initLoader(false);

    ui->seedText->setReadOnly(false);
    ui->seedText->setAcceptRichText(false);
    ui->seedText->setFocus(Qt::OtherFocusReason);

    utils::defineDefaultButtonSlot(this, SLOT(on_Enter()) );
}

EnterSeed::~EnterSeed()
{
    state->deleteEnterSeed(this);
    delete ui;
}

void EnterSeed::on_Enter() {
    if ( ui->seedText->hasFocus() )
        return;
    on_continueButton_clicked();
}

void EnterSeed::updateProgress(bool show) {
    if (show)
        ui->progress->show();
    else
        ui->progress->hide();
}


void EnterSeed::on_cancelButton_clicked()
{
    state->cancel();
}

void EnterSeed::on_continueButton_clicked()
{
    state::TimeoutLockObject to(state);

    QString seedStr = ui->seedText->toPlainText().toLower().trimmed();

    // No need to validate symbols because we better validation is down
    /*QPair <bool, QString> valRes = util::validateMwc713Str( seedStr );
    if (!valRes.first) {
        control::MessageBox::message(this, "Verification error", valRes.second );
        return;
    }*/

    QVector<QString> seed = util::parsePhrase2Words( seedStr );

    if (seed.size()!=24) {
        control::MessageBox::message(this, "Verification error",
                             "Your phrase should contain 24 words. You entered " + QString::number(seed.size()) +  " words." );
        return;
    }

    const QSet<QString> & words = util::getBip39words();
    QString nonDictWord;

    for ( auto & s : seed ) {
        if ( !words.contains(s) ) {
            if (!nonDictWord.isEmpty())
                nonDictWord += ", ";
            nonDictWord += s;
        }
    }

    if (!nonDictWord.isEmpty()) {
        control::MessageBox::message(this, "Verification error",
                                     "Your phrase contains non dictionary words: " + nonDictWord );
        return;
    }

    state->createWalletWithSeed(seed);
}

}
