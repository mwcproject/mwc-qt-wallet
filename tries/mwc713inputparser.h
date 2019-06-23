#ifndef MWC713INPUTPARSER_H
#define MWC713INPUTPARSER_H

#include "inputparser.h"
#include <QObject>
#include "../wallet/mwc713events.h"

namespace tries {

// Recieve signals
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
