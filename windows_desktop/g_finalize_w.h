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

#include "../core_desktop/navwnd.h"

namespace Ui {
class FinalizeUpload;
}

namespace bridge {
class Config;
class Finalize;
}

namespace wnd {

class Finalize : public core::NavWnd
{
    Q_OBJECT

public:
    explicit Finalize(QWidget *parent);
    ~Finalize();

private slots:
    void on_uploadFileBtn_clicked();

private:
    Ui::FinalizeUpload *ui;
    bridge::Config * config = nullptr;
    bridge::Finalize * finalize = nullptr;
};

}


#endif // G_FINALIZE_H
