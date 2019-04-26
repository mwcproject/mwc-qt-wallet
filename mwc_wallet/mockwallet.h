#ifndef MOCKWALLET_H
#define MOCKWALLET_H

#include "wallet.h"

class MockWallet : public Wallet
{
public:
    MockWallet();
    virtual ~MockWallet() override;

    //virtual QVector<QString> createNewSeed() override;

};

#endif // MOCKWALLET_H
