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

#include "MwcCheckBox.h"

namespace control {

MwcCheckBox::MwcCheckBox(QWidget *parent) : QCheckBox(parent) {}

MwcCheckBox::MwcCheckBox(const QString &text, QWidget *parent) :
        QCheckBox(text, parent) {}

MwcCheckBox::~MwcCheckBox() {}

// to handle hoover
void MwcCheckBox::enterEvent(QEvent * event) {
    QCheckBox::enterEvent(event);
    if (isEnabled() && !hasFocus()) {
        QFont f = font();
        f.setUnderline(true);
        setFont(f);
    }
}

void MwcCheckBox::leaveEvent(QEvent * event) {
    QCheckBox::leaveEvent(event);
    if (!hasFocus()) {
        QFont f = font();
        f.setUnderline(false);
        f.setBold(false);
        setFont(f);
    }
}

// to handle focus
void MwcCheckBox::focusInEvent(QFocusEvent * event) {
    QCheckBox::focusInEvent(event);
    Q_UNUSED(event)
    if (isEnabled()) {
        QFont f = font();
        f.setUnderline(true);
        setFont(f);
    }
}

void MwcCheckBox::focusOutEvent(QFocusEvent * event) {
    QCheckBox::focusOutEvent(event);
    QFont f = font();
    f.setUnderline(false);
    setFont(f);
};


}
