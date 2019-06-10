#include "c_newseed_w.h"
#include "ui_c_newseed.h"
#include "../state/c_newseedshow.h"
#include "../util/widgetutils.h"

namespace wnd {


NewSeed::NewSeed( QWidget *parent, state::NewSeedShow * _state, const QVector<QString> & _seed) :
    QWidget( parent),
    ui(new Ui::NewSeed),
    state(_state),
    seed(_seed)
{
    ui->setupUi(this);

    state->setWindowTitle( "A new Seed for your wallet" );

    QString thePhrase = "";
    for (const auto & s : seed) {
        if (thePhrase.length()>0)
            thePhrase+=" ";
        thePhrase+=s;
    }

    ui->seedText->setPlainText( "Seed:\n" + thePhrase);

    ui->seedText->setFocus();

    utils::defineDefaultButtonSlot(this, SLOT(on_submitButton_clicked()) );
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
