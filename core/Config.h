#ifndef GUI_WALLET_CONFIG_H
#define GUI_WALLET_CONFIG_H

#include <QString>

namespace core {

// Note, workflow for config not enforced. Please don't abuse it
class Config {
public:
    static const QString & getMwc713conf();
    static const QString & getMwcGuiWalletConf();

    static const QString & getMwcpath(); // Not using now because so far there is no local node is running
    static const QString & getWallet713path();
    static const QString & getMainStyleSheetPath();
    static const QString & getDialogsStyleSheetPath();
    static const QString & getAirdropUrl();

    static QString toString();
public:
    static void setMwc713conf( QString conf );
    static void setMwcGuiWalletConf( QString conf );

    static void setConfigData(QString mwcPath, QString wallet713path,
            QString mainStyleSheetPath, QString dialogsStyleSheetPath, QString airdropUrl);

private:
    static QString mwc713conf;
    static QString mwcGuiWalletConf;
    static QString mwcPath;
    static QString wallet713path;
    static QString mainStyleSheetPath;
    static QString dialogsStyleSheetPath;
    static QString airdropUrl;
};

}

#endif //GUI_WALLET_CONFIG_H
