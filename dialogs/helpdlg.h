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

#ifndef HELPDLG_H
#define HELPDLG_H

#include <QDialog>
#include "../control/mwcdialog.h"

namespace Ui {
class HelpDlg;
}

namespace dlg {

class HelpDlg : public control::MwcDialog
{
    Q_OBJECT

public:
    // documentName - doc name to display:  XXXXXX.html. Expected to be found at resources qrc:/help/XXXXXX.html
    explicit HelpDlg( QWidget *parent, QString documentName );
    ~HelpDlg();

private slots:
    void on_backwardBtn_clicked();
    void on_forwardBtn_clicked();
    void on_doneButton_clicked();

    void on_backwardAvailable(bool available);
    void on_forwardAvailable(bool available);

private:
    Ui::HelpDlg *ui;
};

}

#endif // HELPDLG_H
