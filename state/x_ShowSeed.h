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

#ifndef MWC_QT_WALLET_SHOWSEED_H
#define MWC_QT_WALLET_SHOWSEED_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../state/a_initaccount.h"

namespace wnd {
class NewSeed;
}

namespace state {

class ShowSeed : public QObject, public State, SubmitCaller {
    Q_OBJECT
public:
    ShowSeed( StateContext * context);
    virtual ~ShowSeed() override;

    void deleteWnd() {wnd= nullptr;}

protected:
    virtual NextStateRespond execute() override;
    virtual QString getHelpDocName() override {return "seed.html";}

    // submit is hidden, never expected to be called.
    virtual void submit() override {Q_ASSERT(false);}
    virtual void wndDeleted(wnd::NewSeed * w) override { if (w==wnd) wnd=nullptr; }

private slots:
    void recoverPassphrase( QVector<QString> seed );

private:
    wnd::NewSeed * wnd = nullptr; // not own, just a pointer
};



}

#endif //MWC_QT_WALLET_SHOWSEED_H
