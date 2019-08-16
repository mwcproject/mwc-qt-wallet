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

#include "mwc713task.h"

namespace wallet {

Mwc713Task::Mwc713Task(QString _taskName, QString _inputStr, MWC713 * _wallet713, QString _shadowStr) :
    taskName(_taskName), wallet713(_wallet713), inputStr(_inputStr), shadowStr(_shadowStr)
{
    Q_ASSERT(wallet713);
}

Mwc713Task::~Mwc713Task() {
}

// Filter events by type
QVector< WEvent > filterEvents(const QVector<WEvent> & events, WALLET_EVENTS type ) {
    QVector< WEvent > res;
    for (const auto & e : events) {
        if (e.event == type)
            res.push_back(e);
    }
    return res;
}

// Print events into the string
QString printEvents(const QVector<WEvent> & events) {
    QString res;

    for (const auto & e : events) {
        if (res.length()>0)
            res += ", ";

        res += "Evt(T=" + toString(e.event) + ", msg=" + e.message + ")";
    }
    return res;
}

}

