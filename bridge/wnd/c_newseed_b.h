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

#ifndef MWC_QT_WALLET_C_NEWSEED_B_H
#define MWC_QT_WALLET_C_NEWSEED_B_H

#include <QObject>

namespace bridge {

class NewSeed : public QObject {
Q_OBJECT
public:
    explicit NewSeed(QObject * parent = nullptr);
    ~NewSeed();

    void showSeedData(const QVector<QString> & seed);

    // Done looking at a new seed. Let's go forward
    Q_INVOKABLE void doneWithNewSeed();

    // Submit a new word from the user input
    Q_INVOKABLE void submitSeedWord(QString word);
private:
signals:
    void sgnShowSeedData(QVector<QString> seed);
};

}

#endif //MWC_QT_WALLET_C_NEWSEED_B_H
