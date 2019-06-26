#ifndef E_RECIEVE_W_H
#define E_RECIEVE_W_H

#include <QWidget>

namespace Ui {
class Recieve;
}

namespace state {
class Recieve;
}

namespace wnd {

class Recieve : public QWidget {
Q_OBJECT

public:
    explicit Recieve(QWidget *parent, state::Recieve * state, bool mwcMqStatus, bool keybaseStatus,
                     QString mwcMqAddress);

    virtual ~Recieve() override ;

    void updateMwcMqAddress(QString address);
    void updateMwcMqState(bool online);
    void updateKeybaseState(bool online);

    void onTransactionActionIsFinished( bool success, QString message );

private slots:
    void on_pushButton_clicked();

private:

private:
    Ui::Recieve *ui;
    state::Recieve * state;
};

}

#endif // E_RECIEVE_W_H
