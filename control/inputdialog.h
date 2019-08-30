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
    static QString getText( QWidget *parent, QString title, QString message, QString placeholder, QString text2init, int inputMaxLength, bool * ok = nullptr  );

private:
    explicit InputDialog( QWidget *parent, QString title, QString message, QString placeholder, QString text2init, int inputMaxLength );
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
