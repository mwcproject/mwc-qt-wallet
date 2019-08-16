#ifndef MWC_QT_WALLET_NODEINFO_H
#define MWC_QT_WALLET_NODEINFO_H

#include "state.h"
#include "../wallet/wallet.h"

namespace wnd {
class NodeInfo;
}

namespace state {

class NodeInfo : public QObject, public State {
Q_OBJECT
public:
    NodeInfo(StateContext * context);
    virtual ~NodeInfo() override;

    void wndIsGone(wnd::NodeInfo * w) { if(w==wnd) wnd = nullptr;} // window is closed

    void requestNodeInfo();

    wallet::WalletConfig getWalletConfig() const;
    void updateWalletConfig( const wallet::WalletConfig & config );

protected:
    virtual NextStateRespond execute() override;
    virtual QString getHelpDocName() override {return "node_overview.html";}

private slots:
    void onNodeStatus( bool online, QString errMsg, int height, int64_t totalDifficulty, int connections );

private:
    wnd::NodeInfo * wnd = nullptr;
};

}

#endif //MWC_QT_WALLET_NODEINFO_H
