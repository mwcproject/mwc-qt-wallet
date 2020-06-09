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
#include "../control_desktop/messagebox.h"
#include "../util_desktop/widgetutils.h"
#include "../util_desktop/timeoutlock.h"
#include "../dialogs_desktop/c_addbip39word.h"
#include "../bridge/util_b.h"
#include "../bridge/wnd/a_initaccount_b.h"

namespace wnd {

EnterSeed::EnterSeed(QWidget *parent) :
    core::PanelBaseWnd(parent),
    ui(new Ui::EnterSeed)
{
    ui->setupUi(this);

    accountInit = new bridge::InitAccount(this);
    util = new bridge::Util(this);

    ui->progress->initLoader(false);

    ui->seedText->setReadOnly(false);
    ui->seedText->setAcceptRichText(false);
    ui->seedText->setFocus(Qt::OtherFocusReason);

    utils::defineDefaultButtonSlot(this, SLOT(on_Enter()) );
}

EnterSeed::~EnterSeed()
{
    delete ui;
}

void EnterSeed::on_Enter() {
    if ( ui->seedText->hasFocus() )
        return;

    QString seedStr = ui->seedText->toPlainText().toLower().trimmed();
    QVector<QString> seed = util->parsePhrase2Words( seedStr );

    if (seed.size()>=24) {
        on_continueButton_clicked();
    }
    else {
        on_addWordButton_clicked();
    }
}

void EnterSeed::on_cancelButton_clicked()
{
    accountInit->cancelInitAccount();
}

void EnterSeed::on_continueButton_clicked()
{
    util::TimeoutLockObject to("EnterSeed");

    QString seedStr = ui->seedText->toPlainText().toLower().trimmed();

    QVector<QString> seed = util->parsePhrase2Words( seedStr );

    if (seed.size()!=24) {
        control::MessageBox::messageText(this, "Verification error",
                             "Your phrase should contain 24 words. You entered " + QString::number(seed.size()) +  " words." );
        return;
    }

    QVector<QString> words = util->getBip39words();
    QString nonDictWord;

    for ( auto & s : seed ) {
        if ( !words.contains(s) ) {
            if (!nonDictWord.isEmpty())
                nonDictWord += ", ";
            nonDictWord += s;
        }
    }

    if (!nonDictWord.isEmpty()) {
        control::MessageBox::messageText(this, "Verification error",
                                     "Your phrase contains non dictionary words: " + nonDictWord );
        return;
    }

    accountInit->createWalletWithSeed(seed);
}

void wnd::EnterSeed::on_addWordButton_clicked()
{
    util::TimeoutLockObject to("EnterSeed");

    QString seedStrOrig = ui->seedText->toPlainText();
    QString seedStr = seedStrOrig.toLower().trimmed();

    QVector<QString> seed = util->parsePhrase2Words( seedStr );

    dlg::AddBip39Word addWordDlg(this, seed.length()+1 );

    if (addWordDlg.exec() == QDialog::Accepted) {
        if (seedStrOrig.length()>0 && seedStrOrig[seedStrOrig.length()-1].isLetterOrNumber())
            seedStrOrig += " ";
        seedStrOrig += addWordDlg.getResultWord();
        ui->seedText->setText(seedStrOrig);
    }


}

}

