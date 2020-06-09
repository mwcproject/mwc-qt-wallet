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

#include "timeoutlock.h"
#include "../state/state.h"
#include "../bridge/statemachine_b.h"

namespace util {

TimeoutLockObject::TimeoutLockObject( QString _id ) : id(_id) {

    stateMachine = new bridge::StateMachine(nullptr);
    stateMachine->blockLogout(id);
}

TimeoutLockObject::~TimeoutLockObject() {
    stateMachine->unblockLogout(id);
    delete stateMachine;
}


}