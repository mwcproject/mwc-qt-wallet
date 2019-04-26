#include "confirmseed.h"
#include "ui_confirmseed.h"
#include <QMessageBox>
#include "../data/stringutils.h"

ConfirmSeed::ConfirmSeed(WalletWindowType wwType, QWidget *parent, QVector<QString> seedWords) :
    WalletWindow(wwType, parent),
    ui(new Ui::ConfirmSeed),
    seed2check(seedWords)
{
    ui->setupUi(this);
}

ConfirmSeed::~ConfirmSeed()
{
    delete ui;
}

bool ConfirmSeed::validateData()
{
    QVector<QString> inputSeed = parsePhrase2Words( ui->seedTextEdit->toPlainText().toLower() );

    // Compare the seeds
    if ( inputSeed.size()< seed2check.size() ) {
        QMessageBox::warning(this, "Seed verification",
                             "Your input has less words then was provided in the seed" );
        return false;
    }

    if ( inputSeed.size() > seed2check.size() ) {
        QMessageBox::warning(this, "Seed verification",
                             "Your input has more words then was provided in the seed" );
        return false;
    }

    Q_ASSERT( inputSeed.size() == seed2check.size() );
    for ( int t=0; t<inputSeed.size(); t++ ) {
        if (inputSeed[t] != seed2check[t]) {
            QMessageBox::warning(this, "Seed verification",
                                 "Your word " + QString::number(t+1) +  " is incorrect" );
            return false;
        }
    }

    return true;
}
