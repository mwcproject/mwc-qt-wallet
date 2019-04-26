#include "choosewallet.h"
#include "ui_choosewallet.h"
#include <QFileDialog>
#include <QMessageBox>

ChooseWallet::ChooseWallet(WalletWindowType appType, QWidget *parent) :
    WalletWindow(appType, parent),
    ui(new Ui::ChooseWallet)
{
    ui->setupUi(this);
}

ChooseWallet::~ChooseWallet()
{
    delete ui;
}

void ChooseWallet::on_pushButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select your wallet"),
                                                 QDir::homePath() + "/.mwc/wallets",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);

    if (dir.length()>0)
        ui->walletPath->setText(dir);
}

bool ChooseWallet::validateData() {

    walletPath = ui->walletPath->text();

    if (walletPath.length()>0)
        return true;

    QMessageBox::critical(this, "Wallet path", "Please define the folder where we will keep the wallet files");
    return false;
}

