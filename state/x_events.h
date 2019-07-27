#ifndef EVENTS_H
#define EVENTS_H

#include "state.h"
#include "../wallet/wallet.h"
#include <QVector>

namespace wnd {
class Events;
}

namespace state {

class Events : public QObject, public State
{
    Q_OBJECT
public:
    Events( StateContext * context );
    virtual ~Events() override;

    void deleteEventsWnd(wnd::Events * w);

    QVector<int> getColumnsWidhts() const;
    void updateColumnsWidhts(const QVector<int> & widths);

    QVector<wallet::WalletNotificationMessages> getWalletNotificationMessages();
    int64_t getWatermarkTime() const {return messageWaterMark;}

    // Check if some error/warnings need to be shown
    bool hasNonShownWarnings() const;
public:
signals:
    void updateNonShownWarnings(bool hasNonShownWarns);

private slots:
    void onNewNotificationMessage(wallet::WalletNotificationMessages::LEVEL level, QString message);

protected:
    virtual NextStateRespond execute() override;

private:
    wnd::Events *   wnd = nullptr;
    int64_t         messageWaterMark = 0;
};

}

#endif // EVENTS_H
