# MWC QT Wallet project for QtCreator

QT       += core gui widgets network svg
requires(qtConfig(completer))

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mwc-qt-wallet
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# It is a desktop build project
DEFINES += WALLET_DESKTOP

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

macx {
    CONFIG += app_bundle
    ICON = mw-logo.icns

#mySetOfExtraFiles.files = $$PWD/imagesFolder
#mySetOfExtraFiles.path = Contents/Resources
#QMAKE_BUNDLE_DATA += mySetOfExtraFiles

    OBJECTIVE_SOURCES += macos/changetitlebarcolor.mm
    LIBS +=        -framework AppKit
}


SOURCES += $$files(*.cpp)
SOURCES += $$files(control_desktop/*.cpp)
SOURCES += $$files(core/*.cpp)
SOURCES += $$files(core_desktop/*.cpp)
SOURCES += $$files(state/*.cpp)
SOURCES += $$files(bridge/*.cpp)
SOURCES += $$files(bridge/wnd/*.cpp)
SOURCES += $$files(tries/*.cpp)
SOURCES += $$files(util/*.cpp)
SOURCES += $$files(util_desktop/*.cpp)
SOURCES += $$files(wallet/*.cpp)
SOURCES += $$files(node/*.cpp)
SOURCES += $$files(wallet/tasks/*.cpp)
SOURCES += $$files(windows_desktop/*.cpp)
SOURCES += $$files(dialogs_desktop/*.cpp)
SOURCES += $$files(tests/*.cpp)
SOURCES += $$files(misk/*.cpp)

SOURCES -= $$files(out/*.cpp, true)
SOURCES -= $$files(VS/*.cpp, true)

HEADERS += $$files(control_desktop/*.h)
HEADERS += $$files(core/*.h)
HEADERS += $$files(core_desktop/*.h)
HEADERS += $$files(state/*.h)
HEADERS += $$files(bridge/*.h)
HEADERS += $$files(bridge/wnd/*.h)
HEADERS += $$files(tries/*.h)
HEADERS += $$files(util/*.h)
HEADERS += $$files(util_desktop/*.h)
HEADERS += $$files(wallet/*.h)
HEADERS += $$files(node/*.h)
HEADERS += $$files(wallet/tasks/*.h)
HEADERS += $$files(windows_desktop/*.h)
HEADERS += $$files(dialogs_desktop/*.h)
HEADERS += $$files(tests/*.h)
HEADERS += $$files(misk/*.h)

HEADERS -= $$files(out/*.h, true)
HEADERS -= $$files(VS/*.h, true)

FORMS += $$files(*.ui, true)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources_desktop.qrc

DISTFILES += \
    mw-logo.icns
