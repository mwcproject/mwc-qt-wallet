#ifndef CLAIMCHALLENDGE_H
#define CLAIMCHALLENDGE_H

#include <QWidget>

namespace Ui {
class ClaimChallendge;
}

namespace wnd {

class ClaimChallendge : public QWidget
{
    Q_OBJECT

public:
    explicit ClaimChallendge(QWidget *parent = nullptr);
    ~ClaimChallendge();

private:
    Ui::ClaimChallendge *ui;
};

}

#endif // CLAIMCHALLENDGE_H
