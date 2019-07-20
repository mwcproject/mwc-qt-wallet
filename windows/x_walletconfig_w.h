#ifndef WALLETCONFIGW_H
#define WALLETCONFIGW_H

#include "../core/navwnd.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"

namespace Ui {
class WalletConfig;
}

namespace state {
    class WalletConfig;
}

namespace wnd {

class WalletConfig : public core::NavWnd
{
    Q_OBJECT

public:
    explicit WalletConfig(QWidget *parent, state::WalletConfig * state);
    ~WalletConfig();

private slots:
    void on_mwc713directorySelect_clicked();

    void on_mwc713directoryEdit_textEdited(const QString &arg1);

    void on_keybasePathEdit_textChanged(const QString &arg1);

    void on_keybasePathSelect_clicked();

    void on_confirmationNumberEdit_textChanged(const QString &arg1);

    void on_changeOutputsEdit_textEdited(const QString &arg1);

    void on_restoreDefault_clicked();

    void on_applyButton_clicked();

    void on_mwcmqHost_textEdited(const QString &arg1);

private:
    void setValues(const QString & mwc713directory,
                   const QString & keyBasePath,
                   const QString & mwcmqHost,
                   int inputConfirmationNumber,
                   int changeOutputs);
    // return true if data is fine. In case of error will show message for the user
    bool readInputValue( wallet::WalletConfig & newWalletConfig, core::SendCoinsParams & newSendParams );

    QString mwcDomainConfig2InputStr(QString mwcDomain);
    QString mwcDomainInputStr2Config(QString mwcDomain);

    QString keybasePathConfig2InputStr(QString kbpath);
    QString keybasePathInputStr2Config(QString kbpath);

        // Update button state
    void updateButtons();

private:
    Ui::WalletConfig *ui;
    state::WalletConfig * state;

    wallet::WalletConfig currentWalletConfig;
    core::SendCoinsParams sendParams;

    wallet::WalletConfig defaultWalletConfig;
    core::SendCoinsParams defaultSendParams;
};

}

#endif // WALLETCONFIGW_H
