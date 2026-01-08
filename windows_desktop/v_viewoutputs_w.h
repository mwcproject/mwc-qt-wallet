// Copyright 2024 The MWC Developers
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

#ifndef V_VIEWACCOUNTS_W_H
#define V_VIEWACCOUNTS_W_H

#include "e_outputs_w.h"
#include "../core_desktop/navwnd.h"
#include <QJsonArray>

namespace Ui {
class ViewOutputs;
}

namespace bridge {
class Wallet;
class ViewOutputs;
}

namespace wnd {

class ViewOutputs : public core::NavWnd
{
    Q_OBJECT

public:
    explicit ViewOutputs(QWidget *parent = nullptr);
    ~ViewOutputs();

private:
    void updateShownData(bool resetScrollData);

private slots:
    void on_backButton_clicked();

    void onSgnViewOutputs( QString viewingKey, QJsonArray outputs, QString totalAmount);

    void onItemActivated(QString itemId);
private:
    Ui::ViewOutputs *ui;
    bridge::ViewOutputs * viewAcounts = nullptr;

    QVector<OutputData> allData; // all outputs

};

}

#endif // V_VIEWACCOUNTS_W_H
