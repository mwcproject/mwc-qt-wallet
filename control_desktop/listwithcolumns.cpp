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

#include "listwithcolumns.h"
#include <QHeaderView>
#include <QApplication>
#include <QTextEdit>

const int ROW_HEIGHT = 30;

ListWithColumns::ListWithColumns(QWidget *parent) :
    QTableWidget (parent)
{
    setListLook();
}

ListWithColumns::ListWithColumns(int rows, int columns, QWidget *parent) :
    QTableWidget (rows, columns, parent)
{
    setListLook();
}

ListWithColumns::~ListWithColumns() {}


void ListWithColumns::setListLook() {
    setShowGrid(false);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    // no sorting
    setSortingEnabled(false);

    // Horizontal scrolling
//    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
//    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);


    verticalHeader()->setVisible(false);
    // Decrease slightly horizontally size
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    //verticalHeader()->setMinimumSectionSize(verticalHeader()->minimumSectionSize()*5/6);
    verticalHeader()->setDefaultSectionSize(ROW_HEIGHT);
    // Increase V size so

    horizontalHeader()->setFixedHeight( ROW_HEIGHT );

}

// static
int ListWithColumns::getNumberOfVisibleRows(int height)  {
    return std::max( 1, height / ROW_HEIGHT -2 ); // -2 for headers
}


// get number of rows that expected to be visible
int ListWithColumns::getNumberOfVisibleRows() const {
    QSize sz = size();
    return getNumberOfVisibleRows( sz.height() );
}


void ListWithColumns::setColumnWidths(QVector<int> widths) {
    Q_ASSERT( columnCount() == widths.size() );

    for (int u=0;u<widths.size();u++)
        setColumnWidth(u,widths[u]);
}

QVector<int> ListWithColumns::getColumnWidths() const {
    QVector<int> widths(columnCount());

    for (int t=0;t<widths.size();t++)
        widths[t] = columnWidth(t);

    return widths;
}


void ListWithColumns::clearData()
{
    clearContents();
    setRowCount(0);
}

void ListWithColumns::appendRow( const QVector<QString> & rowData, double selection ) {
    Q_ASSERT(rowData.size() == columnCount() );

    int rowIdx = rowCount();

    QColor clr;

    if (selection>=1.0)
        clr = selectedHi;
    if (selection <= 0.0) {
        clr = rowIdx % 2 == 0 ? bkColor1 : bkColor2;
    }
    else {
        // Calculating the gradient
        clr.setRgbF( selectedLow.redF() * (1.0-selection) + selectedHi.redF() * selection,
                     selectedLow.greenF() * (1.0-selection) + selectedHi.greenF() * selection,
                     selectedLow.blueF() * (1.0-selection) + selectedHi.blueF() * selection,
                     selectedLow.alphaF() * (1.0-selection) + selectedHi.alphaF() * selection );
    }

    setRowCount(rowIdx+1);
    int sz = rowData.size();
    for ( int i=0; i<sz; i++ ) {
        auto * itm = new QTableWidgetItem( rowData[i] );

        QColor c2set = clr;
        /*if (i%2==1)
        {
            c2set = bkColor2;
        }*/

        if (highlightedColumns.contains(i))
            c2set = highlightedColumnColor;

        itm->setBackground( QBrush(c2set) );
        itm->setTextAlignment( textAlignment );
        setItem(rowIdx , i, itm );

     /*   if (multilineColumns.contains(i)) {
            QTextEdit *edit = new QTextEdit();
            edit->setText(rowData[i]);
            edit->setReadOnly(true);
            setCellWidget(rowIdx, i, edit);
        }*/
    }
}

void ListWithColumns::setItemText(int row, int column, QString text) {
    QTableWidgetItem * itm = item(row, column);
    Q_ASSERT(itm);
    if (itm == nullptr)
        return;
    itm->setText(text);
}


int ListWithColumns::getSelectedRow() const {
    QList<QTableWidgetItem *> selItms = selectedItems();
    if (selItms.size() == 0)
        return -1;

    return selItms.front()->row();
}


//#include "listwithcolumns.moc"



