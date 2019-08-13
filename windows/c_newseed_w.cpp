#include "c_newseed_w.h"
#include "ui_c_newseed.h"
#include "../state/a_initaccount.h"
#include "../util/widgetutils.h"
#include "../control/messagebox.h"
#include "../state/timeoutlock.h"

namespace wnd {


NewSeed::NewSeed( QWidget *parent, state::SubmitCaller * _state, state::StateContext * _context,
                  const QVector<QString> & _seed, bool hideSubmitButton ) :
    core::NavWnd( parent, _context, hideSubmitButton ),
    ui(new Ui::NewSeed),
    state(_state),
    context(_context),
    seed(_seed)
{
    ui->setupUi(this);

    if ( hideSubmitButton )
        ui->submitButton->hide();
    else
        utils::defineDefaultButtonSlot(this, SLOT(on_submitButton_clicked()) );

    if (!seed.isEmpty()) {
        updateSeedData("Seed:",seed); // Seed allways comes from ctor. Mean we are created a new account with a seed.
    }

}

NewSeed::~NewSeed()
{
    state->wndDeleted(this);
    delete ui;
}

// if seed empty or has size 1, it is error message
void NewSeed::showSeedData(const QVector<QString> & seed) {
    state::TimeoutLockObject to( context->stateMachine );

    if (seed.size()<2) {
        control::MessageBox::message( this, "Getting Seed Failure", "Unable to retrieve a seed from mwc713." + (seed.size()>0 ? seed[0] : "") );
        return;
    }

    // By update allways come mnemonic. It is mean that we read the seed from the wallet. That is why it has different name.
    updateSeedData("Mnemonic:", seed);
}

void NewSeed::updateSeedData( const QString & name, const QVector<QString> & seed) {
    QString thePhrase = "";
    for (const auto & s : seed) {
        if (thePhrase.length()>0)
            thePhrase+=" ";
        thePhrase+=s;
    }
    ui->seedText->setPlainText( name + "\n" + thePhrase);
    ui->seedText->setFocus();
}

void NewSeed::on_submitButton_clicked()
{
    state->submit();
}

}
