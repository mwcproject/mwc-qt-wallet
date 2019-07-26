#ifndef MWC_QT_WALLET_MWCCOMBOBOX_H
#define MWC_QT_WALLET_MWCCOMBOBOX_H

#include <QComboBox>

namespace control {

class MwcComboBox : public QComboBox {
    Q_OBJECT
public:
    explicit MwcComboBox(QWidget *parent = nullptr);

    //On Windows this is not needed as long as the combobox is editable
    //This is untested since I don't have Linux
    void showPopup();
};

}

#endif //MWC_QT_WALLET_MWCCOMBOBOX_H
