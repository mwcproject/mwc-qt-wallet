#ifndef NEWSEED_H
#define NEWSEED_H

#include "state.h"
#include <QObject>

namespace state {


class NewSeed : public QObject, public State
{
public:
    NewSeed(StateContext * context);
    virtual ~NewSeed() override;

protected:
    virtual NextStateRespond execute() override;

protected slots:
    void onNewSeed(QVector<QString> seed);
private:
};

}

#endif // NEWSEED_H
