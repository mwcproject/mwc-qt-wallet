#ifndef GUI_WALLET_CONFIG_H
#define GUI_WALLET_CONFIG_H

#include <QString>

namespace config {

void setMwc713conf( QString conf );
void setMwcGuiWalletConf( QString conf );

void setConfigData(QString mwcPath, QString wallet713path,
                   QString mainStyleSheetPath, QString dialogsStyleSheetPath, QString airdropUrl, int64_t logoutTimeMs);


// Note, workflow for config not enforced. Please don't abuse it
const QString & getMwc713conf();
const QString & getMwcGuiWalletConf();

const QString & getMwcpath(); // Not using now because so far there is no local node is running
const QString & getWallet713path();
const QString & getMainStyleSheetPath();
const QString & getDialogsStyleSheetPath();
const QString & getAirdropUrl();
int64_t         getLogoutTimeMs();

QString toString();


}

#endif //GUI_WALLET_CONFIG_H
