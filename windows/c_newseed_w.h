#ifndef NEWSEEDW_H
#define NEWSEEDW_H

#include "../core/navwnd.h"

namespace Ui {
class NewSeed;
}

namespace state {
    class SubmitCaller;
    struct StateContext;
}

namespace wnd {

class NewSeed : public core::NavWnd
{
    Q_OBJECT

public:
    explicit NewSeed(QWidget *parent, state::SubmitCaller * state, state::StateContext * context,
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
    state::StateContext * context;
    QVector<QString> seed;
};

}

#endif // NEWSEEDW_H
