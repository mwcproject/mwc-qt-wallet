// Copyright 2019 The MWC Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
        QFile file(":/resource/dialogs_style.css");
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
        cursor = QPoint(0,0);

        QPoint pos = event->globalPos();
        QRect rc = geometry();
        rc = rc.marginsRemoved( QMargins(5,5,5,5) );
        if ( rc.contains(pos) ) {
            cursor = event->globalPos() - geometry().topLeft();
            event->accept();
        }
    }
}

void MwcDialog::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        if (cursor.x()!=0 && cursor.y()!=0 ) {
            move(event->globalPos() - cursor);
            event->accept();
        }
    }
    else {
        cursor = QPoint(0,0);
    }
}

}
