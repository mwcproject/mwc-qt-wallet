#ifndef INITACCOUNT_H
#define INITACCOUNT_H

#include "state.h"
#include <QString>
#include <QObject>
#include <QVector>
#include "../core/testseedtask.h"
#include "../windows/z_progresswnd.h"

namespace wnd {
class NewSeed;
class ProgressWnd;
class EnterSeed;
class NewSeedTest;
}

namespace state {

class SubmitCaller {
public:
    virtual void submit() = 0;
    virtual void wndDeleted(wnd::NewSeed * w) = 0;
};

class InitAccount : public QObject, public State, public SubmitCaller, public wnd::IProgressWndState
{
    Q_OBJECT
public:
    InitAccount(StateContext * context);
    virtual ~InitAccount() override;

    // Step 1
    void setPassword(const QString & password);

    // Step 2
    enum NEW_WALLET_CHOICE { CREATE_NEW, CREATE_WITH_SEED };
    void submitCreateChoice(NEW_WALLET_CHOICE newWalletChoice);

    // Step 3  New seed
    virtual void submit() override;
    virtual void wndDeleted(wnd::NewSeed * w) override;

    // Single Word verification
    void submit(QString word);

    // create form seed
    void createWalletWithSeed( QVector<QString> sd );


    void cancel();

    virtual void cancelProgress() override {Q_ASSERT(false);}
    virtual void destroyProgressWnd(wnd::ProgressWnd *) override {progressWnd= nullptr;}
    void deleteEnterSeed(wnd::EnterSeed *) {}
    void deleteNewSeedTestWnd(wnd::NewSeedTest *) {}

private slots:
    void onNewSeed(QVector<QString> seed);

    void onLoginResult(bool ok);

    void onRecoverProgress( int progress, int maxVal );

    void onRecoverResult(bool started, bool finishedWithSuccess, QString newAddress, QStringList errorMessages);

protected:
    virtual NextStateRespond execute() override;

    // return true if done
    bool finishSeedVerification();
private:
    wnd::ProgressWnd* progressWnd = nullptr;
    int progressMaxVal = 10;

    QString pass;
    QVector<QString> seed;
    QVector<core::TestSeedTask> tasks;
};


}


#endif // INITACCOUNT_H
