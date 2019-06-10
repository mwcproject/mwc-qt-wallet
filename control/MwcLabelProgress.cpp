#include "MwcLabelProgress.h"
#include <QMovie>

namespace control {

MwcLabelProgress::MwcLabelProgress(QWidget *parent, Qt::WindowFlags f) :
        QLabel(parent,f)
{
}

MwcLabelProgress::MwcLabelProgress(const QString &text, QWidget *parent, Qt::WindowFlags f) :
        QLabel(text,parent, f)
{
}

MwcLabelProgress::~MwcLabelProgress() {}

void MwcLabelProgress::initLoader(bool visible) {
    QMovie *movie = new QMovie(":/img/loading.gif", QByteArray(), this);
    setMovie(movie);
    setScaledContents(true);
    movie->start();

    if (visible)
        show();
    else
        hide();
}

}
