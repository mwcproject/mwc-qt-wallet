#include "mwc_windows/enterseed.h"
#include "ui_enterseed.h"
#include "../data/stringutils.h"
#include <QMessageBox>

EnterSeed::EnterSeed(WalletWindowType appType, QWidget *parent) :
    WalletWindow(appType, parent),
    ui(new Ui::EnterSeed)
{
    ui->setupUi(this);
}

EnterSeed::~EnterSeed()
{
    delete ui;
}

bool EnterSeed::validateData() {
    seed = parsePhrase2Words( ui->seedTextEdit->toPlainText().toLower() );

    if (seed.size()!=24) {
        QMessageBox::warning(this, "Seed verification",
                             "Your phrase should contain 24 words. You enter " + QString::number(seed.size()) +  " words." );
        return false;
    }

    return true;
}


