#include "c_newseedtest_w.h"
#include "ui_c_newseedtest.h"
#include "../state/a_initaccount.h"
#include <QCompleter>
#include <QFile>
#include <QStringListModel>
#include <QTreeView>
#include <QStandardItemModel>
#include "../util/widgetutils.h"

namespace wnd {

NewSeedTest::NewSeedTest(QWidget *parent, state::InitAccount *_state, int wordNumber) :
    QWidget(parent),
    ui(new Ui::NewSeedTest),
    state(_state)
{
    ui->setupUi(this);

    state->setWindowTitle("Verify your seed");

    ui->wordEdit->setPlaceholderText( "Please enter word number " + QString::number(wordNumber) );

//    ui->wordLabelLo->setText("<b>Word number " + QString::number(wordNumber) + "</b>");


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

    ui->wordEdit->setFocus();

    utils::defineDefaultButtonSlot(this, SLOT(on_submitButton_clicked()) );
}

NewSeedTest::~NewSeedTest()
{
    state->deleteNewSeedTestWnd(this);
    delete ui;
}


void NewSeedTest::on_submitButton_clicked()
{
    // no need to verify the symbols
    state->submit( ui->wordEdit->text().trimmed() );
}

}
