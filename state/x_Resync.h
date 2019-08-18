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

#ifndef MWC_QT_WALLET_RESYNC_H
#define MWC_QT_WALLET_RESYNC_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../windows/z_progresswnd.h"

namespace state {

// Do resync for account. Expected that caller want us to start 'check' process for mwc713
class Resync : public QObject, public State, public wnd::IProgressWndState {
    Q_OBJECT
public:
    Resync( StateContext * context);
    virtual ~Resync() override;


    virtual void cancelProgress() override {Q_ASSERT(false);} // progress cancel not expected
    virtual void destroyProgressWnd(wnd::ProgressWnd * w) override { if (w==wnd) wnd = nullptr;}
protected:
    virtual NextStateRespond execute() override;

private slots:
    void onRecoverProgress( int progress, int maxVal );
    void onCheckResult(bool ok, QString errors );

private:
    wnd::ProgressWnd * wnd = nullptr;
    int prevState = STATE::TRANSACTIONS;
    QPair<bool,bool> prevListeningStatus = QPair<bool,bool>(false,false);
    int maxProgrVal = 100;
    // Need to recalibrate ther progress..
    // First few respond might be too fast
    int respondCounter = 0;
    int respondZeroLevel = 0;
    int progressBase = 0;
};

}

#endif //MWC_QT_WALLET_RESYNC_H
