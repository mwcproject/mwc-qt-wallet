#include "mwc_windows/fromseedfile.h"
#include "ui_fromseedfile.h"
#include <QFileDialog>
#include <QMessageBox>

FromSeedFile::FromSeedFile(WalletWindowType wwType, QWidget *parent) :
    WalletWindow ( wwType, parent),
    ui(new Ui::FromSeedFile)
{
    ui->setupUi(this);
}

FromSeedFile::~FromSeedFile()
{
    delete ui;
}

void FromSeedFile::on_browseButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select your wallet"),
                                                            QDir::homePath(),
                                                            tr("MWC Wallet (*.seed)"));

    if (fileName.length()>0) {
        ui->seedFile->setText( fileName );
    }
}

bool FromSeedFile::validateData() {
    seedFileName = ui->seedFile->text();
    seedPassword = ui->seedPassword->text();

    if (seedFileName.length()==0) {
        QMessageBox::critical(this, "Init from Seed File", "Please specify the wallet seed file name");
        return false;
    }

    return true;
}
