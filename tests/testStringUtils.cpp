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

#include "testStringUtils.h"
#include "../util/stringutils.h"

namespace test {

// just run tests with asserts and qDebug. Nothing fancy fpr now
void testLongLong2ShortStr() {
    using namespace util;

    // test for short numbers
    Q_ASSERT( longLong2ShortStr( 0, 9 ) == "0" );
    Q_ASSERT( longLong2ShortStr( -1000, 9 ) == "-1,000" );
    Q_ASSERT( longLong2ShortStr( 1123123, 9 ) == "1,123,123" );
    Q_ASSERT( longLong2ShortStr( 12312312, 9 ) == "12.312 M" );
    Q_ASSERT( longLong2ShortStr( 123123123, 9 ) == "123.123 M" );

    Q_ASSERT( longLong2ShortStr( 1231231235, 9 ) == "1,231.2 M" );
    Q_ASSERT( longLong2ShortStr( 1231201235, 9 ) == "1,231.2 M" );
    Q_ASSERT( longLong2ShortStr( 1231001235, 9 ) == "1,231 M" );
    Q_ASSERT( longLong2ShortStr( 123103123, 9 ) == "123.103 M" );
    Q_ASSERT( longLong2ShortStr( 123100123, 9 ) == "123.1 M" );
    Q_ASSERT( longLong2ShortStr( 123000123, 9 ) == "123 M" );

    Q_ASSERT( longLong2ShortStr( 12310312355l, 9 ) == "12,310 M" );
    Q_ASSERT( longLong2ShortStr( 123103123555l, 9 ) == "123,103 M" );
    Q_ASSERT( longLong2ShortStr( 1231031235556l, 9 ) == "1,231 B" );
    Q_ASSERT( longLong2ShortStr( 1231231235556l, 9 ) == "1,231.2 B" );

    Q_ASSERT( longLong2ShortStr( -1231231235556l, 9 ) == "-1,231 B" );
    Q_ASSERT( longLong2ShortStr(  12312312355566l, 9 ) == "12,312 B" );

    Q_ASSERT( longLong2ShortStr( 1231231235556667l, 9 ) == "1,231.2 T" );
    Q_ASSERT( longLong2ShortStr( 12312312355566677l, 9 )== "12,312 T" );

    Q_ASSERT( longLong2ShortStr( 1231231235556667778l, 9 )== "1,231.2 Q" );

    Q_ASSERT( longLong2ShortStr( 1031231235556667778l, 8 )== "1,031 Q" );

}

void testUtils() {
    Q_ASSERT( util::urlEncode("abc") == "abc");
    Q_ASSERT( util::urlEncode("а") == "%D0%B0");
    Q_ASSERT( util::urlEncode("%abcа") == "%25abc%D0%B0");
}

}
