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

#ifndef LISTWITHCOLUMNS_H
#define LISTWITHCOLUMNS_H

#include <QTableWidget>

class ListWithColumns : public QTableWidget
{
    Q_OBJECT

public:
    ListWithColumns(QWidget *parent = nullptr);
    ListWithColumns(int rows, int columns, QWidget *parent = nullptr);
    virtual ~ListWithColumns() override;

    // Note, changes will be applicable to a new data only!!!
  //  void setMultilineColumn(int column) { multilineColumns += column; }
  //  void removeMultilineColumn(int column) { multilineColumns -= column; }

    // get number of rows that expected to be visible
    int getNumberOfVisibleRows() const;
    static int getNumberOfVisibleRows(int height);

    void setTextAlignment(int alignment) { textAlignment = alignment; }
    void setHightlightColors(QColor low, QColor hi) {selectedLow=low; selectedHi=hi;}
    // Alpha delta for row stripe coloring. Range 0-255
    void setStripeAlfaDelta( int alpha ) { bkColor2.setAlpha(alpha); }

    void setColumnWidths(QVector<int> widths);
    QVector<int> getColumnWidths() const;

    // Get current selected row.
    int getSelectedRow() const;

    // clear the data
    void clearData();

    // Caller responsible to setup the columns and be sure that the data match them.
    // selection - highlighting level.
    // <=0 - nothing
    // >=1 - highlight a lot
    void appendRow( const QVector<QString> & rowData, double selection = -1.0 );

protected:
    void setListLook();

protected:
    int textAlignment = Qt::AlignCenter;

    // Two background color
    QColor bkColor1 = QColor(255,255,255,0);
    QColor bkColor2 = QColor(255,255,255,0);

    QColor selectedLow = QColor(0,0,0,0);  // Special color for row selection
    QColor selectedHi = QColor(0,0,0,0);

   // QSet<int> multilineColumns;
};

#endif // LISTWITHCOLUMNS_H
