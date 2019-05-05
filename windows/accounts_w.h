#ifndef ACCOUNTS_H
#define ACCOUNTS_H

#include <QWidget>

namespace Ui {
class Accounts;
}

namespace state {
    class Accounts;
}

class QListWidgetItem;

namespace wnd {

class Accounts : public QWidget
{
    Q_OBJECT

public:
    explicit Accounts(QWidget *parent, state::Accounts * state);
    ~Accounts();

private slots:
    void on_addAccountButton_clicked();

    void on_refreshButton_clicked();

    void on_accountList_itemActivated(QListWidgetItem *item);

private:
    void updateUiState();
    void startWaiting();
    void stopWaiting();

private:
    Ui::Accounts *ui;
    state::Accounts * state;
};

}

#endif // ACCOUNTS_H
