#include "listwithcolumns.h"
#include <QHeaderView>
#include <QApplication>

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
    verticalHeader()->setDefaultSectionSize(30);
    // Increase V size so

    horizontalHeader()->setFixedHeight( 30 );

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
    bkColor1 = QColor(255,255,255,0);
    bkColor2 = QColor(255,255,255,20);

    selectedLow = QColor(255,0xED,0,90); // Yellow color
    selectedHi = QColor(255,0xED,0,200);

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

void ListWithColumns::appendRow( const QVector<QString> & rowData, SELECTION selection ) {
    Q_ASSERT(rowData.size() == columnCount() );

    int rowIdx = rowCount();

    QColor clr;
    switch (selection) {
    case SELECTION::SELECT_LOW:
        clr = selectedLow;
        if (rowIdx % 2 == 0)
            clr.setAlpha( clr.alpha() + 20 );
        break;
    case SELECTION::SELECT_HI:
        clr = selectedHi;
        if (rowIdx % 2 == 0)
            clr.setAlpha( clr.alpha() + 20 );
        break;
    default:
        clr = rowIdx % 2 == 0 ? bkColor1 : bkColor2;
        break;
    }

    setRowCount(rowIdx+1);
    int sz = rowData.size();
    for ( int i=0; i<sz; i++ ) {
        auto * itm = new QTableWidgetItem( rowData[i] );
        itm->setBackgroundColor( clr );
        setItem(rowIdx , i, itm );
    }
}

//#include "listwithcolumns.moc"



