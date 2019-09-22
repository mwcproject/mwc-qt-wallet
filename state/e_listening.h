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

#ifndef LISTENING_H
#define LISTENING_H

#include "state.h"
#include "../wallet/wallet.h"

namespace wnd {
    class Listening;
}

namespace state {

class Listening : public QObject, public State
{
    Q_OBJECT
public:
    Listening( StateContext * context);
    virtual ~Listening() override;

    void wndIsGone(wnd::Listening * w) { if(w==wnd) wnd = nullptr;} // window is closed


    void triggerMwcStartState();

    void requestNextMwcMqAddress();

    void requestNextMwcMqAddressForIndex(int idx);

    void triggerKeybaseStartState();

protected:
    virtual NextStateRespond execute() override;
    virtual QString getHelpDocName() override {return "listener.html";}

private slots:
    void onMwcMqListenerStatus(bool online);
    void onKeybaseListenerStatus(bool online);

    // Listening, you will not be able to get a results
    void onListeningStartResults( bool mqTry, bool kbTry, // what we try to start
                                   QStringList errorMessages, bool initialStart ); // error messages, if get some

    void onListeningStopResult(bool mqTry, bool kbTry, // what we try to stop
                                QStringList errorMessages );

    void onMwcAddressWithIndex(QString mwcAddress, int idx);

    // Login from anotehr wallet happens
    void onListenerMqCollision();

    // Looking for "Failed to start mwcmqs subscriber. Error connecting to ..."
    void onNewNotificationMessage(wallet::WalletNotificationMessages::LEVEL level, QString message);

private:
    wnd::Listening * wnd = nullptr;
};

}

#endif // LISTENING_H
