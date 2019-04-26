#ifndef NODEMANUALLY_H
#define NODEMANUALLY_H

#include "../wnd_core/walletwindow.h"

namespace Ui {
class NodeManually;
}

class NodeConnection;

class NodeManually : public WalletWindow
{
    Q_OBJECT

public:
    explicit NodeManually(WalletWindowType appType, QWidget *parent,
                          NodeConnection * nodeConnection );
    virtual ~NodeManually() override;

    virtual bool validateData() override;

private slots:
    void on_testConnectionButton_clicked();

private:
    Ui::NodeManually *ui;
    NodeConnection * data;
};

#endif // NODEMANUALLY_H
