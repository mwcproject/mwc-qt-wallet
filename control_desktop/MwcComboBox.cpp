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

#include "MwcComboBox.h"
#include "MwcListView.h"
#include <QScrollBar>

namespace control {

MwcComboBox::MwcComboBox(QWidget *parent) : QComboBox(parent) {
    ensurePopupView();
}

void MwcComboBox::showPopup() {
    ensurePopupView();

    QComboBox::showPopup();
    auto *listView = qobject_cast<QListView *>(view());
    if (!listView) {
        return;
    }

    if (count() <= maxVisibleItems()) {
        auto *sb = listView->verticalScrollBar();
        sb->setRange(0, 0);
        sb->setValue(0);
        sb->setSingleStep(0);
        sb->setPageStep(0);
        listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        QWidget *popup = listView->parentWidget();
        if (popup) {
            // Qt 6.8 popup height can leave a few extra pixels; snap to whole rows.
            const int visibleRows = qMin(maxVisibleItems(), count());
            if (visibleRows > 0) {
                int rowHeight = listView->sizeHintForRow(0);
                if (rowHeight <= 0) {
                    rowHeight = listView->fontMetrics().height() + 6;
                }
                const int spacing = listView->spacing();
                const int desiredViewportHeight = visibleRows * rowHeight
                        + (visibleRows > 1 ? (visibleRows - 1) * spacing : 0);
                const int currentViewportHeight = listView->viewport()->height();
                const int delta = desiredViewportHeight - currentViewportHeight;
                if (delta != 0) {
                    popup->resize(popup->width(), popup->height() + delta);
                }
            }
        }
    } else {
        listView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }

#ifdef Q_OS_WIN
    // WIndows doesn;t need to udjust the popup, it is already works great
    //popup->move(popup->x() + 10, popup->y());
#else
   // QWidget *popup = this->findChild<QFrame *>();
   // Using pixels. Will work for used styles that defined in pixels too
   // popup->move(popup->x() + 10, popup->y()/* + height()*/);
#endif
}

void MwcComboBox::ensurePopupView() {
    if (qobject_cast<MwcListView *>(view())) {
        return;
    }

    auto *listView = new MwcListView(this);
    listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setView(listView);
}

}
