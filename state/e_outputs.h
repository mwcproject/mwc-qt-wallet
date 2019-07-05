#ifndef OUTPUTS_H
#define OUTPUTS_H

#include "state.h"
#include "../wallet/wallet.h"

namespace wnd {
class Outputs;
}

namespace state {


class Outputs : public QObject, public State
{
    Q_OBJECT
public:
    Outputs(const StateContext & context);
    virtual ~Outputs() override;

    void deleteWnd() {wnd = nullptr;}

    // request wallet for outputs
    void requestOutputs(QString account);

    QString getCurrentAccountName() const;

    // IO for columns widhts
    QVector<int> getColumnsWidhts() const;
    void updateColumnsWidhts(const QVector<int> & widths);

    void switchCurrentAccount(const wallet::AccountInfo & account);
    QVector<wallet::AccountInfo> getWalletBalance();

private slots:
    void onOutputs( QString account, int64_t height, QVector<wallet::WalletOutput> outputs);

    void onWalletBalanceUpdated();


protected:
    virtual NextStateRespond execute() override;

private:
    wnd::Outputs * wnd = nullptr;
};

}

#endif // OUTPUTS_H
