#include "claimchallendge.h"
#include "ui_claimchallendge.h"

namespace wnd {

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

}

