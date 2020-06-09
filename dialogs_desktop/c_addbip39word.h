// Copyright 2020 The MWC Developers
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

#ifndef C_ADDBIP39WORD_H
#define C_ADDBIP39WORD_H

#include "../control_desktop/mwcdialog.h"

namespace Ui {
class AddBip39Word;
}

class QCompleter;

namespace dlg {

class AddBip39Word : public control::MwcDialog
{
    Q_OBJECT

public:
    explicit AddBip39Word(QWidget *parent, int wordNumber);
    ~AddBip39Word();

    QString getResultWord() const {return resultWord;}

private slots:
    void on_wordEdit_textChanged(const QString &str);
    void on_cancelButton_clicked();
    void on_submitButton_clicked();
private:
    Ui::AddBip39Word *ui;
    QCompleter * completer = nullptr;
    QStringList words;
    QString resultWord;
};

}

#endif // C_ADDBIP39WORD_H
