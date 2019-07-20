#include "listwithcolumns.h"
#include <QHeaderView>
#include <QApplication>

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

    // Setup some colors
    /*  System color
    bkColor1 = QApplication::palette().color(QPalette::Base); // Normally it is white
    bkColor2 = bkColor1;
    qreal r,g,b;
    bkColor2.getRgbF(&r,&g,&b);
    const qreal d = 0.05;
    if (r+g+b < 1.5)
        bkColor2.setRgbF( std::min(1.0,r+d), std::min(1.0,g+d), std::min(1.0,b+d) );
    else
        bkColor2.setRgbF( std::max(0.0,r-d), std::max(0.0,g-d), std::max(0.0,b-d) );
*/

    // MWC palette
//    bkColor1 = QColor(255,255,255,0);
//    bkColor2 = QColor(255,255,255,20);
//    selectedLow = QColor(255,0xED,0,90); // Yellow color
//    selectedHi = QColor(255,0xED,0,200);

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
        itm->setBackgroundColor( clr );
        itm->setTextAlignment( textAlignment );
        setItem(rowIdx , i, itm );
    }
}

int ListWithColumns::getSelectedRow() const {
    QList<QTableWidgetItem *> selItms = selectedItems();
    if (selItms.size() == 0)
        return -1;

    return selItms.front()->row();
}


//#include "listwithcolumns.moc"



