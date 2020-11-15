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

#ifndef INPUTPASSWORD_H
#define INPUTPASSWORD_H

#include "state.h"
#include <QObject>
#include "../wallet/wallet.h"

namespace state {

class InputPassword : public QObject, public State
{
    Q_OBJECT
public:
    InputPassword(StateContext * context);
    virtual ~InputPassword() override;

    // Async call to submit the password. This state migth get back to UI if password is incorrect
    void submitPassword(const QString & password);

protected:
    virtual NextStateRespond execute() override;

protected slots:
    // Result of the login
    void onLoginResult(bool ok);

    void onWalletBalanceUpdated();
private:
    bool inLockMode = false;
    QString lockedWalletPath;
};

}

#endif // INPUTPASSWORD_H
