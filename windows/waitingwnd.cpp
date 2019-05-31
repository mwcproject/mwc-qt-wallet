#include "waitingwnd.h"
#include "ui_waitingwnd.h"
#include <QMovie>
#include <QLabel>

namespace wnd {

WaitingWnd::WaitingWnd(QWidget *parent, QString title, QString progressMessage) :
    QWidget(parent),
    ui(new Ui::WaitingWnd)
{
    ui->setupUi(this);

    QLabel *lbl = ui->progress;
    QMovie *movie = new QMovie(":/img/loader.gif", QByteArray(), this);
    lbl->setMovie(movie);
    movie->start();

    updateTitle(title);
    updateProgressMessage(progressMessage);
}

WaitingWnd::~WaitingWnd()
{
    delete ui;
}

void WaitingWnd::updateTitle(QString title) {
    ui->titleLabel->setText("<html><body><p><span style=\" font-size:18pt; font-weight:600;\">" + title + "</span></p></body></html>");
}

void WaitingWnd::updateProgressMessage( QString message ) {
    ui->progressMsg->setText(message);
}

}
