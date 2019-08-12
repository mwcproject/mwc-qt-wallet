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

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14

macx {
    CONFIG += app_bundle
    ICON = mw-logo.icns

#mySetOfExtraFiles.files = $$PWD/imagesFolder
#mySetOfExtraFiles.path = Contents/Resources
#QMAKE_BUNDLE_DATA += mySetOfExtraFiles

    OBJECTIVE_SOURCES += macos/changetitlebarcolor.mm
    LIBS +=        -framework AppKit
}


SOURCES += $$files(*.cpp, true)
SOURCES -= $$files(out/*.cpp, true)
SOURCES -= $$files(VS/*.cpp, true)

HEADERS += $$files(*.h, true)
HEADERS -= $$files(out/*.h, true)
HEADERS -= $$files(VS/*.h, true)

FORMS += $$files(*.ui, true)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES += \
    CMakeLists.txt \
    mw-logo.icns
