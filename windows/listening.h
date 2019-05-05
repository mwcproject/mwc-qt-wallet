#ifndef LISTENING_H
#define LISTENING_H

#include <QWidget>

namespace Ui {
class Listening;
}

namespace wnd {

class Listening : public QWidget
{
    Q_OBJECT

public:
    explicit Listening(QWidget *parent = nullptr);
    ~Listening();

private slots:
    void on_nextAddressButton_clicked();

    void on_mwcAddressFromIndexButton_clicked();

private:
    Ui::Listening *ui;
};

}

#endif // LISTENING_H
