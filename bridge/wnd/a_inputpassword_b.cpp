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

#include "a_inputpassword_b.h"
#include "../BridgeManager.h"
#include "../../state/a_inputpassword.h"
#include "../../state/state.h"
#include "../../state/statemachine.h"

namespace bridge {

static state::InputPassword * getState() {
    return (state::InputPassword *) state::getState(state::STATE::INPUT_PASSWORD);
}

InputPassword::InputPassword(QObject* parent) : QObject(parent) {
    getBridgeManager()->addInputPassword(this);
}

InputPassword::~InputPassword() {
    getBridgeManager()->removeInputPassword(this);
}

// Try to login into the wallet with this password.
void InputPassword::submitPassword(QString password) {
    getState()->submitPassword(password);
}


}