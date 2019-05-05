#ifndef OUTPUTS_H
#define OUTPUTS_H

#include <QWidget>

namespace Ui {
class Outputs;
}

namespace wnd {

class Outputs : public QWidget
{
    Q_OBJECT

public:
    explicit Outputs(QWidget *parent = nullptr);
    ~Outputs();

private:
    Ui::Outputs *ui;
};

}

#endif // OUTPUTS_H
