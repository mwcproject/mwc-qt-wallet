#ifndef CREATEWITHSEED_H
#define CREATEWITHSEED_H

#include "state.h"
#include <QString>
#include <QPair>
#include <QObject>
#include <QVector>
#include "../windows/c_progresswnd.h"

namespace wnd {
class ProgressWnd;
class EnterSeed;
}

namespace state {


class CreateWithSeed: public QObject, public State, public wnd::IProgressWndState
{
    Q_OBJECT
public:
    CreateWithSeed(const StateContext & context);
    virtual ~CreateWithSeed() override;

    void deleteEnterSeed() {seedWnd = nullptr; }
    void deleteProgressWnd() {progressWnd = nullptr; }

    // Second Step, switching to the progress and starting this process at mwc713
    void createWalletWithSeed( QVector<QString> seed );

    void cancel();
protected:
    // IProgressWndState
    virtual void cancelProgress() override {}
    virtual void destroyProgressWnd() override {progressWnd=nullptr;}

    virtual NextStateRespond execute() override;

private slots:
    void onListeningStopResult(bool mqTry, bool kbTry, // what we try to stop
                               QStringList errorMessages );

    // Recovery progress and a status
    void onRecoverProgress( int progress, int maxVal );
    void onRecoverResult(bool started, bool finishedWithSuccess, QString newAddress, QStringList errorMessages);

    void onWalletBalanceUpdated();
private:
    wnd::EnterSeed   * seedWnd = nullptr;
    wnd::ProgressWnd * progressWnd = nullptr; // Active progress Wnd

    bool mwcMqOriginalState = false;
    bool keybaseOriginalState = false;
    QVector<QString>    seed2recover;
    QString             pass2recover;
    int progressMaxVal = 0;
};


}

#endif // CREATEWITHSEED_H
