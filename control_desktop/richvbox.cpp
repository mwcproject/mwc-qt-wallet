#include "richvbox.h"
#include <QVBoxLayout>

namespace control {

RichVBox::RichVBox(QWidget *parent) : QScrollArea(parent)
{
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );

    this->setStyleSheet( "border: transparent; background: transparent" );
//    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    vlist = new QWidget (this);
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

