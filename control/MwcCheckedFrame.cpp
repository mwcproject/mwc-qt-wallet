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


