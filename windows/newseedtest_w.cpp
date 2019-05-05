#include "newseedtest_w.h"
#include "ui_newseedtest.h"
#include "../state/newseedtest.h"

namespace wnd {

NewSeedTest::NewSeedTest(QWidget *parent, state::NewSeedTest *_state, int wordNumber) :
    QWidget(parent),
    ui(new Ui::NewSeedTest),
    state(_state)
{
    ui->setupUi(this);

    ui->wordLabelHi->setText("<b>Please Confirm word number " +
                             QString::number(wordNumber) + " for your passphrase</b>");
    ui->wordLabelLo->setText("<b>Word number " + QString::number(wordNumber) + "</b>");
}

NewSeedTest::~NewSeedTest()
{
    delete ui;
}


void NewSeedTest::on_submitButton_clicked()
{
    state->submit( ui->wordEdit->text() );
}

}
