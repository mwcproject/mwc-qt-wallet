#ifndef RECEIVECOINSW_H
#define RECEIVECOINSW_H

#include <QWidget>

namespace Ui {
class ReceiveCoins;
}

namespace state {
    class Listening;
    class FileTransactions;
    class Transactions;
}

namespace wnd {

class ReceiveCoins : public QWidget
{
    Q_OBJECT

public:
    explicit ReceiveCoins(QWidget *parent,
                          state::Listening * listeningState,
                          state::FileTransactions * fileTransactionsState,
                          state::Transactions * transactionsState);
    ~ReceiveCoins();

private slots:
    //void on_mwcBoxConnectBtn_clicked();

    void on_processResponceFileButton_clicked();

    void on_refreshButton_clicked();

private:
    void updateBoxState();

    void initTableHeaders();
    void saveTableHeaders();
    void updateTransactionTable();
private:
    Ui::ReceiveCoins *ui;
    state::Listening * listeningState;
    state::FileTransactions * fileTransactionsState;
    state::Transactions * transactionsState;
};

}


#endif // RECEIVECOINSW_H
