#include "mwcdialog.h"
#include <QFile>
#include <QTextStream>
#include <QMouseEvent>
#include <QMessageBox>
#include <QDebug>
#include <QApplication>
#include "../core/Config.h"

namespace control {

MwcDialog::MwcDialog( QWidget * parent )
{
    { // Apply style sheet
        QFile file( config::getDialogsStyleSheetPath() );
        if (file.open(QFile::ReadOnly | QFile::Text)) {
               QTextStream ts( &file );
               setStyleSheet( ts.readAll() );
        }
        else {
            qDebug() << "Failed to read Dialogs stylesheet";
            QMessageBox::critical(parent, "Error", "Unable to read dialogs stylesheet");
            QApplication::quit();
        }
    }

     setWindowFlags(Qt::FramelessWindowHint);
     setModal(true);
}

MwcDialog::~MwcDialog() {}


void MwcDialog::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        cursor = event->globalPos() - geometry().topLeft();
        event->accept();
    }
}

void MwcDialog::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        move(event->globalPos() - cursor);
        event->accept();
    }
}

}
