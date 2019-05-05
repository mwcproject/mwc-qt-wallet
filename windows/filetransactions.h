#ifndef FILETRANSACTIONS_H
#define FILETRANSACTIONS_H

#include <QWidget>

namespace Ui {
class FileTransactions;
}

namespace wnd {

class FileTransactions : public QWidget
{
    Q_OBJECT

public:
    explicit FileTransactions(QWidget *parent = nullptr);
    ~FileTransactions();

private:
    Ui::FileTransactions *ui;
};

}

#endif // FILETRANSACTIONS_H
