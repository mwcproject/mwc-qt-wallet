#ifndef INPUTPASSWORDW_H
#define INPUTPASSWORDW_H

#include <QWidget>

namespace Ui {
class InputPassword;
}

namespace state {
    class InputPassword;
}

namespace wnd {

class InputPassword : public QWidget
{
    Q_OBJECT

public:
    explicit InputPassword(QWidget *parent, state::InputPassword * state, bool lockMode);
    ~InputPassword();

    void startWaiting();
    void stopWaiting();

    void reportWrongPassword();

    void updateMwcMqState(bool online);
    void updateKeybaseState(bool online);

private slots:
    void on_submitButton_clicked();

    void on_instancesButton_clicked();

private:
    Ui::InputPassword *ui;
    state::InputPassword * state;
};

}

#endif // INPUTPASSWORDW_H
