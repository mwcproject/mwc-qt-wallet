#ifndef WALLETWINDOW_H
#define WALLETWINDOW_H

#include <QWidget>

class WalletData;

enum WalletWindowType {
    CONNECT_TO_SERVER,
    NODE_MANUALLY, // setup node end point manually
    NODE_STATUS,
    SELECT_WALLET,
    NEW_PASSWORD,
    NEW_WALLET,
    NEW_SEED,
    CONFIRM_SEED,
    ENTER_SEED,
    FROM_SEED_FILE,

    MAIN_WINDOW
};

enum WalletWindowAction { CANCEL, NEXT };

// Setup windows factory
//QWidget * createWidget(WalletWindowType wnd, WalletData & data, QWidget * mainWindow );


class WalletWindow : public QWidget
{
    Q_OBJECT
public:
    explicit WalletWindow(WalletWindowType wndType, QWidget *parent) :
        QWidget(parent), windowType(wndType) {}

    virtual ~WalletWindow() = default;

    WalletWindowType getWindowType() const {return windowType;}

    virtual bool validateData() = 0;
private:
    const WalletWindowType windowType;
};


#endif // WALLETWINDOW_H
