#ifndef ACCOUNTS_H
#define ACCOUNTS_H

#include <QWidget>

namespace Ui {
class Accounts;
}

class Accounts : public QWidget
{
    Q_OBJECT

public:
    explicit Accounts(QWidget *parent = nullptr);
    ~Accounts();

private:
    Ui::Accounts *ui;
};

#endif // ACCOUNTS_H
