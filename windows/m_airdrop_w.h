#ifndef AIDROPW_H
#define AIDROPW_H

#include <QWidget>

namespace Ui {
class Airdrop;
}

namespace state {
    class Airdrop;
    struct AirDropStatus;
    struct AirdropRequests;
}

namespace wnd {

class Airdrop : public QWidget
{
    Q_OBJECT

public:
    explicit Airdrop(QWidget *parent, state::Airdrop * state);
    ~Airdrop();

    // true if status is active
    bool updateAirDropStatus( const state::AirDropStatus & status );

    bool updateClaimStatus( int idx, const state::AirdropRequests & request,
                            QString status, QString message, int64_t amount, int errCode);

    void reportMessage( QString title, QString message );
private slots:
    void on_nextButton_clicked();

    void on_refreshClaimsButton_clicked();

    void on_btcAddressEdit_textChanged(const QString &arg1);

    void on_claimAirdropBtn_clicked();

private:
    // initiate update status from the state
    void updateClaimStatus();

    void initTableHeaders();
    void saveTableHeaders();

    void showProgress(const QString & message);
    void hideProgress();
private:
    Ui::Airdrop *ui;
    state::Airdrop * state;
};

}

#endif // AIDROPW_H
