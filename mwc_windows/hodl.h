#ifndef HODL_H
#define HODL_H

#include <QWidget>

namespace Ui {
class Hodl;
}

class Hodl : public QWidget
{
    Q_OBJECT

public:
    explicit Hodl(QWidget *parent = nullptr);
    ~Hodl();

private:
    Ui::Hodl *ui;
};

#endif // HODL_H
