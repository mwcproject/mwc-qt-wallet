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

#include "c_addbip39word.h"
#include "ui_c_addbip39word.h"
#include <QFile>
#include <QCompleter>
#include <QStandardItemModel>
#include "../control_desktop/messagebox.h"
#include <QListView>

namespace dlg {

AddBip39Word::AddBip39Word(QWidget *parent, int wordNumber) :
    control::MwcDialog(parent),
    ui(new Ui::AddBip39Word)
{
    ui->setupUi(this);

    ui->titleLabel->setText( "Mnemonic passphrase, Word " + QString::number(wordNumber) + " from 24" );

    // Read the model with bip39 words from the resources
    QFile file(":/txt/bip39_words.txt");

    if (file.open(QFile::ReadOnly)) {
            completer = new QCompleter(this);

            while (!file.atEnd()) {
                    QByteArray line = file.readLine();
                    if (!line.isEmpty())
                        words << line.trimmed();
            }
            // Creating a model with a dictionary
            QStandardItemModel *m = new QStandardItemModel(words.count(), 1, completer);
            for (int i = 0; i < words.count(); ++i) {
                    QModelIndex index = m->index(i, 0);
                    m->setData(index, words[i]);
            }

            completer->setMaxVisibleItems(10);
            completer->setCompletionMode( QCompleter::PopupCompletion );
            completer->setModel( m );
            completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);

            // By some reasons QListView can't be loaded form the stylesheet. I guess it is caches.
            // That is why we are creting our own and set style for it
            QListView * list = new QListView(this);
            list->setStyleSheet("QListView {color: #6F00D6;background: white;selection-color: #6F00D6;selection-background-color: #cc9afb;}");
            completer->setPopup( list );

            ui->wordEdit->setCompleter(completer);
    }
    else {
        Q_ASSERT(false);
        control::MessageBox::messageText(parent, "Internal Error", "Error: Unable to read Bip39 dictionary.");
    }

    ui->wordEdit->setFocus();
    ui->submitButton->setEnabled(false);
}

AddBip39Word::~AddBip39Word()
{
    delete ui;
}

void AddBip39Word::on_wordEdit_textChanged(const QString &str)
{
    bool foundWord = false;
    for (const QString & w : words) {
        if (w.compare(str, Qt::CaseInsensitive) == 0) {
            foundWord = true;
            break;
        }
    }

    ui->submitButton->setEnabled(foundWord);
}

void AddBip39Word::on_cancelButton_clicked()
{
    reject();
}

void AddBip39Word::on_submitButton_clicked()
{
    resultWord = ui->wordEdit->text().toLower();
    accept();
}

}
