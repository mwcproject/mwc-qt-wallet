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

#ifndef C_PROGRESSWND_H
#define C_PROGRESSWND_H

#include "../core_desktop/PanelBaseWnd.h"

namespace Ui {
class ProgressWnd;
}

namespace bridge {
class ProgressWnd;
}

namespace wnd {

class ProgressWnd : public core::PanelBaseWnd
{
    Q_OBJECT
public:
    // Empty string - hide this item, null - hidden
    explicit ProgressWnd(QWidget *parent, QString callerId, QString header, QString msgProgress, QString msgPlus, bool cancellable );
    virtual ~ProgressWnd() override;

private slots:
    void on_cancelButton_clicked();

    void onSgnSetHeader(QString header);
    void onSgnSetMsgPlus(QString msgPlus);
    void onSgnInitProgress(int min, int max);
    void onSgnUpdateProgress(int pos, QString msgProgress);

private:
    Ui::ProgressWnd *ui;
    bridge::ProgressWnd * progressWnd = nullptr;
};

}


#endif // C_PROGRESSWND_H
