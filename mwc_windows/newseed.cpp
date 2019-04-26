#include "newseed.h"
#include "ui_newseed.h"
#include <QMessageBox>

NewSeed::NewSeed( WalletWindowType wwType, QWidget *parent, const QVector<QString> & s) :
    WalletWindow( wwType, parent),
    ui(new Ui::NewSeed),
    seed(s)
{
    ui->setupUi(this);

    QString thePhrase = "";
    for (const auto & s : seed) {
        if (thePhrase.length()>0)
            thePhrase+=" ";
        thePhrase+=s;
    }

    ui->plainTextEdit->setPlainText(thePhrase);
}

NewSeed::~NewSeed()
{
    delete ui;
}

bool NewSeed::validateData()
{
    return true;
}
