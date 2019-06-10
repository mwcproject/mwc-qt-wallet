#ifndef GUI_WALLET_CONFIGREADER_H
#define GUI_WALLET_CONFIGREADER_H

#include <QMap>

namespace util {

class ConfigReader {
public:
    ConfigReader();

    // Config is reading toml format. We are persing it manually, so details might be different from toml
    bool readConfig(QString confPath);

    bool isDefined(QString key) const;
    QString getString(QString key) const;

private:

    QMap<QString, QString>  config;
};

}

#endif //GUI_WALLET_CONFIGREADER_H
