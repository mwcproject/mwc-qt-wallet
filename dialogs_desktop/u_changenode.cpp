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

#include "u_changenode.h"
#include "ui_u_changenode.h"
#include "../control_desktop/messagebox.h"
#include <QHostInfo>
#include <QUrl>
#include <cstring>
#include <QFileDialog>
#include "../util/ioutils.h"
#include <QDir>
#include "../bridge/config_b.h"

namespace dlg {


ChangeNode::ChangeNode(QWidget * parent, const wallet::MwcNodeConnection & _nodeConnection, const QString & network ) :
        control::MwcDialog(parent),
        ui(new Ui::ChangeNode),
        nodeConnection( _nodeConnection )
{
    ui->setupUi(this);

    config = new bridge::Config(this);

    ui->radioCloudNode->setChecked( nodeConnection.connectionType == wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::CLOUD );
    ui->radioEmbeddedNode->setChecked( nodeConnection.connectionType == wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::LOCAL );
    ui->radioCustomNode->setChecked( nodeConnection.connectionType == wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::CUSTOM );

    ui->radioCustomNode->setText( network.toLower().contains("main") ? "Custom mwc-node (for mainnet)" : "Custom mwc-node (for floonet)" );

    if ( nodeConnection.connectionType == wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::CUSTOM )
        ui->customNodeWnd->show();
    else
        ui->customNodeWnd->hide();

    if ( nodeConnection.connectionType == wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::LOCAL )
        ui->embeddedNodeWnd->show();
    else
        ui->embeddedNodeWnd->hide();

    ui->mwcNodeUriEdit->setText( nodeConnection.mwcNodeURI );
    ui->mwcNodeSecretEdit->setText( nodeConnection.mwcNodeSecret );

    ui->nodeDataLocation->setText( nodeConnection.localNodeDataPath );

    ui->noTorCheck->setChecked(config->getNoTorForEmbeddedNode());
}

ChangeNode::~ChangeNode() {
    delete ui;
}

void ChangeNode::on_resetButton_clicked() {
    ui->radioCloudNode->setChecked(true);
    ui->customNodeWnd->hide();
    ui->embeddedNodeWnd->hide();
}

void ChangeNode::on_radioCloudNode_clicked()
{
    ui->customNodeWnd->hide();
    ui->embeddedNodeWnd->hide();
}

void ChangeNode::on_radioEmbeddedNode_clicked()
{
    ui->customNodeWnd->hide();
    ui->embeddedNodeWnd->show();
}

void ChangeNode::on_radioCustomNode_clicked()
{
    ui->customNodeWnd->show();
    ui->embeddedNodeWnd->hide();
}

void ChangeNode::on_selectNodeDataLocationBtn_clicked()
{
    QPair<bool,QString> basePath = ioutils::getAppDataPath();
    if (!basePath.first) {
        control::MessageBox::messageText(nullptr, "Error", basePath.second);
        return;
    }

    QString dir = QFileDialog::getExistingDirectory(
            nullptr,
            "Select your embedded node data folder",
            basePath.second);
    if (dir.isEmpty())
        return;

    auto dirOk = util::validateMwc713Str(dir);
    if (!dirOk.first) {
        core::getWndManager()->messageTextDlg("Directory Name",
                                              "This directory name is not acceptable.\n" + dirOk.second);
        return;
    }

    QDir baseDir(basePath.second);
    QString nodeDir = baseDir.relativeFilePath(dir);

    ui->nodeDataLocation->setText( nodeDir );
}

void ChangeNode::on_cancelButton_clicked() {
    reject();
}

void ChangeNode::on_applyButton_clicked() {

    wallet::MwcNodeConnection resCon = nodeConnection;

    if ( ui->radioCloudNode->isChecked() ) {
        resCon.setAsCloud();
    }
    else if ( ui->radioEmbeddedNode->isChecked() ) {
        QString nodeDataPath = ui->nodeDataLocation->text();
        // Let's create a directory and validate it...
        QPair<bool,QString> fullPath = ioutils::getAppDataPath( nodeDataPath );
        if (!fullPath.first) {
            control::MessageBox::messageText(nullptr, "Error", fullPath.second);
            return;
        }

        QDir d(fullPath.second);

        if (! ( d.exists() || d.mkdir(fullPath.second)) ) {
            control::MessageBox::messageText(this, "Input", "Please specify correct directory for thr node data. Directory\n" + fullPath.second + "\nis not accessible" );
            ui->nodeDataLocation->setFocus();
            return;
        }

        resCon.setAsLocal(nodeDataPath );
    }
    else if ( ui->radioCustomNode->isChecked() ) {
        QString mwcNodeUri = ui->mwcNodeUriEdit->text();
        QString mwcNodeSecret = ui->mwcNodeSecretEdit->text();

        // Validate the input data
        if ( !mwcNodeUri.isEmpty() || !mwcNodeSecret.isEmpty()) {
            if ( mwcNodeUri.isEmpty() ) {
                control::MessageBox::messageText(this, "Input", "Please specify custom mwc-node URI." );
                ui->mwcNodeUriEdit->setFocus();
                return;
            }
            if ( mwcNodeSecret.isEmpty() ) {
                control::MessageBox::messageText(this, "Input", "Please specify custom mwc-node secret." );
                ui->mwcNodeSecretEdit->setFocus();
                return;
            }

            if ( ! (mwcNodeUri.startsWith("http://") || mwcNodeUri.startsWith("https://") ) ) {
                control::MessageBox::messageText(this, "Input", "Please specify http or https protocol for mwc node connection. Please note, https connection does require CA certificate." );
                ui->mwcNodeUriEdit->setFocus();
                return;
            }

            // Remove the port part to verify the host
            int uriPortIdx = mwcNodeUri.indexOf(':', std::strlen("https://") );
            if (uriPortIdx<=0) {
                control::MessageBox::messageText(this, "Input", "Please specify a port for mwc-node connection." );
                ui->mwcNodeUriEdit->setFocus();
                return;
            }
            Q_ASSERT( uriPortIdx>0 );

            bool portok = false;
            mwcNodeUri.mid(uriPortIdx+1).toInt(&portok);
            if (uriPortIdx<=0) {
                control::MessageBox::messageText(this, "Input", "Please specify valid mwc node URI." );
                ui->mwcNodeUriEdit->setFocus();
                return;
            }

            // Checking if URI is reachable...
            QUrl url2test(mwcNodeUri);
            if ( !url2test.isValid() ) {
                control::MessageBox::messageText( this, "Input", "mwc node URL "+mwcNodeUri+" is invalid. Please specify a valid URI for mwc node" );
                ui->mwcNodeUriEdit->setFocus();
                return;
            }

            QString hostName = url2test.host();
            QHostInfo host = QHostInfo::fromName( hostName );
            if (host.error() != QHostInfo::NoError) {

                control::MessageBox::messageText(this, "Input",
                                             "MWC Node host " + hostName + " is not reachable.\n" + host.errorString());
                ui->mwcNodeUriEdit->setFocus();
                return;
            }
        }
        resCon.setAsCustom( mwcNodeUri, mwcNodeSecret );
    }

    bool noTor = ui->noTorCheck->isChecked();

    if ( resCon == nodeConnection && noTor == config->getNoTorForEmbeddedNode() ) {
        reject(); // nothing was changed. It is a reject.
        return;
    }

    if (control::MessageBox::questionText(this, "Update MWC Node connection", "Update of mwc node connection required relogin into the wallet. Than you will be able to verify if your wallet was able to connect to the mwc node.\nWould you like to continue?",
                                      "Yes", "No",
                                      "Apply node connection settings and relogin",
                                      "Cancel this operation",
                                      true, false) != core::WndManager::RETURN_CODE::BTN1 ) {
        return;
    }

    config->setNoTorForEmbeddedNode(noTor);

    nodeConnection = resCon;

    accept();
}



}

