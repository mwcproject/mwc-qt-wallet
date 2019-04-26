#include "newwallet.h"
#include "ui_newwallet.h"

NewWallet::NewWallet(WalletWindowType appType, QWidget *parent) :
    WalletWindow(appType, parent),
    ui(new Ui::NewWallet)
{
    ui->setupUi(this);

    ui->radioCreateNew->setChecked(true);
}

NewWallet::~NewWallet()
{
    delete ui;
}

bool NewWallet::validateData() {
    if ( ui->radioCreateNew->isChecked() )
        choice = NEW_SEED;
    else if (ui->radioHaveSeed->isChecked())
        choice = HAVE_SEED;
    else if (ui->radioSeedFile->isChecked())
        choice = SEED_FILE;
    else
        return false;

    return true;
}

