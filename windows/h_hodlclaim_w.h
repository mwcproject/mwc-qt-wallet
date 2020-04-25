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

#ifndef H_HODLCLAIM_W_H
#define H_HODLCLAIM_W_H

#include <QWidget>
#include "../core/navwnd.h"

namespace Ui {
class HodlClaim;
}

namespace state {
class Hodl;
}

namespace wnd {

class HodlClaim : public core::NavWnd {
Q_OBJECT
public:
    explicit HodlClaim( QWidget *parent, state::Hodl * state, const QString & coldWalletHash );
    ~HodlClaim();

    void reportMessage(const QString &title, const QString &message);

    void updateHodlState();

private slots:
    void on_claimMwcButton_clicked();

    void on_refreshButton_clicked();

private:
    void initTableHeaders();
    void saveTableHeaders();

private:
    Ui::HodlClaim *ui;
    state::Hodl * state;
    const QString coldWalletHash;
};

}

#endif // H_HODLCLAIM_W_H
