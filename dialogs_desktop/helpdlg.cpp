// Copyright 2019 The MWC Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "dialogs/helpdlg.h"
#include "ui_helpdlg.h"

namespace dlg {

// documentName - doc name to display:  XXXXXX.html. Expected to be found at resources qrc:/help/XXXXXX.html
HelpDlg::HelpDlg( QWidget *parent, QString documentName ) :
    control::MwcDialog(parent),
    ui(new Ui::HelpDlg)
{
    ui->setupUi(this);
    ui->browser->setSource( QUrl("qrc:/help/" + documentName) );

    connect( ui->browser, &QTextBrowser::backwardAvailable, this, &HelpDlg::on_backwardAvailable );
    connect( ui->browser, &QTextBrowser::forwardAvailable, this, &HelpDlg::on_forwardAvailable );

    ui->forwardBtn->setEnabled(false);
    ui->backwardBtn->setEnabled(false);

    // In any case we don't have any pages with navigation so far
    ui->forwardBtn->hide();
    ui->backwardBtn->hide();
}

HelpDlg::~HelpDlg()
{
    delete ui;
}

void HelpDlg::on_doneButton_clicked()
{
    accept();
}

void HelpDlg::on_backwardBtn_clicked()
{
    ui->browser->backward();
}

void HelpDlg::on_forwardBtn_clicked()
{
    ui->browser->forward();
}

void HelpDlg::on_backwardAvailable(bool available) {
    ui->backwardBtn->setEnabled(available);
}
void HelpDlg::on_forwardAvailable(bool available) {
    ui->forwardBtn->setEnabled(available);
}

}

