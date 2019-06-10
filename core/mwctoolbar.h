#ifndef MWCTOOLBAR_H
#define MWCTOOLBAR_H

#include <QWidget>

namespace Ui {
class MwcToolbar;
}

namespace core {


class MwcToolbar : public QWidget
{
   Q_OBJECT

public:
    explicit MwcToolbar(QWidget *parent = nullptr);
    ~MwcToolbar();

protected:
    virtual void paintEvent(QPaintEvent *) override;

private:
    Ui::MwcToolbar *ui;
};

}

#endif // MWCTOOLBAR_H
