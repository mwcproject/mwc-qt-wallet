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
#include "../state/a_initaccount.h"
#include "../util/widgetutils.h"
#include "../control/messagebox.h"
#include "../state/timeoutlock.h"
#include "../util/stringutils.h"

namespace wnd {


NewSeed::NewSeed( QWidget *parent, state::SubmitCaller * _state, state::StateContext * _context,
                  const QVector<QString> & _seed, bool hideSubmitButton ) :
    core::NavWnd( parent, _context, hideSubmitButton ),
    ui(new Ui::NewSeed),
    state(_state),
    context(_context),
    seed(_seed)
{
    ui->setupUi(this);

    if ( hideSubmitButton )
        ui->submitButton->hide();
    else
        utils::defineDefaultButtonSlot(this, SLOT(on_submitButton_clicked()) );

    if (!seed.isEmpty()) {
        updateSeedData("Mnemonic passphrase:",seed); // Seed allways comes from ctor. Mean we are created a new account with a seed.
    }

}

NewSeed::~NewSeed()
{
    state->wndDeleted(this);
    delete ui;
}

// if seed empty or has size 1, it is error message
void NewSeed::showSeedData(const QVector<QString> & seed) {
    state::TimeoutLockObject to( context->stateMachine );

    if (seed.size()<2) {
        control::MessageBox::messageText( this, "Getting Passphrase Failure", "Unable to retrieve a passphrase from mwc713. " + (seed.size()>0 ? seed[0] : "") );
        return;
    }
    updateSeedData("Mnemonic passphrase:", seed);
}

void NewSeed::updateSeedData( const QString & name, const QVector<QString> & seed) {
    int maxWrdLen = 0;
    for (const auto & s : seed)
        maxWrdLen = std::max(maxWrdLen, s.length());

    maxWrdLen += 3;

    QString thePhrase = "";

    for (int i=0;i<seed.size();i++) {
        thePhrase += util::expandStrR(seed[i], maxWrdLen);
        if (i % 6==5)
            thePhrase += "\n";
    }

    ui->seedText->setPlainText( name + "\n" + thePhrase);
    ui->seedText->setFocus();
}

void NewSeed::on_submitButton_clicked()
{
    state->submit();
}

}
