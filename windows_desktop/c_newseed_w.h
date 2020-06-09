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

#ifndef NEWSEEDW_H
#define NEWSEEDW_H

#include "../core_desktop/navwnd.h"

namespace Ui {
class NewSeed;
}

namespace bridge {
class NewSeed;
}

namespace wnd {

class NewSeed : public core::NavWnd
{
    Q_OBJECT

public:
    explicit NewSeed(QWidget *parent,
                     const QVector<QString> & seed, bool hideSubmitButton = false );
    virtual ~NewSeed() override;

protected:
    void updateSeedData(const QString & name, const QVector<QString> & seed);

private slots:
    void onSgnShowSeedData(QVector<QString> seed);


    void on_submitButton_clicked();

private:
    Ui::NewSeed *ui;
    bridge::NewSeed * newSeed = nullptr;
    QVector<QString> seed;
};

}

#endif // NEWSEEDW_H
