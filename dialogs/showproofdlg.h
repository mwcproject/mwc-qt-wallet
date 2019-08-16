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

#ifndef SHOWPROOFDLG_H
#define SHOWPROOFDLG_H

#include "../control/mwcdialog.h"

namespace Ui {
class ShowProofDlg;
}

namespace dlg {

struct ProofInfo {
    QString mwc;
    QString toAdress;
    QString fromAdress;

    QString output;
    QString kernel;

    // Parse the data and fill the structure.
    bool parseProofText(const QString & proof);
};

class ShowProofDlg : public control::MwcDialog {
Q_OBJECT

public:
    explicit ShowProofDlg(QWidget *parent, const QString &fileName, const ProofInfo & proofInfo);

    ~ShowProofDlg();

private slots:
    void on_viewOutput_clicked();

    void on_viewKernel_clicked();

    void on_pushButton_clicked();

private:
    Ui::ShowProofDlg *ui;
    ProofInfo proof;
};

}

#endif // SHOWPROOFDLG_H
