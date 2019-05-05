#ifndef PASSWORDFORSEED_H
#define PASSWORDFORSEED_H

#include <QWidget>

namespace Ui {
class PasswordForSeed;
}

namespace wnd {

class PasswordForSeed : public QWidget
{
    Q_OBJECT

public:
    explicit PasswordForSeed(QWidget *parent);
    virtual ~PasswordForSeed() override;

    bool validateData();

private slots:
    void on_password1Edit_textChanged(const QString &arg1);

private:
    Ui::PasswordForSeed *ui;
};

}

#endif // PASSWORDFORSEED_H
