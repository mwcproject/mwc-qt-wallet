#ifndef GLOBALMWC_H
#define GLOBALMWC_H

#include <QObject>

class QApplication;
class QMainWindow;

namespace mwc {

const QString VERSION = "v1.05";
const QString APP_NAME = "mwc wallet";

// MWC version that we are expecting to ship
const QString WMC_713_VERSION("2.0.2");


const QString PROMPTS_MWC713 = "<<+)mwc713(+>>";

void setApplication(QApplication * app, QMainWindow * mainWindow);

void closeApplication();

}


#endif // GLOBALMWC_H
