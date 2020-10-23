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

#include "utils.h"
#include "../mwc713task.h"

namespace wallet {

QString getErrorMessage(const QVector<WEvent> &events, QString defaultMessage) {
    QString errors;
    QVector<WEvent> errs = filterEvents(events, WALLET_EVENTS::S_GENERIC_ERROR);
    for (auto &e : errs) {
        if (errors.length() > 0)
            errors += "; ";
        errors += e.message;
    }
    if (errors.isEmpty()) {
        errors = defaultMessage;
    }
    return errors;
}

};
