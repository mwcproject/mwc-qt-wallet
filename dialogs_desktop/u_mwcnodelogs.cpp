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

#include "dialogs_desktop/u_mwcnodelogs.h"
#include "ui_u_mwcnodelogs.h"
#include <QScrollBar>
#include "../bridge/node_b.h"

namespace dlg {

MwcNodeLogs::MwcNodeLogs(QWidget *parent) :
    control::MwcDialog(parent),
    ui(new Ui::MwcNodeLogs)
{
    ui->setupUi(this);

    node = new bridge::Node(this);

    ui->fullLogsLink->setText("mwc-node logs location: " + node->getLogsLocation() );

    ui->logsEdit->setPlainText( node->getOutputLines().join("\n") );

    QObject::connect(node, &bridge::Node::sgnMwcOutputLine,
                     this, &MwcNodeLogs::onMwcOutputLine, Qt::QueuedConnection);
}

MwcNodeLogs::~MwcNodeLogs()
{
    delete ui;
}

void MwcNodeLogs::on_okButton_clicked()
{
    accept();
}

void MwcNodeLogs::onMwcOutputLine(QString line) {
    Q_UNUSED(line)

    QScrollBar * vSB = ui->logsEdit->verticalScrollBar();
    if (vSB) {
        if (vSB->value() > 3)
            return;
    }

    ui->logsEdit->setPlainText( node->getOutputLines().join("\n") );
}


}
