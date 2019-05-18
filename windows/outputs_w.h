#ifndef OUTPUTSW_H
#define OUTPUTSW_H

#include <QWidget>

namespace Ui {
class Outputs;
}

namespace state {
    class Outputs;
}

namespace wnd {

class Outputs : public QWidget
{
    Q_OBJECT

public:
    explicit Outputs(QWidget *parent, state::Outputs * state);
    ~Outputs();

private:
    void initTableHeaders();
    void saveTableHeaders();

    void updateOutputsTable();
private:
    Ui::Outputs *ui;
    state::Outputs * state;
};

}

#endif // OUTPUTS_H
