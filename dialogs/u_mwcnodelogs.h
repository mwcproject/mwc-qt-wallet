#ifndef U_MWCNODELOGS_H
#define U_MWCNODELOGS_H

#include <QDialog>
#include "../node/MwcNode.h"
#include "../control/mwcdialog.h"

namespace Ui {
class MwcNodeLogs;
}

namespace dlg {

class MwcNodeLogs : public control::MwcDialog
{
    Q_OBJECT

public:
    explicit MwcNodeLogs(QWidget *parent, node::MwcNode * node );
    ~MwcNodeLogs();

private slots:
    void on_okButton_clicked();
    void onMwcOutputLine(QString line);

private:
    Ui::MwcNodeLogs *ui;
    node::MwcNode * node;
};

}

#endif // U_MWCNODELOGS_H
