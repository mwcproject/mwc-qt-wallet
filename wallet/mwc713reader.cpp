#include "mwc713reader.h"
#include <QProcess>
#include <QDebug>
#include "../util/ioutils.h"
#include "../tries/mwc713inputparser.h"

namespace wallet {

void Mwc713reader::run() {
    while(running) {
        if ( mwcProcess->waitForReadyRead(MWC713_READER_WAITING_PERIOD_MS) ) {
            QString str( ioutils::FilterEscSymbols( mwcProcess->readAllStandardOutput() ) );
            qDebug() << "Get output:" << str;
            parser->processInput(str);
        }
    }
}


}

