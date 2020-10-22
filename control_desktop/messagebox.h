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
#include "../core/WndManager.h"

namespace Ui {
class MessageBox;
}

namespace control {

class MessageBox : public MwcDialog
{
    Q_OBJECT
public:
protected:
    // btnX if empty, the button will be hidden
    // Password accepted as a HASH. EMpty String mean that no password is set.
    // After return, passwordHash value will have input raw Password value. So it can be user for wallet
    explicit MessageBox(QWidget *parent, QString title, QString message, bool htmlMsg, QString btn1, QString btn2,
            QString btn1Tooltip, QString btn2Tooltip,
            bool default1, bool default2, double widthScale, QString & passwordHash, core::WndManager::RETURN_CODE passBlockButton,
            int ttl_blocks );
    virtual ~MessageBox() override;

public:
    // widthScale - Horizontal scale for the message box. Sometimes we need it wider.
    // One button, OK box
    static void messageText( QWidget *parent, QString title, QString message, double widthScale = 1.0);
    static void messageHTML( QWidget *parent, QString title, QString message, double widthScale = 1.0);
    // Two button box
    static core::WndManager::RETURN_CODE questionText( QWidget *parent, QString title, QString message,
            QString btn1, QString btn2,
            QString btn1Tooltip, QString btn2Tooltip,
            bool default1=false, bool default2=true, double widthScale = 1.0 );
    static core::WndManager::RETURN_CODE questionTextWithTTL( QWidget *parent, QString title, QString message,
            QString btn1, QString btn2,
            QString btn1Tooltip, QString btn2Tooltip,
            bool default1, bool default2, double widthScale, int *ttl_blocks );

    static core::WndManager::RETURN_CODE questionHTML( QWidget *parent, QString title, QString message,
            QString btn1, QString btn2,
            QString btn1Tooltip, QString btn2Tooltip,
            bool default1=false, bool default2=true, double widthScale = 1.0 );

    // Password accepted as a HASH. EMpty String mean that no password is set.
    // After return, passwordHash value will have input raw Password value. So it can be user for wallet
    static core::WndManager::RETURN_CODE questionText( QWidget *parent, QString title, QString message, QString btn1, QString btn2,
            QString btn1Tooltip, QString btn2Tooltip,
            bool default1, bool default2, double widthScale, QString & passwordHash, core::WndManager::RETURN_CODE blockButton );
    static core::WndManager::RETURN_CODE questionTextWithTTL( QWidget *parent, QString title, QString message, QString btn1, QString btn2,
            QString btn1Tooltip, QString btn2Tooltip,
            bool default1, bool default2, double widthScale, QString & passwordHash, core::WndManager::RETURN_CODE blockButton, int *ttl_blocks );

private slots:
    void on_passwordEdit_textChanged(const QString &str);
    void on_button1_clicked();
    void on_button2_clicked();
private:
    void processApplyButton(core::WndManager::RETURN_CODE retCode);
private:
    core::WndManager::RETURN_CODE getRetCode() const {return retCode;}

    Ui::MessageBox *ui;
    core::WndManager::RETURN_CODE retCode = core::WndManager::RETURN_CODE::BTN1;
    QString & blockingPasswordHash;
    core::WndManager::RETURN_CODE passBlockButton = core::WndManager::RETURN_CODE::BTN1;
    int ttl_blocks = -1;
};

}

#endif // MESSAGEBOX_H
