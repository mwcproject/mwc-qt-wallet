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

#include "windows/h_hodlnode_w.h"
#include "ui_h_hodlnode_w.h"
#include "../state/h_hodl.h"
#include "../control/messagebox.h"
#include "../state/timeoutlock.h"
#include "../util/execute.h"
#include "../util/crypto.h"
#include "../core/HodlStatus.h"

namespace wnd {

HodlNode::HodlNode(QWidget *parent, state::Hodl * _state) :
    core::NavWnd(parent, _state->getContext()),
    ui(new Ui::HodlNode),
    state(_state)
{
    ui->setupUi(this);
    ui->progress->initLoader(false);

    ui->accountStatus->setText("");

    updateHodlState();
}

HodlNode::~HodlNode()
{
    state->deleteHodlNodeWnd(this);
    delete ui;
}

// Hodl object changed it's state, need to refresh
void HodlNode::updateHodlState() {
    ui->hodlStatus->setText( state->getContext()->hodlStatus->getHodlStatus() );

    QString pubKey = state->getContext()->hodlStatus->getRootPubKey();

    if (pubKey != ui->publicKey->text()) {
        ui->signInButton->setEnabled(false);
        ui->accountStatus->setText("");
        ui->viewOutputsButton->setEnabled(false);
        return;
    }

    ui->signInButton->setEnabled( !state->getContext()->hodlStatus->isInHodl("") &&
               pubKey.length()>0 && crypto::isPublicKeyValid(pubKey) );
    ui->accountStatus->setText( state->getContext()->hodlStatus->getWalletHodlStatus("").first );
    ui->viewOutputsButton->setEnabled(state->getContext()->hodlStatus->hasHodlOutputs() && !state->getContext()->hodlStatus->getHodlOutputs("").isEmpty() );
}

void HodlNode::reportMessage(const QString & title, const QString & message) {
    state::TimeoutLockObject to( state );
    ui->progress->hide();

    control::MessageBox::messageText(this, title, message);
}

void HodlNode::on_signInButton_clicked()
{
    ui->progress->show();
    state->registerAccountForHODL();
}

void HodlNode::on_viewOutputsButton_clicked()
{
    QVector<core::HodlOutputInfo> outputs = state->getContext()->hodlStatus->getHodlOutputs("");

 /*
    Outputs that was discovered by HODL server during last scan:<br>
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
      QString result = "Outputs that was discovered by HODL server during last scan:<br>"
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
    if ( crypto::isPublicKeyValid(pubKey) ) {
        state->setColdWalletPublicKey(pubKey);
    }
    else {
        state->setColdWalletPublicKey("");
    }

    updateHodlState();
}

void HodlNode::hideWaitingStatus() {
    ui->progress->hide();
}



}

