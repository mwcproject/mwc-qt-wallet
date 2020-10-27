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

#ifndef MWC_QT_WALLET_MWCRADIOBUTTON_H
#define MWC_QT_WALLET_MWCRADIOBUTTON_H

#include <QRadioButton>

namespace control {


class MwcRadioButton : public QRadioButton {
    Q_OBJECT
public:
    explicit MwcRadioButton(QWidget *parent = Q_NULLPTR);
    explicit MwcRadioButton(const QString &text, QWidget *parent = Q_NULLPTR);
    virtual ~MwcRadioButton();

protected:
    // to handle hoover
    virtual void enterEvent(QEvent * event) override;
    virtual void leaveEvent(QEvent * event) override;
    // to handle focus
    virtual void focusInEvent(QFocusEvent * event) override;
    virtual void focusOutEvent(QFocusEvent * event) override;
};

}

#endif //MWC_QT_WALLET_MWCRADIOBUTTON_H
