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

#ifndef MWC_QT_WALLET_RICHBUTTON_H
#define MWC_QT_WALLET_RICHBUTTON_H

#include <QPushButton>

namespace control {

class RichButton;

// Events will be easier to handle with callbacks, not with events. Not cool but more robust model
class RichButtonPressCallback {
public:
    virtual void richButtonPressed(RichButton * button, QString coockie) = 0;
};

class RichButton : public QPushButton {
Q_OBJECT
public:
    RichButton(QWidget * parent, QString text, int cx, int cy, QString tooltip, int fontSize=10);

    RichButton * setCallback(RichButtonPressCallback * pressCallback, const QString & cookie);

private slots:
    void onClicked(bool checked);

protected:
    RichButtonPressCallback * pressCallback = nullptr;
    QString cookie;
};

}

#endif //MWC_QT_WALLET_RICHBUTTON_H
