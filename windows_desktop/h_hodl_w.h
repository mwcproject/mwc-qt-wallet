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

#ifndef HODLW_H
#define HODLW_H

#include "../core_desktop/navwnd.h"

namespace Ui {
class Hodl;
}

namespace bridge {
class Util;
class Hodl;
class HodlStatus;
}

namespace wnd {

class Hodl : public core::NavWnd
{
    Q_OBJECT

public:
    explicit Hodl(QWidget *parent);
    ~Hodl();

private slots:
    void on_learnMoreButton_clicked();
    void on_signInButton_clicked();
    void on_claimMwcButton_clicked();

    void onSgnUpdateHodlState();
    void onSgnReportMessage(QString  title, QString message);
    void onSgnHideWaitingStatus();
protected:
    void mouseDoubleClickEvent( QMouseEvent * e ) override;

private:
    Ui::Hodl *ui;
    bridge::Util * util = nullptr;
    bridge::Hodl * hodl = nullptr;
    bridge::HodlStatus * hodlStatus = nullptr;
};

}

#endif // HODL_H
