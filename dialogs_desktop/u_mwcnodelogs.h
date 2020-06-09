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

#ifndef U_MWCNODELOGS_H
#define U_MWCNODELOGS_H

#include <QDialog>
#include "../control_desktop/mwcdialog.h"

namespace Ui {
class MwcNodeLogs;
}

namespace bridge {
class Node;
}

namespace dlg {

class MwcNodeLogs : public control::MwcDialog
{
    Q_OBJECT

public:
    explicit MwcNodeLogs(QWidget *parent );
    ~MwcNodeLogs();

private slots:
    void on_okButton_clicked();
    void onMwcOutputLine(QString line);

private:
    Ui::MwcNodeLogs *ui;
    bridge::Node * node = nullptr;
};

}

#endif // U_MWCNODELOGS_H
