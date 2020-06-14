// Copyright 2020 The MWC Developers
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

#ifndef MWC_QT_WALLET_STATEMACHINE_B_H
#define MWC_QT_WALLET_STATEMACHINE_B_H

#include <QObject>

namespace bridge {

class StateMachine : public QObject {
Q_OBJECT
public:
    StateMachine(QObject * parent = nullptr);
    ~StateMachine();

    // Switch to another page
    // actionWindowState:  state::STATE
    Q_INVOKABLE bool setActionWindow( int stateId, bool enforce = false );

    // Return current document name for the help.
    // This name is defined by current active state
    Q_INVOKABLE QString getCurrentHelpDocName();

    // Return true if current state is allow to exit. Note, this call is blocking because
    // it might requre some user input
    // nextWindowState type:  State::STATE
    Q_INVOKABLE bool canSwitchState(int nextWindowState);

    // Return current active state. Value from state::STATE
    Q_INVOKABLE int getCurrentStateId();

    // Resync require some more action as activate it properly.
    // That is why we have a special API for that
    Q_INVOKABLE void activateResyncState();

    // Switch to Show seed page
    Q_INVOKABLE void activateShowSeed(QString password);

    // Switch to a logout screen
    Q_INVOKABLE void logout();

    // Block logout, put that ID into the Q
    Q_INVOKABLE void blockLogout(QString id);
    // Unblock logout. Release all Q until this id. If id not found, nothing will be done
    Q_INVOKABLE void unblockLogout(QString id);
};

}

#endif //MWC_QT_WALLET_STATEMACHINE_B_H
