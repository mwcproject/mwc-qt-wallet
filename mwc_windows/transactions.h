#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

#include <QWidget>

namespace Ui {
class Transactions;
}

class Transactions : public QWidget
{
    Q_OBJECT

public:
    explicit Transactions(QWidget *parent = nullptr);
    ~Transactions();

private:
    Ui::Transactions *ui;
};

#endif // TRANSACTIONS_H
