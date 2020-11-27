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

#ifndef WIDGETUTILS_H
#define WIDGETUTILS_H

class QWidget;
class QTextEdit;
class QString;

namespace utils {

    void defineDefaultButtonSlot( QWidget *parent, const char *slot );

    // Just a resize the edit inside Widget to the content will fit
    void resizeEditByContent( QWidget * parentWindow, QTextEdit * edit, bool html, const QString & message );

}


#endif // WIDGETUTILS_H
