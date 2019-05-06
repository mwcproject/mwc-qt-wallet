#ifndef SENDCOINSPARAMSDIALOG_H
#define SENDCOINSPARAMSDIALOG_H

#include <QDialog>
#include "../state/sendcoins.h"

namespace Ui {
class SendCoinsParamsDialog;
}

namespace wnd {

class SendCoinsParamsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SendCoinsParamsDialog(QWidget *parent, const state::SendCoinsParams & params);
    virtual ~SendCoinsParamsDialog() override;

    state::SendCoinsParams getSendCoinsParams() const {return params;}

private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::SendCoinsParamsDialog *ui;
    state::SendCoinsParams params;
};

}

#endif // SENDCOINSPARAMSDIALOG_H
