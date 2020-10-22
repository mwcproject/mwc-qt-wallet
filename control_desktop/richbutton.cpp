// Copyright 2020 The MWC Developers
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

#include "richbutton.h"

namespace control {

RichButton::RichButton(QWidget * parent, QString text, int cx, int cy, QString tooltip, int fontSize) :
    QPushButton(parent)
{
    // Lock the size...
    setMaximumSize( cx,cy );
    setMinimumSize( cx,cy );

    setText(text);

    setToolTip(tooltip);

    QString style = "QPushButton"
                    " {"
                    " color: white;"
                    " font-family: Open Sans;"
                    " font-style: normal;"
                    " font-weight: normal;"
                    " border: 1px solid #FFFFFF;"
                    " border-radius: 3px;"
                    " font-size: "+QString::number(fontSize)+"px;"
                    " background: transparent;"
                    " }"
                    " QPushButton:enabled:!focus::hover"
                    " {"
                    " background-color: rgba(255, 255, 255, 0.15);"
                    " }"
                    "  QPushButton:focus {"
                    " background: #FFFFFF;"
                    " color: #6F00D6;"
                    " }"
                    " QPushButton:pressed {"
                    " background: rgba(255, 255, 255, 0.3);"
                    "}";

    setStyleSheet(style);

    QObject::connect(this, &RichButton::clicked,
                     this, &RichButton::onClicked, Qt::QueuedConnection);
}

RichButton * RichButton::setCallback( RichButtonPressCallback * _pressCallback, const QString & _cookie ) {
    pressCallback = _pressCallback;
    cookie = _cookie;
    return this;
}

void RichButton::onClicked(bool checked) {
    Q_UNUSED(checked);
    if (pressCallback!= nullptr) {
        pressCallback->richButtonPressed(this, cookie);
    }
}


}


