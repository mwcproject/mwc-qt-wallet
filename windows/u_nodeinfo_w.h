#ifndef U_NODEINFO_H
#define U_NODEINFO_H

#include "../core/navwnd.h"

namespace Ui {
class NodeInfo;
}

namespace state {
class NodeInfo;
}

namespace wnd {

class NodeInfo : public core::NavWnd {
Q_OBJECT

public:
    explicit NodeInfo(QWidget *parent, state::NodeInfo * state);

    ~NodeInfo();

    void setNodeStatus( bool online, QString errMsg, int height, int64_t totalDifficulty, int connections );

private slots:

    void on_refreshButton_clicked();

    void on_chnageNodeButton_clicked();

private:
    Ui::NodeInfo *ui;
    state::NodeInfo * state;
};

}

#endif // U_NODEINFO_H
