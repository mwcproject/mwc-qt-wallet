#ifndef SENDCOINSPARAMSDIALOG_H
#define SENDCOINSPARAMSDIALOG_H

#include "../control/mwcdialog.h"
#include "../core/appcontext.h"

namespace Ui {
class SendCoinsParamsDialog;
}

namespace wnd {

class SendCoinsParamsDialog : public control::MwcDialog
{
    Q_OBJECT

public:
    explicit SendCoinsParamsDialog(QWidget *parent, const core::SendCoinsParams & params);
    virtual ~SendCoinsParamsDialog() override;

    core::SendCoinsParams getSendCoinsParams() const {return params;}

private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::SendCoinsParamsDialog *ui;
    core::SendCoinsParams params;
};

}

#endif // SENDCOINSPARAMSDIALOG_H
