QT += quick
QT += svg
QT += androidextras

CONFIG += c++11

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

HEADERS += qtandroidservice.h
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
    android/res/values/libs.xml \
    android/src/com/mwc/mobile/ActivityUtils.java \
    android/src/com/mwc/mobile/QtAndroidService.java

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
