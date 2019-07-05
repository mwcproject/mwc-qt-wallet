#ifndef MWC_QT_WALLET_RESYNC_H
#define MWC_QT_WALLET_RESYNC_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../windows/z_progresswnd.h"

namespace state {

// Do resync for account. Expected that caller want us to start 'check' process for mwc713
class Resync : public QObject, public State, public wnd::IProgressWndState {
    Q_OBJECT
public:
    Resync(const StateContext & context);
    virtual ~Resync() override;


    virtual void cancelProgress() override {Q_ASSERT(false);} // progress cancel not expected
    virtual void destroyProgressWnd() override {wnd = nullptr;}
protected:
    virtual NextStateRespond execute() override;

private slots:
    void onRecoverProgress( int progress, int maxVal );
    void onCheckResult(bool ok, QString errors );

private:
    wnd::ProgressWnd * wnd = nullptr;
    int prevState = STATE::TRANSACTIONS;
    QPair<bool,bool> prevListeningStatus = QPair<bool,bool>(false,false);
    int maxProgrVal = 100;
};

}

#endif //MWC_QT_WALLET_RESYNC_H
