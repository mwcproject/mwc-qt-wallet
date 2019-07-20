#ifndef C_PROGRESSWND_H
#define C_PROGRESSWND_H

#include <QWidget>

namespace Ui {
class ProgressWnd;
}

namespace wnd {

class ProgressWnd;

// Cancell callback interface. Because of usage we are fine withinterface, no needs in callback function
class IProgressWndState {
public:
    virtual void cancelProgress() = 0;
    virtual void destroyProgressWnd(ProgressWnd * w) = 0;
};


class ProgressWnd : public QWidget
{
    Q_OBJECT

public:
    // Empty string - hide this item, null - hidden
    explicit ProgressWnd(QWidget *parent, IProgressWndState * state, QString header, QString msgProgress, QString msgPlus, bool cancellable );
    virtual ~ProgressWnd() override;

    void setHeader(QString header);
    void setMsgPlus(QString msgPlus);

    void initProgress(int min, int max);
    void updateProgress(int pos, QString msgProgress);

private slots:
    void on_cancelButton_clicked();

private:
    Ui::ProgressWnd *ui;
    IProgressWndState * state;
};

}


#endif // C_PROGRESSWND_H
