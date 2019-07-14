#include "Config.h"

namespace core {

QString Config::mwc713conf;
QString Config::mwcGuiWalletConf;
QString Config::mwcPath;
QString Config::wallet713path;
QString Config::mainStyleSheetPath;
QString Config::dialogsStyleSheetPath;
QString Config::airdropUrl;


// Note, workflow for config not enforced. Please don't abuse it
const QString & Config::getMwc713conf() {return mwc713conf;}
const QString & Config::getMwcGuiWalletConf() {return mwcGuiWalletConf;}

const QString & Config::getWallet713path() {return wallet713path;}
const QString & Config::getMainStyleSheetPath() {return mainStyleSheetPath;}
const QString & Config::getDialogsStyleSheetPath() {return dialogsStyleSheetPath;}
const QString & Config::getAirdropUrl() {return airdropUrl;}

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
                              QString _mainStyleSheetPath, QString _dialogsStyleSheetPath, QString _airdropUrl) {
    mwcPath = _mwcPath;
    wallet713path = _wallet713path;
    mainStyleSheetPath = _mainStyleSheetPath;
    dialogsStyleSheetPath = _dialogsStyleSheetPath;
    airdropUrl = _airdropUrl;
}

}