#include "core/mwctoolbar.h"
#include "ui_mwctoolbar.h"
#include <QPainter>
#include <QStyleOption>
#include "appcontext.h"
#include "../state/statemachine.h"

namespace core {

MwcToolbar::MwcToolbar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MwcToolbar)
{
    ui->setupUi(this);

    //setAutoFillBackground(true);
    //setStyleSheet("background-color: #6F00D6;");
}

MwcToolbar::~MwcToolbar()
{
    delete ui;
}

void MwcToolbar::updateButtonsState( state::STATE state ) {
    ui->airdropToolButton->setChecked(state==state::AIRDRDOP_MAIN);
    ui->sendToolButton->setChecked(state==state::SEND_COINS);
    ui->recieveToolButton->setChecked( state==state::RECIEVE_COINS);
    ui->transactionToolButton->setChecked(state==state::TRANSACTIONS);
    ui->hodlToolButton->setChecked(state==state::HODL);
}

void MwcToolbar::paintEvent(QPaintEvent *)
{
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void MwcToolbar::on_airdropToolButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::AIRDRDOP_MAIN );
}

void MwcToolbar::on_sendToolButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::SEND_COINS );
}

void MwcToolbar::on_recieveToolButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::RECIEVE_COINS );

}

void MwcToolbar::on_transactionToolButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::TRANSACTIONS );
}

void MwcToolbar::on_hodlToolButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::HODL );
}


}


