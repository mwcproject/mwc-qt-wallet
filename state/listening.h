#ifndef LISTENING_H
#define LISTENING_H

#include "state.h"
#include "../wallet/wallet.h"

namespace state {

class Listening : public State
{
public:
    Listening(const StateContext & context);
    virtual ~Listening() override;

    QPair<bool, QString> getBoxListeningStatus();
    QPair<bool, QString> getKeystoneListeningStatus();
    //QPair<bool, QString> getForeightApiListeningStatus();

    QPair<bool, QString> startBoxListeningStatus();
    QPair<bool, QString> startKeystoneListeningStatus();
    //QPair<bool, QString> startForeightApiListeningStatus(int port, QString foregnApiSecret);

    QPair<bool, QString> stopBoxListeningStatus();
    QPair<bool, QString> stopKeystoneListeningStatus();
    //QPair<bool, QString> stopForeightApiListeningStatus();


    QPair<QString,int> getBoxAddress();
    void changeBoxAddress(int idx);
    void nextBoxAddress();

protected:
    virtual NextStateRespond execute() override;
};

}

#endif // LISTENING_H
