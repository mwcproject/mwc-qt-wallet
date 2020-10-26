#ifndef PANELTITLE_H
#define PANELTITLE_H

#include <QWidget>

namespace Ui {
class PanelTitle;
}

namespace core {

class PanelTitle : public QWidget {
Q_OBJECT

public:
    explicit PanelTitle(QWidget *parent);
    ~PanelTitle();

    void setTitle(const QString & title);
private:
    Ui::PanelTitle *ui;
};

void setCurrentTitle(const QString & title);

}

#endif // PANELTITLE_H
