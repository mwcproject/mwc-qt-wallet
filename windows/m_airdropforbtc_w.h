#ifndef AIRDROPFORBTC_H
#define AIRDROPFORBTC_H

#include <QWidget>

namespace Ui {
class AirdropForBTC;
}

namespace state {
    class Airdrop;
}

namespace wnd {

class AirdropForBTC : public QWidget
{
    Q_OBJECT

public:
    explicit AirdropForBTC(QWidget *parent, state::Airdrop * state, QString btcAddress, QString challenge, QString identifier );
    ~AirdropForBTC();

    void reportMessage( QString title, QString message );

private slots:
    void on_claimButton_clicked();
    void on_backButton_clicked();

private:
    Ui::AirdropForBTC *ui;
    state::Airdrop * state;
    QString btcAddress;
    QString challenge;
    QString identifier;
};

}

#endif // AIRDROPFORBTC_H
