#ifndef ENTERSEED_H
#define ENTERSEED_H

#include "../wnd_core/walletwindow.h"

namespace Ui {
class EnterSeed;
}

class EnterSeed : public WalletWindow
{
    Q_OBJECT

public:
    explicit EnterSeed(WalletWindowType appType, QWidget *parent);
    virtual ~EnterSeed() override;

    virtual bool validateData() override;

    const QVector<QString> & getSeed() const {return seed;}

private:
    Ui::EnterSeed *ui;
    QVector<QString> seed;
};

#endif // ENTERSEED_H
