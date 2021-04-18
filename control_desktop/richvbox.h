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
    void clearAll(bool resetScrollValue);
    // Add next item into the list
    RichVBox & addItem(RichItem * item);
    // Done with adding
    RichVBox & apply();

    void itemClicked(QString id, RichItem * item);
    void itemDblClicked(QString id, RichItem * item);
    void itemFocus(QString id, RichItem * item);
    void itemActivated(QString id, RichItem * item);

private
slots:
    void onHorzValueChanged(int value);
    void onHorzRangeChanged(int min, int max);

signals:
    void onItemClicked(QString id);
    void onItemDblClicked(QString id);
    void onItemFocus(QString id);
    void onItemActivated(QString id);

private:
    QWidget * vlist = nullptr;
    QVBoxLayout * layout = nullptr;
    RichItem * focusItem = nullptr;

    int scrollValue = 0;
    int needSetValue = false;
};

}


#endif // RICHVBOX_H
