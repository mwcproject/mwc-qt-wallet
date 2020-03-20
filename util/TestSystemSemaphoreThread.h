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

#ifndef MWC_QT_WALLET_TESTSYSTEMSEMAPHORETHREAD_H
#define MWC_QT_WALLET_TESTSYSTEMSEMAPHORETHREAD_H

#include <QThread>

class QSystemSemaphore;

namespace util {

// Very specific class. Make it pulbilly available only because of QT magic.
// QT magic works great only if class has both declaration and implementation in different files.
class TestSystemSemaphoreThread : public QThread {
Q_OBJECT
public:
    TestSystemSemaphoreThread(QSystemSemaphore *semaphore);

protected:
    void run() override;

private:
    QSystemSemaphore *sem2test = nullptr;
};

}

#endif //MWC_QT_WALLET_TESTSYSTEMSEMAPHORETHREAD_H
