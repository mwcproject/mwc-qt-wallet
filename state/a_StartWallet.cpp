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

#include "a_StartWallet.h"
#include "../core/appcontext.h"
#include "../core/windowmanager.h"
#include "../state/statemachine.h"
#include "../windows/a_waitingwnd.h"
#include "../util/Log.h"

namespace state {

// Init the wallet. Then check how it is started. If it needs to have password or something
StartWallet::StartWallet(StateContext * context) :
        State(context, STATE::START_WALLET)
{
}

StartWallet::~StartWallet() {
}

NextStateRespond StartWallet::execute() {
    if ( !context->wallet->isRunning() ) {
        // Just update the wallet with a status. Then continue
        context->appContext->pushCookie<QString>("checkWalletInitialized",
                context->wallet->checkWalletInitialized() ? "OK" : "FAILED" );
    }

    return NextStateRespond(NextStateRespond::RESULT::DONE);
}




}
