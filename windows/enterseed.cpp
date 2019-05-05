#include "windows/enterseed.h"
#include "ui_enterseed.h"
#include "../util/stringutils.h"
#include <QMessageBox>
#include "../state/createwithseed.h"

namespace wnd {

EnterSeed::EnterSeed(QWidget *parent, state::CreateWithSeed * _state) :
    QWidget(parent),
    ui(new Ui::EnterSeed),
    state(_state)
{
    ui->setupUi(this);
}

EnterSeed::~EnterSeed()
{
    delete ui;
}

void EnterSeed::on_submitButton_clicked()
{
    QVector<QString> seed = util::parsePhrase2Words( ui->seedTextEdit->toPlainText().toLower() );

    if (seed.size()!=24) {
        QMessageBox::warning(this, "Seed verification",
                             "Your phrase should contain 24 words. You enter " + QString::number(seed.size()) +  " words." );
        return;
    }

    QPair<bool, QString> res = state->createWalletWithSeed(seed);
    if (!res.first) {
        QMessageBox::warning(this, "Seed verification",
                             "We unable to recover your wallet from your seed phrase.\nReason: " + res.second );
        return;
    }
}


}
