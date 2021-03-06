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
#include "../core/Notification.h"
#include "../wallet/mwc713.h"

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

protected:
    virtual NextStateRespond execute() override;
    virtual bool mobileBack() override {return false;}
    virtual QString getHelpDocName() override {return "listener.html";}

private slots:
    // Listening, you will not be able to get a results
    void onListeningStartResults( bool mqTry, bool torTry, // what we try to start
                                   QStringList errorMessages, bool initialStart ); // error messages, if get some

    void onListeningStopResult(bool mqTry, bool torTry, // what we try to stop
                                QStringList errorMessages );

    // Login from anotehr wallet happens
    void onListenerMqCollision();


    // Looking for "Failed to start mwcmqs subscriber. Error connecting to ..."
    void onNewNotificationMessage(notify::MESSAGE_LEVEL level, QString message);

private:
    QString lastShownErrorMessage;
};

}

#endif // LISTENING_H
