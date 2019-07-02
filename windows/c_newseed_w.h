#ifndef NEWSEEDW_H
#define NEWSEEDW_H

#include "../core/navwnd.h"

namespace Ui {
class NewSeed;
}

namespace state {
    class SubmitCaller;
}

namespace wnd {

class NewSeed : public core::NavWnd
{
    Q_OBJECT

public:
    explicit NewSeed(QWidget *parent, state::SubmitCaller * state, state::StateMachine * stateMachine,
                const QVector<QString> & seed, bool hideSubmitButton = false );
    virtual ~NewSeed() override;

    // if seed empty or has size 1, it is error message
    void showSeedData(const QVector<QString> & seed);

protected:
    void updateSeedData(const QVector<QString> & seed);

private slots:
    void on_submitButton_clicked();

private:
    Ui::NewSeed *ui;
    state::SubmitCaller * state;
    QVector<QString> seed;
};

}

#endif // NEWSEEDW_H
