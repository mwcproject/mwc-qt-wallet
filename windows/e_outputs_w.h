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

    void setOutputCount(QString account, int count);
    void setOutputsData(QString account, int64_t height, const QVector<wallet::WalletOutput> & outp );

    // return selected account
    QString updateWalletBalance();

private slots:
    void on_accountComboBox_activated(int index);

    void on_prevBtn_clicked();

    void on_nextBtn_clicked();

private:
    void initTableHeaders();
    void saveTableHeaders();

    void requestOutputs(QString account);

    QString currentSelectedAccount();

    // return enable state for the buttons
    QPair<bool,bool> updatePages( int currentPos, int total, int pageSize );

    int calcPageSize() const;
private:
    Ui::Outputs *ui;
    state::Outputs * state;
    QVector<wallet::AccountInfo> accountInfo;
    QVector<wallet::WalletOutput> outputs;

    int currentPagePosition = 0; // position at the paging...
    int totalOutputs = 0;

    QPair<bool,bool> buttonState = QPair<bool,bool>(false, false);
};

}

#endif // OUTPUTS_H
