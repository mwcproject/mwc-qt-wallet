#ifndef SENDCOINS_H
#define SENDCOINS_H

#include <QWidget>

namespace Ui {
class SendCoins;
}

namespace wnd {

class SendCoins : public QWidget
{
    Q_OBJECT

public:
    explicit SendCoins(QWidget *parent = nullptr);
    ~SendCoins();

private:
    Ui::SendCoins *ui;
};

}

#endif // SENDCOINS_H
