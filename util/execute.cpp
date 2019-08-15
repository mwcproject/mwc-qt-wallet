#include "util/execute.h"
#include <QProcess>

namespace util {

void openUrlInBrowser(const QString & url) {
#if defined(Q_OS_MACOS)
    QProcess::startDetached("open " + url);
#elif defined(LINUX)
    QProcess::startDetached("xdg-open " + url);
#elif defined(Q_OS_WIN)
    Q_ASSERT(false); // support me
#else
    Q_ASSERT(false); // unexperted OS.
#endif


}

}


