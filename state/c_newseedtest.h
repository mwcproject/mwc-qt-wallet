#ifndef NEWSEEDTEST_H
#define NEWSEEDTEST_H

#include "state.h"
#include <QString>
#include <QPair>
#include "../core/testseedtask.h"
#include <QObject>

namespace state {

class NewSeedTest : public QObject, public State
{
    Q_OBJECT
public:
    NewSeedTest(const StateContext & context);
    virtual ~NewSeedTest() override;

    void submit(QString word);

protected:
    virtual NextStateRespond execute() override;

private slots:
    void onWalletBalanceUpdated();

private:
    core::TestSeedTask currentTask;
    QMetaObject::Connection slotConn;
};

}

#endif // NEWSEEDTEST_H
