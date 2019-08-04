#ifndef InitAccountW_H
#define InitAccountW_H

#include <QWidget>

namespace Ui {
class InitAccount;
}

namespace state {
    class InitAccount;
    class WalletConfig;
}

namespace wnd {

class InitAccount : public QWidget
{
    Q_OBJECT

public:
    explicit InitAccount(QWidget *parent, state::InitAccount * state, state::WalletConfig * configState);
    virtual ~InitAccount() override;

private slots:
    void on_password1Edit_textChanged(const QString &text);

    void on_submitButton_clicked();

    void on_instancesButton_clicked();

private:
    Ui::InitAccount *ui;
    state::InitAccount * state;
    state::WalletConfig * configState;
};

}

#endif // InitAccountW_H
