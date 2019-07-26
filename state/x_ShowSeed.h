#ifndef MWC_QT_WALLET_SHOWSEED_H
#define MWC_QT_WALLET_SHOWSEED_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../state/a_initaccount.h"

namespace wnd {
class NewSeed;
}

namespace state {

class ShowSeed : public QObject, public State, SubmitCaller {
    Q_OBJECT
public:
    ShowSeed( StateContext * context);
    virtual ~ShowSeed() override;

    void deleteWnd() {wnd= nullptr;}

protected:
    virtual NextStateRespond execute() override;

    // submit is hidden, never expected to be called.
    virtual void submit() override {Q_ASSERT(false);}
    virtual void wndDeleted(wnd::NewSeed * w) override { if (w==wnd) wnd=nullptr; }

private slots:
    void recoverPassphrase( QVector<QString> seed );

private:
    wnd::NewSeed * wnd = nullptr; // not own, just a pointer
};



}

#endif //MWC_QT_WALLET_SHOWSEED_H
