#ifndef CLAIMING_H
#define CLAIMING_H

#include <QWidget>

namespace Ui {
class Claiming;
}

class Claiming : public QWidget
{
    Q_OBJECT

public:
    explicit Claiming(QWidget *parent = nullptr);
    ~Claiming();

private:
    Ui::Claiming *ui;
};

#endif // CLAIMING_H
