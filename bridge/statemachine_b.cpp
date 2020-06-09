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

#include "statemachine_b.h"
#include "../state/state.h"
#include "../state/statemachine.h"
#include "../core/appcontext.h"

namespace bridge {

static state::StateMachine * getStateMachine() {
    return state::getStateContext()->stateMachine;
}

StateMachine::StateMachine(QObject *parent) : QObject(parent) {}

StateMachine::~StateMachine() {}

// actionWindowState:  state::STATE
bool StateMachine::setActionWindow( int actionWindowState, bool enforce ) {
    // Use Special methods for those states!
    Q_ASSERT(actionWindowState!=state::STATE::RESYNC && actionWindowState!=state::STATE::SHOW_SEED);

    return getStateMachine()->setActionWindow( state::STATE(actionWindowState), enforce );
}

QString StateMachine::getCurrentHelpDocName() {
    state::State * state = getStateMachine()->getCurrentStateObj();
    QString docName = "";
    if (state != nullptr) {
        docName = state->getHelpDocName();
    }

    if ( docName.isEmpty() ) {
        docName = "default.html";
    }

    return docName;
}

// Return true if current state is allow to exit. Note, this call is blocking because
// it might requre some user input
bool StateMachine::canSwitchState() {
    return getStateMachine()->canSwitchState();
}

// Return current active state. Value from state::STATE
int StateMachine::getCurrentStateId() {
    return getStateMachine()->getCurrentStateId();
}

// Resync require some more action as activate it properly.
// That is why we have a special API for that
void StateMachine::activateResyncState() {
    auto appContext = state::getStateContext()->appContext;
    appContext->pushCookie("PrevState", (int)appContext->getActiveWndState());
    getStateMachine()->setActionWindow( state::STATE::RESYNC );
}

// Switch to Show seed page
void StateMachine::activateShowSeed(QString password) {
    auto appContext = state::getStateContext()->appContext;
    appContext->pushCookie<QString>("password", password);
    getStateMachine()->setActionWindow( state::STATE::SHOW_SEED );
}

// Switch to a logout screen
void StateMachine::logout() {
    getStateMachine()->logout();
}

// Block logout, put that ID into the stack
void StateMachine::blockLogout(QString id) {
    getStateMachine()->blockLogout(id);
}
// Unblock logout. Release all stack until this id. If id not found, nothing will be done
// EMpty id - unblock all
void StateMachine::unblockLogout(QString id) {
    getStateMachine()->unblockLogout(id);
}


}
