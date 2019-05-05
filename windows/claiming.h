#ifndef CLAIMING_H
#define CLAIMING_H

#include <QWidget>

namespace Ui {
class Claiming;
}

namespace wnd {

class Claiming : public QWidget
{
    Q_OBJECT

public:
    explicit Claiming(QWidget *parent = nullptr);
    ~Claiming();

private:
    Ui::Claiming *ui;
};

}

#endif // CLAIMING_H
