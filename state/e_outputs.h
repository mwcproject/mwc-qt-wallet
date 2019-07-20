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
    Outputs( StateContext * context);
    virtual ~Outputs() override;

    void deleteWnd(wnd::Outputs * w) { if(w==wnd) wnd = nullptr;}

    // request wallet for outputs
    void requestOutputCount(QString account);
    void requestOutputs(QString account, int offset, int number);

    QString getCurrentAccountName() const;

    // IO for columns widhts
    QVector<int> getColumnsWidhts() const;
    void updateColumnsWidhts(const QVector<int> & widths);

    void switchCurrentAccount(const wallet::AccountInfo & account);
    QVector<wallet::AccountInfo> getWalletBalance();

private slots:
    void onOutputCount(QString account, int count);
    void onOutputs( QString account, int64_t height, QVector<wallet::WalletOutput> outputs);

    void onWalletBalanceUpdated();


protected:
    virtual NextStateRespond execute() override;

private:
    wnd::Outputs * wnd = nullptr;
};

}

#endif // OUTPUTS_H
