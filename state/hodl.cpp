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

#include "hodl.h"
#include "../wallet/wallet.h"
#include "../windows/hodl_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../core/global.h"

namespace state {

Hodl::Hodl(StateContext * context) :
        State(context, STATE::HODL)
{
}

Hodl::~Hodl() {}

NextStateRespond Hodl::execute() {
    if (context->appContext->getActiveWndState() != STATE::HODL)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    context->wndManager->switchToWindowEx( mwc::PAGE_HODL,
                new wnd::Hodl( context->wndManager->getInWndParent(), this ) );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

QVector<wallet::WalletTransaction> Hodl::getTransactions() {
    return QVector<wallet::WalletTransaction>();
    //return context->wallet->getTransactions();
}

void Hodl::submitForHodl( const QVector<QString> & transactions ) {
    // starting  a int64_t proces of submitting transactions that we will define later.
    Q_UNUSED(transactions);
}



}
