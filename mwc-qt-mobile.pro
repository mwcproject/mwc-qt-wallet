QT += quick
QT += svg
QT += androidextras

CONFIG += c++11
CONFIG += lrelease embed_translations

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# It is a mobile build project
DEFINES += WALLET_MOBILE

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += $$files(*.cpp)
SOURCES += $$files(core/*.cpp)
SOURCES += $$files(core_mobile/*.cpp)
SOURCES += $$files(state/*.cpp)
SOURCES += $$files(bridge/*.cpp)
SOURCES += $$files(bridge/wnd/*.cpp)
SOURCES += $$files(tries/*.cpp)
SOURCES += $$files(util/*.cpp)
SOURCES += $$files(wallet/*.cpp)
SOURCES += $$files(node/*.cpp)
SOURCES += $$files(wallet/tasks/*.cpp)

SOURCES -= $$files(out/*.cpp, true)
SOURCES -= $$files(VS/*.cpp, true)

HEADERS += $$files(core/*.h)
HEADERS += $$files(core_mobile/*.h)
HEADERS += $$files(state/*.h)
HEADERS += $$files(bridge/*.h)
HEADERS += $$files(bridge/wnd/*.h)
HEADERS += $$files(tries/*.h)
HEADERS += $$files(util/*.h)
HEADERS += $$files(wallet/*.h)
HEADERS += $$files(node/*.h)
HEADERS += $$files(wallet/tasks/*.h)

HEADERS -= $$files(out/*.h, true)
HEADERS -= $$files(VS/*.h, true)

RESOURCES += resources_mobile.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/local.properties \
    android/res/drawable-anydpi/ic_action_name.xml \
    android/res/drawable-hdpi/ic_action_name.png \
    android/res/drawable-mdpi/ic_action_name.png \
    android/res/drawable-v24/ic_launcher_background.xml \
    android/res/drawable-xhdpi/ic_action_name.png \
    android/res/drawable-xxhdpi/ic_action_name.png \
    android/res/drawable/ic_launcher_foreground.xml \
    android/res/drawable/ic_mwc_logo.xml \
    android/res/mipmap-anydpi-v26/ic_launcher.xml \
    android/res/mipmap-anydpi-v26/ic_launcher_round.xml \
    android/res/mipmap-hdpi/ic_launcher.png \
    android/res/mipmap-hdpi/ic_launcher_round.png \
    android/res/mipmap-mdpi/ic_launcher.png \
    android/res/mipmap-mdpi/ic_launcher_round.png \
    android/res/mipmap-xhdpi/ic_launcher.png \
    android/res/mipmap-xhdpi/ic_launcher_round.png \
    android/res/mipmap-xxhdpi/ic_launcher.png \
    android/res/mipmap-xxhdpi/ic_launcher_round.png \
    android/res/mipmap-xxxhdpi/ic_launcher.png \
    android/res/mipmap-xxxhdpi/ic_launcher_round.png \
    android/res/values/libs.xml \
    android/src/com/mwc/mobile/ActivityUtils.java \
    android/src/com/mwc/mobile/QtAndroidService.java \
    android/src/mw/mwc/wallet/QmlHelper.java \

TRANSLATIONS += \
    i18n/mwc_base.ts \
    i18n/mwc_en.ts \
    i18n/mwc_fr.ts \
    i18n/mwc_he.ts \
    i18n/mwc_id.ts \
    i18n/mwc_it.ts \
    i18n/mwc_ja.ts \
    i18n/mwc_ko.ts \
    i18n/mwc_pt.ts \
    i18n/mwc_ru.ts \
    i18n/mwc_vi.ts \
    i18n/mwc_zh.ts

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

ANDROID_EXTRA_LIBS = $$PWD/android_bin/x86/libmwc713.so $$PWD/android_bin/x86/libmwczip.so $$PWD/android_bin/x86/libtor.so \
    $$PWD/android_bin/x86_64/libmwc713.so       $$PWD/android_bin/x86_64/libmwczip.so       $$PWD/android_bin/x86_64/libtor.so \
    $$PWD/android_bin/armeabi-v7a/libmwc713.so  $$PWD/android_bin/armeabi-v7a/libmwczip.so  $$PWD/android_bin/armeabi-v7a/libtor.so \
    $$PWD/android_bin/arm64-v8a/libmwc713.so    $$PWD/android_bin/arm64-v8a/libmwczip.so    $$PWD/android_bin/arm64-v8a/libtor.so

ANDROID_ABIS = armeabi-v7a arm64-v8a x86 x86_64

