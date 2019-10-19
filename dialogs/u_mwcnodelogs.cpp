#include "dialogs/u_mwcnodelogs.h"
#include "ui_u_mwcnodelogs.h"
#include <QScrollBar>

namespace dlg {

MwcNodeLogs::MwcNodeLogs(QWidget *parent, node::MwcNode * _node) :
    control::MwcDialog(parent),
    ui(new Ui::MwcNodeLogs),
    node(_node)
{
    ui->setupUi(this);

    ui->fullLogsLink->setText("mwc-node logs location: " + node->getLogsLocation() );

    ui->logsEdit->setPlainText( node->getOutputLines().join("\n") );

    QObject::connect(node, &node::MwcNode::onMwcOutputLine,
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
