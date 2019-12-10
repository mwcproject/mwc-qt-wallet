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

#include "Bip39.h"
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
            control::MessageBox::messageText(nullptr, "Fatal Error", "Unable to read bip39 dictionary from the resources");
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

