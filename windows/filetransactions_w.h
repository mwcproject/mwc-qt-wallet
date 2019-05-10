#ifndef FILETRANSACTIONSW_H
#define FILETRANSACTIONSW_H

#include <QWidget>

namespace Ui {
class FileTransactions;
}

namespace state {
    class FileTransactions;
}

namespace wnd {

class FileTransactions : public QWidget
{
    Q_OBJECT

public:
    explicit FileTransactions(QWidget *parent, state::FileTransactions * state);
    ~FileTransactions();

private slots:
    void on_generateSendCoinsFileButton_clicked();

    void on_signTransactionButton_clicked();

    void on_processResponceFileButton_clicked();

private:
    Ui::FileTransactions *ui;
    state::FileTransactions * state;
};

}

#endif // FILETRANSACTIONS_H
