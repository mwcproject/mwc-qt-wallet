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

const QString DATETIME_TEMPLATE_MWC713 = "yyyy-MM-dd hh:mm:ss";
const QString DATETIME_TEMPLATE_THIS = "hh:mm:ss dd-MM-yyyy";

const QString DEL_ACCONT_PREFIX = "del_";

#ifdef Q_OS_MACOS
const QString MWC713_DEFAULT_CONFIG = ":/resource/wallet713_mac.toml";
#else
const QString MWC713_DEFAULT_CONFIG = ":/resource/wallet713_def.toml";
#endif

const int64_t AIRDROP_TRANS_KEEP_TIME_MS = 1000L*3600L*24L*15; // 15 days should be enough

void setApplication(QApplication * app, QMainWindow * mainWindow);

void closeApplication();

}


#endif // GLOBALMWC_H
