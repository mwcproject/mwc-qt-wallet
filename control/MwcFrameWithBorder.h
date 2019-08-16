#ifndef MWC_QT_WALLET_MWCFRAMEWITHBORDER_H
#define MWC_QT_WALLET_MWCFRAMEWITHBORDER_H

#include <QFrame>

namespace control {


class MwcFrameWithBorder : public QFrame {
    Q_OBJECT
public:
    MwcFrameWithBorder(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
};

class MwcLine : public QFrame {
Q_OBJECT
public:
    MwcLine(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
};


}

#endif //MWC_QT_WALLET_MWCFRAMEWITHBORDER_H
