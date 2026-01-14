// Copyright 2026 The MWC Developers
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

#ifndef MWC_QT_WALLET_MWCTABWIDGET_H
#define MWC_QT_WALLET_MWCTABWIDGET_H

#include <QTabWidget>
#include <QProxyStyle>

namespace control {

class MwcTabWidget : public QTabWidget {
    Q_OBJECT
public:
    explicit MwcTabWidget(QWidget *parent = Q_NULLPTR);
    virtual ~MwcTabWidget() override;
};

}

#endif // MWC_QT_WALLET_MWCTABWIDGET_H
