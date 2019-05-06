#ifndef SENDCOINSW_H
#define SENDCOINSW_H

#include <QWidget>

namespace Ui {
class SendCoins;
}

namespace state {
    class SendCoins;
}

namespace wnd {

class SendCoins : public QWidget
{
    Q_OBJECT

public:
    explicit SendCoins(QWidget *parent, state::SendCoins * state);
    virtual ~SendCoins() override;

private slots:
    void on_fromContactButton_clicked();

    void on_sendButton_clicked();

    void on_settingsButton_clicked();

private:
    void updateTransactionList();

private:
    Ui::SendCoins *ui;
    state::SendCoins * state;
};

}

#endif // SENDCOINSW_H
