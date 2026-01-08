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

#include "c_newseed_w.h"
#include "ui_c_newseed.h"
#include "../util_desktop/widgetutils.h"
#include "../control_desktop/messagebox.h"
#include "../util_desktop/timeoutlock.h"
#include "../bridge/wnd/c_newseed_b.h"

namespace wnd {


NewSeed::NewSeed( QWidget *parent,
                  const QVector<QString> & _seed, bool hideSubmitButton ) :
    core::NavWnd( parent,  hideSubmitButton ),
    ui(new Ui::NewSeed),
    seed(_seed)
{
    ui->setupUi(this);

    newSeed = new bridge::NewSeed(this);

    ui->progress->initLoader(false);

    if ( hideSubmitButton ) {
        ui->submitButton->hide();
        ui->progress->show();
    }
    else {
        utils::defineDefaultButtonSlot(this, SLOT(on_submitButton_clicked()) );
    }

    if (!seed.isEmpty()) {
        updateSeedData("Mnemonic passphrase:",seed); // Seed allways comes from ctor. Mean we are created a new account with a seed.
    }
}

void NewSeed::panelWndStarted() {
    ui->seedText->setFocus();
}


NewSeed::~NewSeed()
{
    delete ui;
}

void NewSeed::updateSeedData( const QString & name, const QVector<QString> & seed) {
    ui->progress->hide();

    int maxWrdLen = 0;
    for (const auto & s : seed)
        maxWrdLen = std::max(size_t(maxWrdLen), size_t(s.length()));

    maxWrdLen += 3;

    QString thePhrase = "";

    int rowLen = 5;

    switch (seed.size()) {
        case 12:
            rowLen = 4;
            break;
        case 15:
        case 20:
            rowLen = 5;
            break;
        case 21:
            rowLen = 7;
            break;
        default:
            rowLen = 6;
            break;
    }

    for (int i=0;i<seed.size();i++) {
        thePhrase += util::expandStrR(seed[i], maxWrdLen);
        if (i % rowLen==rowLen-1)
            thePhrase += "\n";
    }

    ui->seedText->setPlainText( name + "\n" + thePhrase);
    ui->seedText->setFocus();
}

void NewSeed::on_submitButton_clicked()
{
    newSeed->doneWithNewSeed();
}

}
