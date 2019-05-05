/*#ifndef CONFIRMSEEDW_H
#define CONFIRMSEEDW_H

#include <QWidget>
#include <QVector>

namespace Ui {
class ConfirmSeed;
}

namespace wnd {

class ConfirmSeed : public QWidget
{
    Q_OBJECT

public:
    explicit ConfirmSeed(QWidget *parent, int wordIndex, QString seedWord );
    virtual ~ConfirmSeed() override;

private:
    Ui::ConfirmSeed *ui;
    QVector<QString> seed2check;
};

}

#endif // CONFIRMSEEDW_H
*/
