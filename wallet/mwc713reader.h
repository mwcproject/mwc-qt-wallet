#ifndef MWC713READER_H
#define MWC713READER_H

#include <QThread>

class QProcess;

namespace tries {
    class Mwc713InputParser;
}

namespace wallet {

const int MWC713_READER_WAITING_PERIOD_MS = 1000;

class Mwc713State;

class Mwc713reader : public QThread
{
    Q_OBJECT
public:
    Mwc713reader(QProcess * _mwcProcess, tries::Mwc713InputParser * _parser) : mwcProcess(_mwcProcess), parser(_parser) {}
    virtual ~Mwc713reader() override {}

    void stopReader() {running=false;}
protected:
    virtual void run() override;
signals:
    void newStateReport(Mwc713State * state);

private:
    QProcess * mwcProcess = nullptr;
    tries::Mwc713InputParser * parser = nullptr;
    volatile bool running = true;
};

}


#endif // MWC713READER_H
