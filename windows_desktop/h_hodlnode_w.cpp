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

#include "h_hodlnode_w.h"
#include "ui_h_hodlnode_w.h"
#include "../control_desktop/messagebox.h"
#include "../util_desktop/timeoutlock.h"
#include "../bridge/wnd/h_hodl_b.h"
#include "../bridge/hodlstatus_b.h"
#include "../bridge/util_b.h"
#include "../core/HodlStatus.h"

namespace wnd {

HodlNode::HodlNode(QWidget *parent) :
    core::NavWnd(parent),
    ui(new Ui::HodlNode)
{
    ui->setupUi(this);

    hodl = new bridge::Hodl(this);
    hodlStatus = new bridge::HodlStatus(this);
    util = new bridge::Util(this);

    QObject::connect( hodl, &bridge::Hodl::sgnUpdateHodlState,
                      this, &HodlNode::onSgnUpdateHodlState, Qt::QueuedConnection);
    QObject::connect( hodl, &bridge::Hodl::sgnReportMessage,
                      this, &HodlNode::onSgnReportMessage, Qt::QueuedConnection);
    QObject::connect( hodl, &bridge::Hodl::sgnHideWaitingStatus,
                      this, &HodlNode::onSgnHideWaitingStatus, Qt::QueuedConnection);

    ui->progress->initLoader(false);

    ui->accountStatus->setText("");

    onSgnUpdateHodlState();
}

HodlNode::~HodlNode()
{
    delete ui;
}

// Hodl object changed it's state, need to refresh
void HodlNode::onSgnUpdateHodlState() {
    ui->hodlStatus->setText( hodlStatus->getHodlStatus() );

    QString pubKey = hodlStatus->getRootPubKey();

    if (pubKey != ui->publicKey->text()) {
        ui->signInButton->setEnabled(false);
        ui->accountStatus->setText("");
        ui->viewOutputsButton->setEnabled(false);
        return;
    }

    ui->signInButton->setEnabled( !hodlStatus->isInHodl("") &&
               pubKey.length()>0 && util->isPublicKeyValid(pubKey) );
    ui->accountStatus->setText( hodlStatus->getWalletHodlStatus("")[0] );
    ui->viewOutputsButton->setEnabled( hodlStatus->hasHodlOutputs() && !hodlStatus->getHodlOutputs("").isEmpty() );
}

void HodlNode::onSgnReportMessage( QString title, QString message) {
    util::TimeoutLockObject to("HodlNode");
    ui->progress->hide();

    control::MessageBox::messageText(this, title, message);
}

void HodlNode::on_signInButton_clicked()
{
    ui->progress->show();
    hodl->registerAccountForHODL();
}

void HodlNode::on_viewOutputsButton_clicked()
{
    QVector<QString> jsons = hodlStatus->getHodlOutputs("");
    QVector<core::HodlOutputInfo> outputs;
    for (auto & j : jsons)
        outputs.push_back( core::HodlOutputInfo::fromJson(j) );

 /*
    Outputs that were discovered by HODL server during last scan:<br>
  <table>
    <tr>
      <th>Commitment</th>
      <th></th>
      <th>MWC</th>
      <th>&nbsp;&nbsp;&nbsp;</th>
    </tr>
    <tr>
      <td>089e22b5084906e5da629272d487eaeb67138366faf28865fcde7599d5ed5410af</td>
      <td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>
      <td>0.154</td>
      <th></th>
    </tr>
  </table>
  */

      int limit = 50;
      QString result = "Outputs that were discovered by HODL server during last scan:<br>"
                       "<table>"
                       "  <tr>"
                       "    <th>Commitment</th>"
                       "    <th></th>"
                       "    <th>MWC</th>"
                       "    <th>&nbsp;&nbsp;&nbsp;</th>"
                       "  </tr>"
                       "  <tr>";

      for (const core::HodlOutputInfo & outpt : outputs) {
          limit--;
          if (limit==0) {
              result += "<tr><td> .......... </td><td></td><td> ... </td></tr>";
              break;
          }
          else {
              result += "<tr><td>" + outpt.outputCommitment + "</td><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td><td>" + util::trimStrAsDouble( QString::number(outpt.value), 6) +
                        "</td><td></td></tr>";
          }

      }
      result += "</table>";

      control::MessageBox::messageHTML(this, "HODL Outputs", result, 1.3);
}

void HodlNode::on_publicKey_textChanged(const QString &pubKey)
{
    if ( util->isPublicKeyValid(pubKey) ) {
        hodl->setColdWalletPublicKey(pubKey);
    }
    else {
        hodl->setColdWalletPublicKey("");
    }

    onSgnUpdateHodlState();
}

void HodlNode::onSgnHideWaitingStatus() {
    ui->progress->hide();
}



}

