#ifndef InitAccountW_H
#define InitAccountW_H

#include <QWidget>

namespace Ui {
class InitAccount;
}

namespace state {
    class InitAccount;
}

namespace wnd {

class InitAccount : public QWidget
{
    Q_OBJECT

public:
    explicit InitAccount(QWidget *parent, state::InitAccount * state);
    ~InitAccount();

private slots:
    void on_password1Edit_textChanged(const QString &text);

    void on_submitButton_clicked();

private:
    Ui::InitAccount *ui;
    state::InitAccount * state;
};

}

#endif // InitAccountW_H
