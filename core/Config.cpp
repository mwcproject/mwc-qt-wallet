#include "Config.h"

namespace core {

QString Config::mwc713conf;
QString Config::mwcGuiWalletConf;
QString Config::mwcPath;
QString Config::wallet713path;
QString Config::mainStyleSheetPath;
QString Config::dialogsStyleSheetPath;


// Note, workflow for config not enforced. Please don't abuse it
QString Config::getMwc713conf() {return mwc713conf;}
QString Config::getMwcGuiWalletConf() {return mwcGuiWalletConf;}

QString Config::getWallet713path() {return wallet713path;}
QString Config::getMainStyleSheetPath() {return mainStyleSheetPath;}
QString Config::getDialogsStyleSheetPath() {return dialogsStyleSheetPath;}

QString Config::toString() {
    return "mwc713conf=" + mwc713conf + "\n" +
            "mwcGuiWalletConf=" + mwcGuiWalletConf + "\n" +
            "mwcPath=" + mwcPath + "\n" +
            "wallet713path=" + wallet713path + "\n" +
            "mainStyleSheetPath=" + mainStyleSheetPath + "\n" +
            "dialogsStyleSheetPath=" + dialogsStyleSheetPath + "\n";
}

void Config::setMwc713conf( QString conf ) {
    mwc713conf = conf;
}
void Config::setMwcGuiWalletConf( QString conf ) {
    mwcGuiWalletConf = conf;
}

void Config::setConfigData(QString _mwcPath, QString _wallet713path,
                              QString _mainStyleSheetPath, QString _dialogsStyleSheetPath) {
    mwcPath = _mwcPath;
    wallet713path = _wallet713path;
    mainStyleSheetPath = _mainStyleSheetPath;
    dialogsStyleSheetPath = _dialogsStyleSheetPath;
}

}