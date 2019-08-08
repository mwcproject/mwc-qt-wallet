#ifndef HODLW_H
#define HODLW_H

#include "../core/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class Hodl;
}

namespace state {
    class Hodl;
}

namespace wnd {

class Hodl : public core::NavWnd
{
    Q_OBJECT

public:
    explicit Hodl(QWidget *parent, state::Hodl * state );
    ~Hodl();

private:
    Ui::Hodl *ui;
    state::Hodl * state;
};

}

#endif // HODL_H
