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

    void setTextAlignment(int alignment) { textAlignment = alignment; }
    void setHightlightColors(QColor low, QColor hi) {selectedLow=low; selectedHi=hi;}
    // Alpha delta for row stripe coloring. Range 0-255
    void setStripeAlfaDelta( int alpha ) { bkColor2.setAlpha(alpha); }

    void setColumnWidths(QVector<int> widths);
    QVector<int> getColumnWidths() const;

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
};

#endif // LISTWITHCOLUMNS_H
