#ifndef LISTENINGW_H
#define LISTENINGW_H

#include "../core/navwnd.h"

namespace Ui {
class Listening;
}

namespace state {
    class Listening;
}

namespace wnd {

class Listening : public core::NavWnd
{
    Q_OBJECT

public:
    explicit Listening(QWidget *parent, state::Listening * state, bool mwcMqStatus, bool keybaseStatus,
                       QString mwcMqAddress, int mwcMqAddrIdx);
    ~Listening();

    void updateMwcMqState(bool online);
    void updateKeybaseState(bool online);
    void updateMwcMqAddress(QString address, int addrIdx);

    void showMessage(QString title, QString message);

private slots:
    void on_mwcMqTriggerButton_clicked();

    void on_mwcMqNextAddress_clicked();

    void on_mwcMqToIndex_clicked();

    void on_keybaseTriggerButton_clicked();

private:


private:
    Ui::Listening *ui;
    state::Listening * state;
};

}

#endif // LISTENINGW_H
