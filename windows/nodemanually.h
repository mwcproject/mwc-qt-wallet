#ifndef NODEMANUALLY_H
#define NODEMANUALLY_H

#include <QWidget>

namespace Ui {
class NodeManually;
}

namespace wnd {

class NodeManually : public QWidget
{
    Q_OBJECT

public:
    explicit NodeManually(QWidget *parent);
    virtual ~NodeManually() override;

    bool validateData();

private slots:
    void on_testConnectionButton_clicked();

private:
    Ui::NodeManually *ui;
};

}

#endif // NODEMANUALLY_H
