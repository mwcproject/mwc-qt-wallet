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

#ifndef SELECTMODE_H
#define SELECTMODE_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../core/Config.h"

namespace state {

class SelectMode : public State
{
public:
    SelectMode( StateContext * context);
    virtual ~SelectMode() override;

    static config::WALLET_RUN_MODE getWalletRunMode();

    // Will require restart
    // Note, it is used by many parties as a helper method that apply a new mode and restart the wallet
    static void updateWalletRunMode( config::WALLET_RUN_MODE newRunMode );

protected:
    virtual NextStateRespond execute() override;
    virtual QString getHelpDocName() override {return "select_mode.html";}
};

}


#endif // SELECTMODE_H
