#ifndef OUTPUTSW_H
#define OUTPUTSW_H

#include "../core/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class Outputs;
}

namespace state {
    class Outputs;
}

namespace wnd {

class Outputs : public core::NavWnd
{
    Q_OBJECT

public:
    explicit Outputs(QWidget *parent, state::Outputs * state);
    ~Outputs();

    void setOutputsData(QString account, int64_t height, const QVector<wallet::WalletOutput> & outp );

    // return selected account
    QString updateWalletBalance();

private slots:
    void on_accountComboBox_activated(int index);

private:
    void initTableHeaders();
    void saveTableHeaders();

    void requestOutputs(QString account);

private:
    Ui::Outputs *ui;
    state::Outputs * state;
    QVector<wallet::AccountInfo> accountInfo;
    QVector<wallet::WalletOutput> outputs;

};

}

#endif // OUTPUTS_H
