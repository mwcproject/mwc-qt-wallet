#ifndef CONTACTS_H
#define CONTACTS_H

#include <QWidget>

namespace Ui {
class Contacts;
}

namespace wnd {

class Contacts : public QWidget
{
    Q_OBJECT

public:
    explicit Contacts(QWidget *parent = nullptr);
    ~Contacts();

private:
    Ui::Contacts *ui;
};

}

#endif // CONTACTS_H
