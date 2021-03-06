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

#ifndef GUI_WALLET_STARTWALLET_H
#define GUI_WALLET_STARTWALLET_H

#include "state.h"
#include "../wallet/wallet.h"


namespace state {

// Init the wallet. Then check how it is started. If it needs to have password or something
class StartWallet : public State {
public:
    StartWallet(StateContext * context);
    virtual ~StartWallet() override;

    void createNewWalletInstance(QString path, bool restoreWallet);

    void cancel();
protected:
    virtual NextStateRespond execute() override;
    virtual bool mobileBack() override {return false;}

private:
};

}

#endif //GUI_WALLET_STARTWALLET_H
