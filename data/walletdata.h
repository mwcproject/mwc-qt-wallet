#ifndef WALLETDATA_H
#define WALLETDATA_H

#include <QString>

class NodeConnection {
public:
    enum NodeType { NONE,  // not init
                    LOCAL, // node renning locally
                    WMC_POOL, // from mwc pool
                    SELECTED }; // Details are selected by user

    void setNode(NodeType nd) {
        node=nd;
        host = "";
        port = 0;
    }

    void setNodeHostPort(const QString & h, int p ) {
        host = h;
        port = p;
    }

    NodeType getNode() const {return node;}
    const QString & getHost() const {return host;}
    int getPort() const {return port;}

private:
    NodeType node = NONE;
    QString  host;
    int      port = 0;
};

class SessionPassword {
public:
    long      hash;  // permanent
    QString    password; // shoudl never stored and better not even keep in memory.
};

// Wallet data container
class WalletData
{
public:
    WalletData();

    SessionPassword * getSessionPswd() {return &sessionPswd;}
    NodeConnection * getNodeConnection() {return &nodeCon;}
private:
    SessionPassword sessionPswd;
    NodeConnection  nodeCon;
};

#endif // WALLETDATA_H
