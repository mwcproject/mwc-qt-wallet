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

#ifndef MIGRATION_H 
#define MIGRATION_H

#include <QObject>
#include "state.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"

namespace state {

class Migration : public QObject, public State
{
    Q_OBJECT

public:
    Migration( StateContext * context);
    virtual ~Migration() override;

private slots:
    void onLogin();
protected:
    virtual NextStateRespond execute() override;
    virtual bool mobileBack() override {return false;}

private:
};

}

#endif // MIGRATION_H
