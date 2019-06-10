#ifndef GUI_WALLET_CONFIG_H
#define GUI_WALLET_CONFIG_H

#include <QString>

namespace core {

// Note, workflow for config not enforced. Please don't abuse it
class Config {
public:
    static QString getMwc713conf();
    static QString getMwcGuiWalletConf();

    static QString getMwcpath(); // Not using now because so far there is no local node is running
    static QString getWallet713path();
    static QString getMainStyleSheetPath();
    static QString getDialogsStyleSheetPath();

    static QString toString();
public:
    static void setMwc713conf( QString conf );
    static void setMwcGuiWalletConf( QString conf );

    static void setConfigData(QString mwcPath, QString wallet713path,
            QString mainStyleSheetPath, QString dialogsStyleSheetPath);

private:
    static QString mwc713conf;
    static QString mwcGuiWalletConf;
    static QString mwcPath;
    static QString wallet713path;
    static QString mainStyleSheetPath;
    static QString dialogsStyleSheetPath;
};

}

#endif //GUI_WALLET_CONFIG_H
