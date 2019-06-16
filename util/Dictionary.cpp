#include "Dictionary.h"
#include <QApplication>
#include "../control/messagebox.h"
#include <QFile>
#include <QTextStream>

namespace util {

static QSet<QString>  bip39words;

const QSet<QString> & getBip39words() {
    if (bip39words.empty()) {
        // load the words
        QFile file(":/txt/bip39_words.txt");
        if (!file.open(QFile::ReadOnly)) {
            control::MessageBox::message(nullptr, "Fatal Error", "Unable to read bip39 dictionary from the resources");
            QApplication::quit();
            return bip39words;
        }

        QTextStream in(&file);

        while(!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty())
                continue;

            bip39words += line;
        }
        file.close();
    }

    return bip39words;
}

}

