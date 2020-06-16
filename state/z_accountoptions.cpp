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

#include "state/z_accountoptions.h"
#include "../core/appcontext.h"
#include "../core/WndManager.h"

namespace state {

AccountOptions::AccountOptions( StateContext * _context) :
    State(_context, STATE::ACCOUNT_OPTIONS)
{
}

AccountOptions::~AccountOptions() {
}

NextStateRespond AccountOptions::execute() {
    if (context->appContext->getActiveWndState() != STATE::ACCOUNT_OPTIONS)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    core::getWndManager()->pageAccountOptions();   // you need to add your page at WndManager
    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

}
