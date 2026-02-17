# MWC QT Wallet project for QtCreator

QT       += core gui widgets network svg concurrent
requires(qtConfig(completer))

greaterThan(QT_MAJOR_VERSION, 6): QT += widgets

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

unix:!macx {
    QMAKE_CXXFLAGS += -include features.h
}

CONFIG += c++14
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

# Rust static library
LIBDIR = $$PWD/../mwc-wallet/target

win32 {
    # The expected build chain is MinGw
    CONFIG(debug, debug|release) {
        LIBS += $$LIBDIR/x86_64-pc-windows-gnu/debug/libmwc_wallet_lib.a
    }
    CONFIG(release, debug|release) {
        LIBS += $$LIBDIR/x86_64-pc-windows-gnu/release/libmwc_wallet_lib.a
    }
    LIBS += -liphlpapi -lnetapi32 -lole32 -loleaut32 -lpropsys -lws2_32 -lntdll
} else {
    CONFIG(debug, debug|release) {
        LIBS += $$LIBDIR/debug/libmwc_wallet_lib.a
#        LIBS += $$LIBDIR/aarch64-apple-darwin/debug/libmwc_wallet_lib.a
    }
    CONFIG(release, debug|release) {
        LIBS += $$LIBDIR/release/libmwc_wallet_lib.a
#        LIBS += $$LIBDIR/aarch64-apple-darwin/release/libmwc_wallet_lib.a
    }
}

# ---- Sanitizers (Debug/ASan config) ----
# QMAKE_CXXFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1
# QMAKE_LFLAGS   += -fsanitize=address,undefined

macx {
    CONFIG += app_bundle
    ICON = mw-logo.icns

#mySetOfExtraFiles.files = $$PWD/imagesFolder
#mySetOfExtraFiles.path = Contents/Resources
#QMAKE_BUNDLE_DATA += mySetOfExtraFiles

    OBJECTIVE_SOURCES += macos/changetitlebarcolor.mm
    LIBS += -framework AppKit
    LIBS += -framework Security
    LIBS += -framework CoreFoundation
    LIBS += -framework SecurityFoundation
    LIBS += -framework SystemConfiguration
    LIBS += -framework CFNetwork
    LIBS += -lsqlite3
    LIBS += -lresolv
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
SOURCES += $$files(wallet/api/*.cpp)
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
HEADERS += $$files(wallet/api/*.h)
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
