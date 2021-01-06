// Copyright 2021 The MWC Developers
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

#ifndef G_DECODESLATEPACKDLG_H
#define G_DECODESLATEPACKDLG_H

#include "../control_desktop/mwcdialog.h"

namespace bridge {
class Wallet;
class Util;
}

namespace Ui {
class DecodeSlatepackDlg;
}

namespace dlg {

class DecodeSlatepackDlg : public control::MwcDialog {
Q_OBJECT

public:
    explicit DecodeSlatepackDlg(QWidget *parent = nullptr);

    ~DecodeSlatepackDlg();

protected:
    void resetData();
    void decodeSlate(const QString &slatepack);

private slots:
    void on_slatepackEdit_textChanged();

    void onSgnDecodeSlatepack(QString error, QString slatepack, QString slateJSon, QString content, QString sender, QString recipient);

    void on_closeButton_clicked();

private:
    Ui::DecodeSlatepackDlg *ui;
    bridge::Wallet  *wallet = nullptr;
    bridge::Util    *util = nullptr;

    QString spInProgress;
};

}

#endif // G_DECODESLATEPACKDLG_H
