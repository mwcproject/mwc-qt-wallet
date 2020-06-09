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

#include "corewindow_b.h"
#include "BridgeManager.h"

namespace bridge {

CoreWindow::CoreWindow(QObject *parent) : QObject(parent) {
    getBridgeManager()->addCoreWindow(this);
}

CoreWindow::~CoreWindow() {
    getBridgeManager()->removeCoreWindow(this);
}

// Window manager
void CoreWindow::setDataPath(const QString& dataPath) {
    emit sgnSetDataPath(dataPath);
}

void CoreWindow::updateActionStates(state::STATE actionState) {
    emit sgnUpdateActionStates( int(actionState) );
}


}