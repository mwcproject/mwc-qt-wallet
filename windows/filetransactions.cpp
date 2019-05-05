#include "filetransactions.h"
#include "ui_filetransactions.h"

namespace wnd {

FileTransactions::FileTransactions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileTransactions)
{
    ui->setupUi(this);
}

FileTransactions::~FileTransactions()
{
    delete ui;
}

}

