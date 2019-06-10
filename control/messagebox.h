#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include "mwcdialog.h"

namespace Ui {
class MessageBox;
}

namespace control {

class MessageBox : public MwcDialog
{
    Q_OBJECT
protected:
    // btnX if empty, the button will be hidden
    explicit MessageBox(QWidget *parent, QString title, QString message, QString btn1, QString btn2, bool default1, bool default2 );
    virtual ~MessageBox() override;

public:
    enum RETURN_CODE {BTN1, BTN2};

    // One button, OK box
    static void message( QWidget *parent, QString title, QString message );
    // Two button box
    static RETURN_CODE question( QWidget *parent, QString title, QString message, QString btn1, QString btn2, bool default1=false, bool default2=true );

private slots:
    void on_button1_clicked();

    void on_button2_clicked();

private:
    RETURN_CODE getRetCode() const {return retCode;}

    Ui::MessageBox *ui;
    RETURN_CODE retCode = BTN1;
};

}

#endif // MESSAGEBOX_H
