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

#ifndef G_FINALIZE_H
#define G_FINALIZE_H

#include "../core/navwnd.h"

namespace Ui {
class FinalizeUpload;
}

namespace state {
class Finalize;
}

namespace wnd {

class FinalizeUpload : public core::NavWnd
{
    Q_OBJECT

public:
    explicit FinalizeUpload(QWidget *parent, state::Finalize * state);
    ~FinalizeUpload();

private slots:
    void on_uploadFileBtn_clicked();

private:
    Ui::FinalizeUpload *ui;
    state::Finalize * state = nullptr;
};

}


#endif // G_FINALIZE_H
