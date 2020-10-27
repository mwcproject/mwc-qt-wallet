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

#include "MwcRadioButton.h"

namespace control {

MwcRadioButton::MwcRadioButton(QWidget *parent) : QRadioButton(parent) {}

MwcRadioButton::MwcRadioButton(const QString &text, QWidget *parent) :
        QRadioButton(text, parent) {}

MwcRadioButton::~MwcRadioButton() {}

// to handle hoover
void MwcRadioButton::enterEvent(QEvent * event) {
    QRadioButton::enterEvent(event);
    if (isEnabled() && !hasFocus()) {
        QFont f = font();
        f.setUnderline(true);
        setFont(f);
    }
}

void MwcRadioButton::leaveEvent(QEvent * event) {
    QRadioButton::leaveEvent(event);
    if (!hasFocus()) {
        QFont f = font();
        f.setUnderline(false);
        f.setBold(false);
        setFont(f);
    }
}

// to handle focus
void MwcRadioButton::focusInEvent(QFocusEvent * event) {
    QRadioButton::focusInEvent(event);
    Q_UNUSED(event)
    if (isEnabled()) {
        QFont f = font();
        f.setUnderline(true);
        setFont(f);
    }
}

void MwcRadioButton::focusOutEvent(QFocusEvent * event) {
    QRadioButton::focusOutEvent(event);
    QFont f = font();
    f.setUnderline(false);
    setFont(f);
};


}
