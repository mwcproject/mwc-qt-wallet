#include "newseed_w.h"
#include "ui_newseed.h"
#include <QMessageBox>
#include "../state/newseedshow.h"

namespace wnd {


NewSeed::NewSeed( QWidget *parent, state::NewSeedShow * _state, const QVector<QString> & _seed) :
    QWidget( parent),
    ui(new Ui::NewSeed),
    state(_state),
    seed(_seed)
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

void NewSeed::on_submitButton_clicked()
{
    state->submit();
}

}
