#ifndef CREATEWITHSEED_H
#define CREATEWITHSEED_H

#include "state.h"
#include <QString>
#include <QPair>
#include <QObject>
#include <QVector>

namespace wnd {
class ProgressWnd;
}

namespace state {


class CreateWithSeed: public QObject, public State
{
    Q_OBJECT
public:
    CreateWithSeed(const StateContext & context);
    virtual ~CreateWithSeed() override;

    // Second Step, switching to the progress and starting this process at mwc713
    void createWalletWithSeed( QVector<QString> seed );

    void cancel();
protected:
    virtual NextStateRespond execute() override;

private slots:
    void onListeningStopResult(bool mqTry, bool kbTry, // what we try to stop
                               QStringList errorMessages );

    // Recovery progress and a status
    void onRecoverProgress( int progress, int maxVal );
    void onRecoverResult(bool started, bool finishedWithSuccess, QString newAddress, QStringList errorMessages);

private:
    wnd::ProgressWnd * progressWnd = nullptr; // Active progress Wnd
    QMetaObject::Connection connListeningStopResult;
    QMetaObject::Connection connRecoverProgress;
    QMetaObject::Connection connRecoverResult;

    bool mwcMqOriginalState = false;
    bool keybaseOriginalState = false;
    QVector<QString>    seed2recover;
    QString             pass2recover;
    int progressMaxVal = 0;
};


}

#endif // CREATEWITHSEED_H
