#ifndef NEWSEEDTESTW_H
#define NEWSEEDTESTW_H

#include <QWidget>

namespace Ui {
class NewSeedTest;
}

namespace state {
    class NewSeedTest;
}

namespace wnd {

class NewSeedTest : public QWidget
{
    Q_OBJECT

public:
    explicit NewSeedTest(QWidget *parent, state::NewSeedTest *state, int wordNumber);
    ~NewSeedTest();

private slots:
    void on_submitButton_clicked();

private:
    Ui::NewSeedTest *ui;
    state::NewSeedTest *state;
};

}


#endif // NEWSEEDTESTW_H
