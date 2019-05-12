#ifndef LISTENINGW_H
#define LISTENINGW_H

#include <QWidget>

namespace Ui {
class Listening;
}

namespace state {
    class Listening;
}

namespace wnd {

class Listening : public QWidget
{
    Q_OBJECT

public:
    explicit Listening(QWidget *parent, state::Listening * state);
    ~Listening();

private slots:
    void on_nextAddressButton_clicked();
    void on_mwcAddressFromIndexButton_clicked();
    void on_mwcBoxConnectBtn_clicked();
    void on_keystoneConnectBtn_clicked();
    void on_foreignConnectBtn_clicked();

private:
    void updateBoxState();
    void updateKeystoneState();
    void updateHttpForeignApiState();


private:
    Ui::Listening *ui;
    state::Listening * state;
};

}

#endif // LISTENINGW_H
