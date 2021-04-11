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

#ifndef GUI_WALLET_MWCLABELPROGRESS_H
#define GUI_WALLET_MWCLABELPROGRESS_H

#include <QLabel>

namespace control {

// Message box normally error and it should cancel the progress. It is a shortcut that help to avoid propagate back response signals
void onMessageBoxShown();

// Progress movie control
class MwcLabelProgress : public QLabel {
Q_OBJECT
public:
    explicit MwcLabelProgress(QWidget * parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags(), bool hideWithMessageBox = true );
    explicit MwcLabelProgress(const QString &text, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags(), bool hideWithMessageBox = true);
    virtual ~MwcLabelProgress() override;

    void initLoader(bool visible);
};

}

#endif //GUI_WALLET_MWCLABELPROGRESS_H
