#ifndef RICHVBOX_H
#define RICHVBOX_H

#include <QScrollArea>
#include "richitem.h"

class QVBoxLayout;

namespace control {

class RichVBox : public QScrollArea
{
    Q_OBJECT
public:
    explicit RichVBox(QWidget *parent);

    // clear all the data
    void clearAll();
    // Add next item into the list
    RichVBox & addItem(RichItem * item);
    // Done with adding
    RichVBox & apply();

    void itemClicked(QString id, RichItem * item);
    void itemDblClicked(QString id, RichItem * item);
    void itemFocus(QString id, RichItem * item);
    void itemActivated(QString id, RichItem * item);

signals:
    void onItemClicked(QString id);
    void onItemDblClicked(QString id);
    void onItemFocus(QString id);
    void onItemActivated(QString id);

private:
    QWidget * vlist = nullptr;
    QVBoxLayout * layout = nullptr;
    RichItem * focusItem = nullptr;
};

}


#endif // RICHVBOX_H
