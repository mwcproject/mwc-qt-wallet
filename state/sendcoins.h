#ifndef SENDCOINS_H
#define SENDCOINS_H

#include "state.h"
#include "../wallet/wallet.h"
#include <QVector>

namespace state {

struct SendCoinsParams {
    int inputConfirmationNumber;
    int changeOutputs;

    SendCoinsParams() :
        inputConfirmationNumber(10), changeOutputs(1) {}

    SendCoinsParams( int _inputConfirmationNumber, int _changeOutputs) :
        inputConfirmationNumber(_inputConfirmationNumber), changeOutputs(_changeOutputs) {}

    void setData(int _inputConfirmationNumber, int _changeOutputs) {
        inputConfirmationNumber = _inputConfirmationNumber;
        changeOutputs = _changeOutputs;
    }

    void saveData(QDataStream & out) const;
    bool loadData(QDataStream & in);
};

class SendCoins : public State
{
public:
    SendCoins(const StateContext & context);
    virtual ~SendCoins() override;

    SendCoinsParams getSendCoinsParams();
    void updateSendCoinsParams( const SendCoinsParams  & params );

    QVector<wallet::WalletContact> getContacts();

    QVector<wallet::WalletTransaction> getTransactions(int numOfTransactions);

    QPair<bool, QString> sendCoins( long nanoCoins, QString address, QString message );

    QVector<int> getColumnsWidhts() const;
    void updateColumnsWidhts(const QVector<int> & widths);
protected:
    virtual NextStateRespond execute() override;

};

}

#endif // SENDCOINS_H
