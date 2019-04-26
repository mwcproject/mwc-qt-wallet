#ifndef CONFIRMSEED_H
#define CONFIRMSEED_H

#include <QWidget>
#include "../wnd_core/walletwindow.h"
#include <QVector>

namespace Ui {
class ConfirmSeed;
}

class ConfirmSeed : public WalletWindow
{
    Q_OBJECT

public:
    explicit ConfirmSeed(WalletWindowType wwType, QWidget *parent, QVector<QString> seedWords );
    virtual ~ConfirmSeed() override;

protected:
    bool validateData() override;
private:
    Ui::ConfirmSeed *ui;
    QVector<QString> seed2check;
};

#endif // CONFIRMSEED_H
