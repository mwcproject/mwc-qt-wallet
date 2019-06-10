#include "ConfigReader.h"
#include <QFile>
#include <QTextStream>

namespace util {

ConfigReader::ConfigReader() {}

bool ConfigReader::readConfig(QString confPath) {
    QFile inputFile(confPath);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString line = in.readLine().trimmed();
            if (line.length()<1 || line[0]=='#') // empty or comment
                continue;

            int idx = line.indexOf('=');
            if (idx<=0)
                continue;

            QString key = line.left(idx).trimmed();
            QString value = line.right( line.length()-idx-1 ).trimmed();
            if (key.isEmpty() )
                continue;

            if (value.size()>0 && value[0] == '"')
                value.remove(0,1);
            if (value.size()>0 && value[value.size()-1] == '"')
                value.remove(value.size()-1,1);

            config[key] = value;
        }
        inputFile.close();
    }
    else
        return false;

    return true;
}

bool ConfigReader::isDefined(QString key) const {
    return config.contains(key);
}

QString ConfigReader::getString(QString key) const {
    return config.value(key);
}


}

