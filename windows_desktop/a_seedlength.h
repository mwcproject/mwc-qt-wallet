#ifndef A_SEEDLENGTH_H
#define A_SEEDLENGTH_H

#include <QWidget>

namespace Ui {
class SeedLength;
}

namespace bridge {
class InitAccount;
}


namespace wnd {

class SeedLength : public QWidget {
Q_OBJECT

public:
    explicit SeedLength(QWidget *parent = nullptr);

    ~SeedLength();

private slots:
    void on_continueButton_clicked();
private:
    Ui::SeedLength *ui;
    bridge::InitAccount * initAccount = nullptr;

};

}

#endif // A_SEEDLENGTH_H
