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

    for (int t=count-1; t>=0; t++) {
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

    layout->addItem( new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Expanding) );
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

