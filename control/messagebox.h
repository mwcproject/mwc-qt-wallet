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
public:
    enum class RETURN_CODE {BTN1, BTN2};
protected:
    // btnX if empty, the button will be hidden
    // Password accepted as a HASH. EMpty String mean that no password is set.
    // After return, passwordHash value will have input raw Password value. So it can be user for wallet
    explicit MessageBox(QWidget *parent, QString title, QString message, bool htmlMsg, QString btn1, QString btn2, bool default1, bool default2, double widthScale, QString & passwordHash, RETURN_CODE passBlockButton );
    virtual ~MessageBox() override;

public:
    // widthScale - Horizontal scale for the message box. Sometimes we need it wider.
    // One button, OK box
    static void messageText( QWidget *parent, QString title, QString message, double widthScale = 1.0);
    static void messageHTML( QWidget *parent, QString title, QString message, double widthScale = 1.0);
    // Two button box
    static RETURN_CODE questionText( QWidget *parent, QString title, QString message, QString btn1, QString btn2, bool default1=false, bool default2=true, double widthScale = 1.0 );
    static RETURN_CODE questionHTML( QWidget *parent, QString title, QString message, QString btn1, QString btn2, bool default1=false, bool default2=true, double widthScale = 1.0 );

    // Password accepted as a HASH. EMpty String mean that no password is set.
    // After return, passwordHash value will have input raw Password value. So it can be user for wallet
    static RETURN_CODE questionText( QWidget *parent, QString title, QString message, QString btn1, QString btn2, bool default1, bool default2, double widthScale, QString & passwordHash, RETURN_CODE blockButton );

private slots:
    void on_passwordEdit_textChanged(const QString &str);
    void on_button1_clicked();
    void on_button2_clicked();
private:
    RETURN_CODE getRetCode() const {return retCode;}

    Ui::MessageBox *ui;
    RETURN_CODE retCode = RETURN_CODE::BTN1;
    QString & blockingPasswordHash;
    RETURN_CODE passBlockButton = RETURN_CODE::BTN1;
};

}

#endif // MESSAGEBOX_H
