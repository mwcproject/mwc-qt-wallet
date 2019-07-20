#ifndef NEWSEEDTEST_H
#define NEWSEEDTEST_H

#include "state.h"
#include <QString>
#include <QPair>
#include "../core/testseedtask.h"
#include <QObject>

namespace wnd {
class NewSeedTest;
}

namespace state {

class NewSeedTest : public QObject, public State
{
    Q_OBJECT
public:
    NewSeedTest( StateContext * context);
    virtual ~NewSeedTest() override;

    void deleteWnd(wnd::NewSeedTest * w) { if(w==wnd) wnd=nullptr;}

    void submit(QString word);

protected:
    virtual NextStateRespond execute() override;

private slots:
    void onWalletBalanceUpdated();

private:
    wnd::NewSeedTest * wnd = nullptr;
    core::TestSeedTask currentTask;

};

}

#endif // NEWSEEDTEST_H
