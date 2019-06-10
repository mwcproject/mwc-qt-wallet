#ifndef MWCDIALOG_H
#define MWCDIALOG_H

#include <QDialog>

namespace control {

class MwcDialog : public QDialog
{
    Q_OBJECT
public:
    MwcDialog( QWidget * parent );
    virtual ~MwcDialog() override;

protected:
    // We don't have caption. So let's be movable without it
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

private:
    QPoint cursor;
};

}


#endif // MWCDIALOG_H
