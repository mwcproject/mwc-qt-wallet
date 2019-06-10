#ifndef GUI_WALLET_MWCLABELPROGRESS_H
#define GUI_WALLET_MWCLABELPROGRESS_H

#include <QLabel>

namespace control {

// Progress movie control
class MwcLabelProgress : public QLabel {
Q_OBJECT
public:
    explicit MwcLabelProgress(QWidget * parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags() );
    explicit MwcLabelProgress(const QString &text, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~MwcLabelProgress() override;

    void initLoader(bool visible);

private:
};

}

#endif //GUI_WALLET_MWCLABELPROGRESS_H
