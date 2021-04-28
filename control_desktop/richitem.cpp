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

#include "richitem.h"
#include "richvbox.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QKeyEvent>

namespace control {

RichItem::RichItem(QString _id, QWidget *parent) : QFrame(parent),
    id(_id)
{
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    setStyleSheet( "border: transparent" );
    setFocusState(false);
    setFocusPolicy( Qt::FocusPolicy::ClickFocus );
}

// Create New layouts
RichItem & RichItem::hbox() {
    auto l = new QHBoxLayout( this);
    if (!layoutStack.isEmpty())
        layoutStack.last()->addItem(l);
    layoutStack.push_back(l);
    return *this;
}

RichItem & RichItem::vbox() {
    auto l = new QVBoxLayout( this);
    if (!layoutStack.isEmpty())
        layoutStack.last()->addItem(l);
    layoutStack.push_back(l);
    return *this;
}

RichItem & RichItem::pop() { // Pop layout back
    Q_ASSERT(layoutStack.size()>=2);
    layoutStack.pop_back();
    return *this;
}

RichItem & RichItem::setSpacing(int spacing) { // Spacing for layout
    Q_ASSERT(!layoutStack.isEmpty());
    layoutStack.back()->setSpacing(spacing);
    return *this;
}

RichItem & RichItem::setContentsMargins(int left, int top, int right, int bottom) {
    Q_ASSERT(!layoutStack.isEmpty());
    layoutStack.back()->setContentsMargins(left, top, right, bottom);
    return *this;
}

RichItem & RichItem::addWidget( QWidget * widget ) {
    Q_ASSERT(!layoutStack.isEmpty());
    layoutStack.back()->addWidget(widget);
    curWidget = widget;
    return *this;
}

RichItem & RichItem::setWidgetStyleSheet(QString styleSheet) {
    Q_ASSERT(curWidget!=nullptr);
    curWidget->setStyleSheet(styleSheet);
    return *this;
}

RichItem & RichItem::setFixedWidth(int cx) {
    Q_ASSERT(curWidget!=nullptr);
    curWidget->setMinimumWidth(cx);
    curWidget->setMaximumWidth(cx);
    return *this;
}

RichItem & RichItem::setFixedHeight(int cy) {
    Q_ASSERT(curWidget!=nullptr);
    curWidget->setMinimumHeight(cy);
    curWidget->setMaximumHeight(cy);
    return *this;
}

RichItem & RichItem::setMinWidth(int cx) {
    Q_ASSERT(curWidget!=nullptr);
    curWidget->setMinimumWidth(cx);
    return *this;
}

RichItem & RichItem::setText(const QString & text) {
    Q_ASSERT(curWidget!=nullptr);
    ((QLabel *)curWidget)->setText(text);
    return *this;
}

RichItem & RichItem::addHSpacer() {
    Q_ASSERT(!layoutStack.isEmpty());
    layoutStack.back()->addItem( new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed) );
    return *this;
}

RichItem & RichItem::addVSpacer() {
    Q_ASSERT(!layoutStack.isEmpty());
    layoutStack.back()->addItem( new QSpacerItem(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding) );
    return *this;
}

RichItem & RichItem::addFixedHSpacer(int cx) {
    Q_ASSERT(!layoutStack.isEmpty());
    layoutStack.back()->addItem( new QSpacerItem(cx,1, QSizePolicy::Fixed, QSizePolicy::Expanding) );
    return *this;
}

RichItem & RichItem::addFixedVSpacer(int cy) {
    Q_ASSERT(!layoutStack.isEmpty());
    layoutStack.back()->addItem( new QSpacerItem(1,cy, QSizePolicy::Expanding, QSizePolicy::Fixed) );
    return *this;
}


RichItem & RichItem::apply() { // last step, first item form activeLayoutStack will be applyed
    Q_ASSERT(!layoutStack.isEmpty());
    setLayout(layoutStack[0]);

    return *this;
}

void RichItem::focusInEvent(QFocusEvent *event) {
    Q_ASSERT(event!=nullptr);
    if (event->gotFocus())
        eventsHandler->itemFocus(id, this);
}

void RichItem::setFocusState(bool focus) {
    if (focus)
        setStyleSheet("background: rgba(255, 255, 255, 0.2)");
    else
        setStyleSheet("background: rgba(255, 255, 255, 0.1)");
}


void RichItem::mouseDoubleClickEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    eventsHandler->itemDblClicked(id, this);
    eventsHandler->itemActivated(id, this);
}

void RichItem::mousePressEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    eventsHandler->itemClicked(id, this);
}

void RichItem::keyPressEvent(QKeyEvent *event) {
    if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return )
        eventsHandler->itemActivated(id, this);
}

//////////////////////////////////////////////////////////////////////////////////////
//  Helpers

// Create Mark + v layout that ready to accept the rows
RichItem * createMarkedItem(QString itemId, QWidget *parent, bool marked, QString markColor, int leftMarkSize,
                            int leftMarkSpacing,
                            int vboxSpacing,
                            int vboxMargin ) {
    control::RichItem * itm = new control::RichItem( itemId, parent);

    // Level Indicator
    itm->hbox().setSpacing(leftMarkSpacing)
            .setContentsMargins(0, 0, leftMarkSize + leftMarkSpacing, 0)
            .addWidget( new QWidget(itm) )
            .setFixedWidth( leftMarkSize )
            .setWidgetStyleSheet(marked ? control::LEFT_MARK_ON(markColor) : control::LEFT_MARK_OFF);

    itm->vbox().setSpacing(vboxSpacing)
            .setContentsMargins(0, vboxMargin, 0, vboxMargin);

    return itm;
}

QLabel * createLabel( control::RichItem * parent, bool wordwrap, bool lowLight, QString text, int fontSize, QString color  ) {
    QLabel * l = new QLabel(parent);
    l->setWordWrap(wordwrap);
    l->setTextInteractionFlags(Qt::TextSelectableByMouse);

    QString selectedColor;
    if (lowLight) {
        color = control::LOW_LIGHT_COLOR;
        selectedColor = control::LOW_LIGHT_SELECTED_COLOR;
    }
    if (color.isEmpty()) {
        color = "white";
    }
    if (selectedColor.isEmpty())
        selectedColor = color;

    l->setStyleSheet( "color: " + color + "; " +
            "border: transparent; background: transparent; font-weight: 400; font-size: " + QString::number(fontSize) + "px; " +
            "selection-background-color: rgba(255, 255, 255, 0.3); selection-color: "+selectedColor+"; " );
    l->setText(text);
    return l;
}

// Create a lable with an icon
QLabel *  createIcon( control::RichItem * parent, QString pixmapPath, int cx, int cy ) {
    QLabel * l = new QLabel(parent);
    l->setStyleSheet("background: transparent; border: transparent");
    l->setScaledContents(true);
    l->setPixmap( QPixmap(pixmapPath) );
    l->setMinimumSize(cx,cy);
    l->setMaximumSize(cx,cy);
    return l;
}

QLabel * createHorzLine(control::RichItem * parent) {
    QLabel * l = new QLabel(parent);
    l->setMinimumHeight(1);
    l->setMaximumHeight(1);
    l->setStyleSheet(HORZ_LINE);
    return l;
}


}
