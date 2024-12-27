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

#ifndef MWC_QT_WALLET_NODEOUTPUTPARSER_H
#define MWC_QT_WALLET_NODEOUTPUTPARSER_H

#include <QObject>
#include "inputparser.h"

namespace tries {

enum class NODE_OUTPUT_EVENT {
    NONE,
    MWC_NODE_STARTED,
    // Sync process, all states are here
    MWC_NODE_SYNC,
    // Need for checking that mode is not dead
    RECEIVE_BLOCK_START,
    RECEIVE_BLOCK_LISTEN,
    // Errors
    ADDRESS_ALREADY_IN_USE
};

QString toString(NODE_OUTPUT_EVENT event);

class NodeOutputParser : public QObject
{
    Q_OBJECT
public:
    NodeOutputParser();
    virtual ~NodeOutputParser() override;

    NodeOutputParser(const NodeOutputParser & other) = delete;
    NodeOutputParser & operator=(const NodeOutputParser & other) = delete;

    // Main routine processing with backed wallet printed
    // Results will be delieved async through signals
    void processInput(QString message);

private:
signals:
     void nodeOutputGenericEvent( NODE_OUTPUT_EVENT event, QString message);

protected:
    InputParser parser; // Work horse that is parsing all inputs
};

}

Q_DECLARE_METATYPE(tries::NODE_OUTPUT_EVENT );


#endif //MWC_QT_WALLET_NODEOUTPUTPARSER_H
