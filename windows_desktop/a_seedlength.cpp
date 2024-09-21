#include "a_seedlength.h"
#include "ui_a_seedlength.h"
#include "../bridge/wnd/a_initaccount_b.h"

namespace wnd {

SeedLength::SeedLength(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::SeedLength) {
    ui->setupUi(this);

    initAccount = new bridge::InitAccount(this);
}

SeedLength::~SeedLength() {
    delete ui;
}

void SeedLength::on_continueButton_pressed() {
    int length = 24;
    if (ui->radioLength12->isChecked())
        length = 12;
    else if (ui->radioLength15->isChecked())
        length = 15;
    else if (ui->radioLength18->isChecked())
        length = 18;
    else if (ui->radioLength21->isChecked())
        length = 21;
    else
        length = 24;

    initAccount->submitSeedLength(length);
}

}
