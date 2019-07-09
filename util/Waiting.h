#ifndef MWC_QT_WALLET_WAITING_H
#define MWC_QT_WALLET_WAITING_H

namespace util {

// When class created, it sswitch to waiting cursor.
// At destructor - restore the cursor back
// Note: usage expected at automatic object only
class Waiting {
public:
    Waiting();
   ~Waiting();
};

}

#endif //MWC_QT_WALLET_WAITING_H
