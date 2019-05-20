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

    void setColumnWidths(QVector<int> widths);
    QVector<int> getColumnWidths() const;

    // clear the data
    void clearData();
    // Caller responsible to setup the columns and be sure that the data match them.
    void appendRow( const QVector<QString> & rowData );

protected:
    void setListLook();

protected:
    // Two background color
    QColor bkColor1; // System color
    QColor bkColor2; // Secondary with modifyed Blue
};

#endif // LISTWITHCOLUMNS_H
