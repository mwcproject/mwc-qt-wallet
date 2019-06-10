#ifndef NEWSEEDW_H
#define NEWSEEDW_H

#include <QWidget>

namespace Ui {
class NewSeed;
}

namespace state {
    class NewSeedShow;
}

namespace wnd {

class NewSeed : public QWidget
{
    Q_OBJECT

public:
    explicit NewSeed(QWidget *parent, state::NewSeedShow * state, const QVector<QString> & seed );
    virtual ~NewSeed() override;

protected:

private slots:
    void on_submitButton_clicked();

private:
    Ui::NewSeed *ui;
    state::NewSeedShow * state;
    QVector<QString> seed;
};

}

#endif // NEWSEEDW_H
