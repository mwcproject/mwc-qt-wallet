#include "mwc_windows/claimchallendge.h"
#include "ui_claimchallendge.h"

ClaimChallendge::ClaimChallendge(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClaimChallendge)
{
    ui->setupUi(this);
}

ClaimChallendge::~ClaimChallendge()
{
    delete ui;
}
