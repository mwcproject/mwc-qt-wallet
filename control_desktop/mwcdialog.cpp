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
#include "../core/WndManager.h"
#include "../core_desktop/DesktopWndManager.h"
#include "../core_desktop/mainwindow.h"

namespace control {

static QString dlgStyle;

MwcDialog::MwcDialog( QWidget * parent )// : QDialog( getParent(parent) )
{
    if (dlgStyle.isEmpty())
    { // Apply style sheet
        QFile file(":/resource_desktop/dialogs_style.css");
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream ts( &file );
            dlgStyle = ts.readAll();
        }
        else {
            qDebug() << "Failed to read Dialogs stylesheet";
            QMessageBox::critical(parent, "Error", "Unable to read dialogs stylesheet");
            QApplication::quit();
        }
    }

    setStyleSheet( dlgStyle );

    setWindowFlags(Qt::FramelessWindowHint);
    setModal(true);
    // If uncomment - will show one Dlg at a time
    //setWindowModality(Qt::ApplicationModal);
}

MwcDialog::~MwcDialog() {}

const QString & MwcDialog::getDlgStyle() const {
    return dlgStyle;
}

void MwcDialog::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        cursor = QPoint(0,0);

        QPoint pos = event->globalPosition().toPoint();
        QRect rc = geometry();
        rc = rc.marginsRemoved( QMargins(5,5,5,5) );
        if ( rc.contains(pos) ) {
            cursor = event->globalPosition().toPoint() - geometry().topLeft();
            event->accept();
        }
    }
}

void MwcDialog::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        if (cursor.x()!=0 && cursor.y()!=0 ) {
            move(event->globalPosition().toPoint() - cursor);
            event->accept();
        }
    }
    else {
        cursor = QPoint(0,0);
    }
}

void MwcDialog::showEvent(QShowEvent* event)  {
    QDialog::showEvent(event);  // Call base implementation

    core::DesktopWndManager * wndMan = (core::DesktopWndManager*) core::getWndManager();
    core::MainWindow * mainMwn = wndMan->getMainWindow();

    if (mainMwn && mainMwn->isVisible()) {
        // Calculate center position
        QRect mainGeo = mainMwn->frameGeometry();
        QPoint centerPos = mainGeo.center() - QPoint(width()/2, height()/2);

        // Move to center
        move(centerPos);
    }
}

}
