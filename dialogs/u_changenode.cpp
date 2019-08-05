#include "dialogs/u_changenode.h"
#include "ui_u_changenode.h"
#include "../control/messagebox.h"
#include <QHostInfo>

namespace dlg {

static const QString MWC_NODE_URI_DEFAULT = "Provided by mwc wallet";
static const QString MWC_NODE_SECRET_DEFAULT = "Provided by mwc wallet";

static const int     MWC_NODE_DEFAULT_PORT = 13415;


ChangeNode::ChangeNode(QWidget * parent, const wallet::WalletConfig & _config ) :
        control::MwcDialog(parent),
        ui(new Ui::ChangeNode),
        config(_config)
{
    ui->setupUi(this);

    if (config.mwcNodeURI.isEmpty() || config.mwcNodeSecret.isEmpty() ) {
        ui->mwcNodeUriEdit->setText(MWC_NODE_URI_DEFAULT);
        ui->mwcNodeSecretEdit->setText(MWC_NODE_SECRET_DEFAULT);
    }
    else {
        ui->mwcNodeUriEdit->setText( config.mwcNodeURI );
        ui->mwcNodeSecretEdit->setText( config.mwcNodeSecret );
    }
}

ChangeNode::~ChangeNode() {
    delete ui;
}

void ChangeNode::on_resetButton_clicked() {
    ui->mwcNodeUriEdit->setText(MWC_NODE_URI_DEFAULT);
    ui->mwcNodeSecretEdit->setText(MWC_NODE_SECRET_DEFAULT);
}

void ChangeNode::on_cancelButton_clicked() {
    reject();
}

void ChangeNode::on_applyButton_clicked() {

    // Check parameters first
    QString mwcNodeUri = ui->mwcNodeUriEdit->text();
    QString mwcNodeSecret = ui->mwcNodeSecretEdit->text();

    if ( mwcNodeUri == MWC_NODE_URI_DEFAULT )
        mwcNodeUri = "";

    if ( mwcNodeSecret == MWC_NODE_SECRET_DEFAULT )
        mwcNodeSecret = "";

    if ( config.mwcNodeURI == mwcNodeUri && config.mwcNodeSecret == mwcNodeSecret ) {
        reject(); // nothing was changed. It is a reject.
        return;
    }

    // Validate the input data
    if ( !mwcNodeUri.isEmpty() || !mwcNodeSecret.isEmpty()) {
        if ( mwcNodeUri.isEmpty() ) {
            control::MessageBox::message(this, "Input", "Please specify mwc node URI or reset all values." );
            ui->mwcNodeUriEdit->setFocus();
            return;
        }
        if ( mwcNodeSecret.isEmpty() ) {
            control::MessageBox::message(this, "Input", "Please specify mwc node secret or reset all values." );
            ui->mwcNodeSecretEdit->setFocus();
            return;
        }

        // Remove the port part to verify the host
        int uriPortIdx = mwcNodeUri.indexOf(':');
        if (uriPortIdx<=0) {
            mwcNodeUri += ":" + QString::number(MWC_NODE_DEFAULT_PORT);
            uriPortIdx = mwcNodeUri.indexOf(':');
        }
        Q_ASSERT( uriPortIdx>0 );

        bool portok = false;
        mwcNodeUri.mid(uriPortIdx+1).toInt(&portok);
        if (uriPortIdx<=0) {
            control::MessageBox::message(this, "Input", "Please specify valid mwc node URI or reset all values." );
            ui->mwcNodeUriEdit->setFocus();
            return;
        }

        // Checking if URI is reachable...
        QHostInfo host = QHostInfo::fromName(mwcNodeUri.left(uriPortIdx));
        if (host.error() != QHostInfo::NoError) {
            control::MessageBox::message( this, "Input", "mwc node host "+mwcNodeUri+" is not reachable.\n" + host.errorString() );
            ui->mwcNodeUriEdit->setFocus();
            return;
        }
    }

    if (control::MessageBox::question(this, "Update mwc node connection", "Update of mwc node connection required relogin into the wallet. Than you will be able to verify if your wallet was able to connect to the mwc node.\nWould you like to continue?",
                                      "Yes", "No", true, false) != control::MessageBox::BTN1 ) {
        return;
    }

    config.mwcNodeURI = mwcNodeUri;
    config.mwcNodeSecret = mwcNodeSecret;

    accept();
}

}
