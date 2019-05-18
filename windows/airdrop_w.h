#ifndef AIDROPW_H
#define AIDROPW_H

#include <QWidget>

namespace Ui {
class Airdrop;
}

namespace state {
    class Airdrop;
}

namespace wnd {

class Airdrop : public QWidget
{
    Q_OBJECT

public:
    explicit Airdrop(QWidget *parent, state::Airdrop * state);
    ~Airdrop();

private slots:
    void on_nextButton_clicked();

    void on_refreshClaimsButton_clicked();

private:
    void updateClaimStatus();

private:
    Ui::Airdrop *ui;
    state::Airdrop * state;
};

}

#endif // AIDROPW_H
