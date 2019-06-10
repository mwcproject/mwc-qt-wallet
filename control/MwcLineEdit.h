#ifndef GUI_WALLET_MWCLINEEDIT_H
#define GUI_WALLET_MWCLINEEDIT_H

#include <QLineEdit>

namespace control {

class MwcLineEditNormal : public QLineEdit {
Q_OBJECT
public:
    explicit MwcLineEditNormal(QWidget *parent = Q_NULLPTR);
    explicit MwcLineEditNormal(const QString & t, QWidget *parent = Q_NULLPTR);
    virtual ~MwcLineEditNormal() override;
};

}

#endif //GUI_WALLET_MWCLINEEDIT_H
