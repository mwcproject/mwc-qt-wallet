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

#include "MwcCheckedFrame.h"
#include <QKeyEvent>

namespace control {

MwcCheckedFrame::MwcCheckedFrame(QWidget* parent, Qt::WindowFlags f) :
        MwcFrameWithBorder(parent, f) {

    updateCheckState();
    setMouseTracking(true);
}

MwcCheckedFrame::~MwcCheckedFrame() {
};


void MwcCheckedFrame::updateCheckState() {
    if (checked) {
        if (hasFocus())
            setStyleSheet("control--MwcCheckedFrame { background: rgba(255, 255, 255, 0.4);}");
        else
            setStyleSheet("control--MwcCheckedFrame { background: rgba(255, 255, 255, 0.35);}");
    }
    else {
        if (hasFocus())
            setStyleSheet("control--MwcCheckedFrame { background: rgba(255, 255, 255, 0.2);}");
        else {
            if (hover)
                setStyleSheet("control--MwcCheckedFrame { background: rgba(255, 255, 255, 0.1);}");
            else
                setStyleSheet("control--MwcCheckedFrame { background: transparent;}");
        }
    }
}

void MwcCheckedFrame::setChecked(bool _checked) {
    if (_checked == checked)
        return;

    checked = _checked;
    updateCheckState();

    if (checked) {
        emit onChecked(ctrlId);
    }
}

void MwcCheckedFrame::focusInEvent(QFocusEvent * event) { Q_UNUSED(event); updateCheckState(); MwcFrameWithBorder::focusInEvent(event); }

void MwcCheckedFrame::focusOutEvent(QFocusEvent * event) {Q_UNUSED(event); updateCheckState(); MwcFrameWithBorder::focusOutEvent(event); }

void MwcCheckedFrame::mousePressEvent(QMouseEvent * event) { Q_UNUSED(event); setChecked(true); MwcFrameWithBorder::mousePressEvent(event); }

void MwcCheckedFrame::keyPressEvent(QKeyEvent * event) {
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return || event->key() == Qt::Key_Space ) {
        setChecked(true);
    }
    MwcFrameWithBorder::keyPressEvent(event);
}

void MwcCheckedFrame::leaveEvent(QEvent * event) {
    hover = false;
    updateCheckState();
    MwcFrameWithBorder::leaveEvent(event);
}

void MwcCheckedFrame::mouseMoveEvent(QMouseEvent * event) {
    if (!hover) {
        hover = true;
        updateCheckState();
    }
    MwcFrameWithBorder::mouseMoveEvent(event);
}


}


