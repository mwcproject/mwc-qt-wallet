#ifndef C_PROGRESSWND_H
#define C_PROGRESSWND_H

#include <QWidget>

namespace Ui {
class ProgressWnd;
}

namespace wnd {

// Cancell callback interface. Because of usege we are fine withinterface, no needs in callbacl function
class IProgressWndState {
public:
    virtual void cancelProgress() = 0;
};

class ProgressWnd : public QWidget
{
    Q_OBJECT

public:
    // Empty string - hide this item, null - hidden
    explicit ProgressWnd(QWidget *parent, QString header, QString msgProgress, QString msgPlus, IProgressWndState * callback = nullptr );
    virtual ~ProgressWnd() override;

    void setHeader(QString header);
    void setMsgPlus(QString msgPlus);

    void initProgress(int min, int max);
    void updateProgress(int pos, QString msgProgress);

private slots:
    void on_cancelButton_clicked();

private:
    Ui::ProgressWnd *ui;
    IProgressWndState * cancelCallback;
};

}


#endif // C_PROGRESSWND_H
