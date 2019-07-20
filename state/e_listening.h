#ifndef LISTENING_H
#define LISTENING_H

#include "state.h"
#include "../wallet/wallet.h"

namespace wnd {
    class Listening;
}

namespace state {

class Listening : public QObject, public State
{
    Q_OBJECT
public:
    Listening( StateContext * context);
    virtual ~Listening() override;

    void wndIsGone(wnd::Listening * w) { if(w==wnd) wnd = nullptr;} // window is closed


    void triggerMwcState();

    void requestNextMwcMqAddress();

    void requestNextMwcMqAddressForIndex(int idx);

    void triggerKeybaseState();

protected:
    virtual NextStateRespond execute() override;

private slots:
    void onMwcMqListenerStatus(bool online);
    void onKeybaseListenerStatus(bool online);

    // Listening, you will not be able to get a results
    void onListeningStartResults( bool mqTry, bool kbTry, // what we try to start
                                   QStringList errorMessages ); // error messages, if get some

    void onListeningStopResult(bool mqTry, bool kbTry, // what we try to stop
                                QStringList errorMessages );

    void onMwcAddressWithIndex(QString mwcAddress, int idx);

private:
    wnd::Listening * wnd = nullptr;
};

}

#endif // LISTENING_H
