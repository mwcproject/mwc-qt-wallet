#ifndef ANDROIDINTERFACE_B_H
#define ANDROIDINTERFACE_B_H

#include <QObject>
#include <QVariantMap>

class AndroidInterface : public QObject
{
    Q_OBJECT
public:
    AndroidInterface();
    Q_INVOKABLE QVariant getApkVersion();
};

#endif // ANDROIDINTERFACE_B_H
