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

#include "x_ShowSeed.h"
#include "../windows/c_newseed_w.h"
#include "../core/windowmanager.h"
#include "../core/global.h"

namespace state {

ShowSeed::ShowSeed( StateContext * context) : State(context,  STATE::SHOW_SEED )
{
    QObject::connect(context->wallet, &wallet::Wallet::onGetSeed,
                                   this, &ShowSeed::recoverPassphrase, Qt::QueuedConnection);
}

ShowSeed::~ShowSeed() {}

NextStateRespond ShowSeed::execute() {
    if ( context->appContext->getActiveWndState() != STATE::SHOW_SEED )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    if (wnd==nullptr) {
        QString walletPassword = context->appContext->pullCookie<QString>("password");

        wnd = (wnd::NewSeed*) context->wndManager->switchToWindowEx( mwc::PAGE_X_SHOW_PASSPHRASE,
                new wnd::NewSeed( context->wndManager->getInWndParent(), this, context, QVector<QString>(), true ) );
        context->wallet->getSeed(walletPassword);
    }
    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

void ShowSeed::recoverPassphrase( QVector<QString> seed ) {
    if (wnd) {
        wnd->showSeedData(seed);
    }
}


}
