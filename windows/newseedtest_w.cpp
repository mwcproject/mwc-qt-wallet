#include "newseedtest_w.h"
#include "ui_newseedtest.h"
#include "../state/newseedtest.h"
#include <QCompleter>
#include <QFile>
#include <QStringListModel>
#include <QTreeView>
#include <QStandardItemModel>

namespace wnd {

NewSeedTest::NewSeedTest(QWidget *parent, state::NewSeedTest *_state, int wordNumber) :
    QWidget(parent),
    ui(new Ui::NewSeedTest),
    state(_state)
{
    ui->setupUi(this);

    ui->wordLabelHi->setText("<b>Please Confirm word number " +
                             QString::number(wordNumber) + " for your passphrase</b>");
    ui->wordLabelLo->setText("<b>Word number " + QString::number(wordNumber) + "</b>");


    // Read the model with bip39 words from the resources
    QFile file(":/txt/bip39_words.txt");
    if (file.open(QFile::ReadOnly)) {
        completer = new QCompleter(this);

        QStringList words;

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

        ui->wordEdit->setCompleter(completer);
    }

}

NewSeedTest::~NewSeedTest()
{
    delete ui;
}


void NewSeedTest::on_submitButton_clicked()
{
    state->submit( ui->wordEdit->text() );
}

}
