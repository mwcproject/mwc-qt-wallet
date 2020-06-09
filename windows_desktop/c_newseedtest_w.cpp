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

#include "c_newseedtest_w.h"
#include "ui_c_newseedtest.h"
#include <QCompleter>
#include <QFile>
#include <QStringListModel>
#include <QTreeView>
#include <QStandardItemModel>
#include "../util_desktop/widgetutils.h"
#include "../bridge/util_b.h"
#include "../bridge/wnd/c_newseed_b.h"

namespace wnd {

NewSeedTest::NewSeedTest(QWidget *parent, int wordNumber) :
    core::PanelBaseWnd(parent),
    ui(new Ui::NewSeedTest)
{
    ui->setupUi(this);

    util = new bridge::Util(this);
    newSeed = new bridge::NewSeed(this);

    ui->wordEdit->setPlaceholderText( "Please enter word number " + QString::number(wordNumber) );

    // Read the model with bip39 words from the resources
    QVector<QString> bip39Words = util->getBip39words();

    if (!bip39Words.isEmpty()) {
        completer = new QCompleter(this);
        // Creating a model with a dictionary
        QStandardItemModel *m = new QStandardItemModel(bip39Words.count(), 1, completer);
        for (int i = 0; i < bip39Words.count(); ++i) {
                QModelIndex index = m->index(i, 0);
                m->setData(index, bip39Words[i]);
        }

        completer->setMaxVisibleItems(10);
        completer->setCompletionMode( QCompleter::PopupCompletion );
        completer->setModel( m );
        completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);

        ui->wordEdit->setCompleter(completer);
    }

    ui->wordEdit->setFocus();

    utils::defineDefaultButtonSlot(this, SLOT(on_submitButton_clicked()) );
}

NewSeedTest::~NewSeedTest()
{
    delete ui;
}


void NewSeedTest::on_submitButton_clicked()
{
    // no need to verify the symbols
    newSeed->submitSeedWord( ui->wordEdit->text().trimmed() );
}

}
