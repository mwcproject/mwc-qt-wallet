#ifndef ENTERSEED_H
#define ENTERSEED_H

#include <QWidget>

namespace Ui {
class EnterSeed;
}

namespace state {
    class CreateWithSeed;
}

namespace wnd {

class EnterSeed : public QWidget
{
    Q_OBJECT

public:
    explicit EnterSeed(QWidget *parent, state::CreateWithSeed * state );
    virtual ~EnterSeed() override;

private slots:
    void on_submitButton_clicked();
    void on_Enter();

private:
    Ui::EnterSeed *ui;
    state::CreateWithSeed * state;
};

}

#endif // ENTERSEED_H
