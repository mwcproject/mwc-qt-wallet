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

#include "widgetutils.h"
#include <QWidget>
#include <QShortcut>
#include <QTextEdit>
#include <QScrollBar>
#include <QCoreApplication>

namespace utils {

void defineDefaultButtonSlot( QWidget *parent, const char *slot ) {
    // parent will take ownership
    new QShortcut( QKeySequence(Qt::Key_Return), parent, slot );
    new QShortcut( QKeySequence(Qt::Key_Enter), parent, slot );
}

// Just a resize the edit inside Widget to the content will fit
void resizeEditByContent( QWidget * parentWindow, QTextEdit * edit, bool html, const QString & message ) {
    Q_ASSERT(parentWindow);
    Q_ASSERT(edit);

        // Setting text option
        QTextOption textOption;
        textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        textOption.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        edit->document()->setDefaultTextOption(textOption);

        // text as a data should work for as.
        if (html)
            edit->setHtml( "<center>" + message + "</center>" );
        else
            edit->setPlainText(message);

        bool resized = false;

        do {
            // prepare for rendering
            QSize curSz = edit->size();

            // size is the wierdest part. We are renderind document to get a size form it.
            // Document tolk in Pt, so need to convert into px. Conversion is not very accurate
            edit->document()->adjustSize();
            int h = int(curSz.height());
            edit->adjustSize();

            // Second Ajustment with a scroll br that works great
            QScrollBar *vSb = edit->verticalScrollBar();
            int scrollDiff = vSb->maximum() - vSb->minimum();
            int page = vSb->pageStep();

            resized = false;
            if (scrollDiff > 0) {
                h = int(h * double(scrollDiff + page) / page + 1);
                resized = true;
            }

            edit->setMaximumHeight(h);
            edit->setMinimumHeight(h);
            edit->adjustSize();

            parentWindow->adjustSize();

            QCoreApplication::processEvents();
        } while (resized);
}


}

