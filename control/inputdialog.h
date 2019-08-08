#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include "mwcdialog.h"

namespace Ui {
class InputDialog;
}

namespace control {

// QInputDialog alternative
class InputDialog : public MwcDialog
{
    Q_OBJECT
public:
    static QString getText( QWidget *parent, QString title, QString message, QString placeholder, QString text2init, bool * ok = nullptr  );

private:
    explicit InputDialog( QWidget *parent, QString title, QString message, QString placeholder, QString text2init);
    virtual ~InputDialog() override;

    QString getText() const {return text;}
private slots:
    void on_cancelBtn_clicked();
    void on_okBtn_clicked();

private:
    Ui::InputDialog *ui;
    QString text;
};

}

#endif // INPUTDIALOG_H
