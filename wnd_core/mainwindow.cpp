#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTime>
#include "versiondialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->statusBar->addPermanentWidget(ui->currentTime);
    ui->statusBar->showMessage("Can show any message here");

    clockTimer = new QTimer(this);
    connect(clockTimer, SIGNAL(timeout()), this, SLOT(updateClock()) );
    clockTimer->start(1000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QWidget * MainWindow::getMainWindow() {
    return ui->mainWindowFrame;
}


void MainWindow::updateClock() {
    ui->currentTime->setText( QTime::currentTime().toString("hh:mm:ss") );
}

void MainWindow::on_actionVersion_triggered()
{
    VersionDialog * verDlg = new VersionDialog(this);
    verDlg->setModal(true);
    verDlg->exec();
}

void MainWindow::on_nextBtn_clicked()
{
    emit processNextStep( WalletWindowAction::NEXT );
}

void MainWindow::on_cancelBtn_clicked()
{
    emit processNextStep( WalletWindowAction::CANCEL );
}
