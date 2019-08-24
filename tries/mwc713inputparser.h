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

#ifndef MWC713INPUTPARSER_H
#define MWC713INPUTPARSER_H

#include "inputparser.h"
#include <QObject>
#include "../wallet/mwc713events.h"

namespace tries {

// Receive signals
class Mwc713InputGetter {

};

class Mwc713InputParser : public QObject
{
    Q_OBJECT
public:
    Mwc713InputParser();
    virtual ~Mwc713InputParser();

    Mwc713InputParser(const Mwc713InputParser & other) = delete;
    Mwc713InputParser & operator=(const Mwc713InputParser & other) = delete;

    // Main routine processing with backed wallet printed
    // Resilting will be delieved async through signals
    void processInput(QString message);

private:

    // Register callbacks for event that we are going to process.
    // See signals below. See for prefix 's'
    void initWalletReady();
    void initWelcome();
    void initInit();
    void initNeedUnlock();
    void initPasswordError(); // notification about the wrong password.
    void initBoxListener();
    void initMwcMqAddress();
    void initInitWalletWorkflow();
    void initListening();
    void initRecovery();
    void initAccount();
    void initSend();
    void initTransactions();

    void initGenericError(); // All error messages

signals:
    void sgGenericEvent( wallet::WALLET_EVENTS event, QString message);

protected:
    InputParser parser; // Work horse thet is parsing all inputs
};

}

#endif // MWC713INPUTPARSER_H
