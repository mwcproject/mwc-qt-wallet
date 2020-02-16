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

#ifndef HODL_H
#define HODL_H


#include "state.h"
#include "../wallet/wallet.h"
#include <QVector>

namespace state {

class Hodl : public State
{
public:
    Hodl(StateContext * context);
    virtual ~Hodl() override;

    QVector<wallet::WalletTransaction> getTransactions();

    void submitForHodl( const QVector<QString> & transactions );

protected:
    virtual NextStateRespond execute() override;
    virtual QString getHelpDocName() override {return "hodl.html";}
};

}

#endif // HODL_H
