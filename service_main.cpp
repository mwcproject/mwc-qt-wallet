#include <QDebug>
#include <QAndroidService>

int main(int argc, char *argv[])
{
    qWarning() << "Service starting with BroadcastReceiver from separate .so file";
    QAndroidService app(argc, argv);

    return app.exec();
}
