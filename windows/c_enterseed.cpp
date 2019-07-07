#include "c_enterseed.h"
#include "ui_c_enterseed.h"
#include "../util/stringutils.h"
#include "../control/messagebox.h"
#include "../state/c_createwithseed.h"
#include "../util/widgetutils.h"
#include "../util/Dictionary.h"

namespace wnd {

EnterSeed::EnterSeed(QWidget *parent, state::CreateWithSeed * _state) :
    QWidget(parent),
    ui(new Ui::EnterSeed),
    state(_state)
{
    state->setWindowTitle("Recover wallet from a Passphrase");

    ui->setupUi(this);

    ui->progress->initLoader(false);

    ui->seedText->setReadOnly(false);
    ui->seedText->setAcceptRichText(false);
    ui->seedText->setFocus(Qt::OtherFocusReason);

    utils::defineDefaultButtonSlot(this, SLOT(on_Enter()) );
}

EnterSeed::~EnterSeed()
{
    state->deleteEnterSeed();
    delete ui;
}

void EnterSeed::on_Enter() {
    if ( ui->seedText->hasFocus() )
        return;
    on_continueButton_clicked();
}

void EnterSeed::updateProgress(bool show) {
    if (show)
        ui->progress->show();
    else
        ui->progress->hide();
}


void EnterSeed::on_cancelButton_clicked()
{
    state->cancel();
}

void EnterSeed::on_continueButton_clicked()
{
    QString seedStr = ui->seedText->toPlainText().toLower().trimmed();

    QPair <bool, QString> valRes = util::validateMwc713Str( seedStr );
    if (!valRes.first) {
        control::MessageBox::message(this, "Verification error", valRes.second );
        return;
    }

    QVector<QString> seed = util::parsePhrase2Words( seedStr );

    if (seed.size()!=24) {
        control::MessageBox::message(this, "Verification error",
                             "Your phrase should contain 24 words. You entered " + QString::number(seed.size()) +  " words." );
        return;
    }

    const QSet<QString> & words = util::getBip39words();
    QString nonDictWord;

    for ( auto & s : seed ) {
        if ( !words.contains(s) ) {
            if (!nonDictWord.isEmpty())
                nonDictWord += ", ";
            nonDictWord += s;
        }
    }

    if (!nonDictWord.isEmpty()) {
        control::MessageBox::message(this, "Verification error",
                                     "Your phrase contains non dictionary words: " + nonDictWord );
        return;
    }

    state->createWalletWithSeed(seed);
}

}
