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

#include "e_outputs.h"
#include "../core/appcontext.h"
#include "../core/Config.h"
#include "../state/statemachine.h"
#include "../util/crypto.h"
#include "../util/Log.h"
#include <QDebug>
#include "../core/global.h"
#include "../core/WndManager.h"
#include "../bridge/BridgeManager.h"
#include "../bridge/wnd/e_outputs_b.h"

namespace state {

Outputs::Outputs(StateContext * context) :
    State(context, STATE::OUTPUTS)
{
}

Outputs::~Outputs() {}

NextStateRespond Outputs::execute() {
    if (context->appContext->getActiveWndState() != STATE::OUTPUTS)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    core::getWndManager()->pageOutputs();

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

}
