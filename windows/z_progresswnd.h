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

#include "../core/PanelBaseWnd.h"

namespace Ui {
class ProgressWnd;
}

namespace wnd {

class ProgressWnd;

// Cancell callback interface. Because of usage we are fine withinterface, no needs in callback function
class IProgressWndState {
public:
    virtual void cancelProgress() = 0;
    virtual void destroyProgressWnd(ProgressWnd * w) = 0;
};


class ProgressWnd : public core::PanelBaseWnd
{
    Q_OBJECT

public:
    // Empty string - hide this item, null - hidden
    explicit ProgressWnd(QWidget *parent, IProgressWndState * state, QString header, QString msgProgress, QString msgPlus, bool cancellable );
    virtual ~ProgressWnd() override;

    void setHeader(QString header);
    void setMsgPlus(QString msgPlus);

    void initProgress(int min, int max);
    void updateProgress(int pos, QString msgProgress);

private slots:
    void on_cancelButton_clicked();

private:
    Ui::ProgressWnd *ui;
    IProgressWndState * state;
};

}


#endif // C_PROGRESSWND_H
