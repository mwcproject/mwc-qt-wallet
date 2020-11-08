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

#include "richvbox.h"
#include <QVBoxLayout>
#include <QScrollBar>

namespace control {

RichVBox::RichVBox(QWidget *parent) : QScrollArea(parent)
{
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );

    this->setStyleSheet( "border: 1px solid rgba(255, 255, 255, 0.2); background: transparent" );
//    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    vlist = new QWidget (this);
    vlist->setStyleSheet( "border: transparent; padding: 0px" );
    layout = new QVBoxLayout(vlist);
    layout->setContentsMargins(0,0,3,0); // Have 3 on the right because of the scroll bar.
    vlist->setLayout( layout );

//    vlist->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );

    // Make it transparent
    layout->setSpacing(5);

    setWidget(vlist);
    setWidgetResizable(true);
}

void RichVBox::clearAll() {
    int count = layout->count();
    focusItem = nullptr;

    for (int t=count-1; t>=0; t--) {
        QLayoutItem *child = layout->takeAt(t);
        if (child!= nullptr) {
            delete child->widget();
            delete child;
        }
    }
}


RichVBox & RichVBox::addItem(RichItem * item) {
    item->setParent(this);

    layout->addWidget(item);
    return *this;
}

RichVBox & RichVBox::apply() {
    // It is a spacer that works as we need. Regular spacer works as regular Widget and it doesn't work for few items.
    // In order to make work, we need something with stretch non 0.
    layout->addStretch(1);
    return *this;
}

void RichVBox::itemClicked(QString id, RichItem * item) {
    Q_UNUSED(item);
    emit onItemClicked(id);
}
void RichVBox::itemDblClicked(QString id, RichItem * item) {
    Q_UNUSED(item);
    emit onItemDblClicked(id);
}
void RichVBox::itemFocus(QString id, RichItem * item) {
    if (focusItem != item ) {
        if (focusItem != nullptr) {
            focusItem->setFocusState(false);
        }
        focusItem = item;
        focusItem->setFocusState(true);
    }

    emit onItemFocus(id);
}
void RichVBox::itemActivated(QString id, RichItem * item) {
    Q_UNUSED(item);
    emit onItemActivated(id);
}


}

