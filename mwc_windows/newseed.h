#ifndef NEWSEED_H
#define NEWSEED_H

#include "../wnd_core/walletwindow.h"

namespace Ui {
class NewSeed;
}

class NewSeed : public WalletWindow
{
    Q_OBJECT

public:
    explicit NewSeed(WalletWindowType wwType, QWidget *parent, const QVector<QString> & seed );
    virtual ~NewSeed() override;

    const QVector<QString> & getSeed() const {return seed;}
protected:
    bool validateData() override;

private:
    Ui::NewSeed *ui;
    QVector<QString> seed;
};

#endif // NEWSEED_H
