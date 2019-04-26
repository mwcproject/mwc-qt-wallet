#-------------------------------------------------
#
# Project created by QtCreator 2019-04-08T18:49:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gui-wallet
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

CONFIG += c++11

macx {
    CONFIG += app_bundle
    ICON = mw-logo.icns

#mySetOfExtraFiles.files = $$PWD/imagesFolder
#mySetOfExtraFiles.path = Contents/Resources
#QMAKE_BUNDLE_DATA += mySetOfExtraFiles
}


SOURCES += \
    main.cpp \
    data/walletdata.cpp \
    mwc_windows/choosewallet.cpp \
    mwc_windows/confirmseed.cpp \
    mwc_windows/connect2server.cpp \
    mwc_windows/newseed.cpp \
    mwc_windows/newwallet.cpp \
    mwc_windows/nodestatus.cpp \
    mwc_windows/passwordforseed.cpp \
    wnd_core/mainwindow.cpp \
    wnd_core/versiondialog.cpp \
    wnd_core/walletwindow.cpp \
    wnd_core/windowmanager.cpp \
    mwc_wallet/wallet.cpp \
    mwc_wallet/mockwallet.cpp \
    data/passwordanalyser.cpp \
    mwc_windows/enterseed.cpp \
    data/stringutils.cpp \
    mwc_windows/fromseedfile.cpp \
    mwc_windows/nodemanually.cpp \
    util/mwcexception.cpp \
    mwc_windows/accounts.cpp \
    mwc_windows/listening.cpp \
    mwc_windows/events.cpp \
    mwc_windows/sendcoins.cpp \
    mwc_windows/filetransactions.cpp \
    mwc_windows/outputs.cpp \
    mwc_windows/transactions.cpp \
    mwc_windows/contacts.cpp \
    mwc_windows/hodl.cpp \
    mwc_windows/claiming.cpp \
    mwc_windows/claimchallendge.cpp

HEADERS += \
    data/walletdata.h \
    mwc_windows/choosewallet.h \
    mwc_windows/confirmseed.h \
    mwc_windows/connect2server.h \
    mwc_windows/newseed.h \
    mwc_windows/newwallet.h \
    mwc_windows/nodestatus.h \
    mwc_windows/passwordforseed.h \
    wnd_core/mainwindow.h \
    wnd_core/versiondialog.h \
    wnd_core/walletwindow.h \
    wnd_core/windowmanager.h \
    mwc_wallet/wallet.h \
    mwc_wallet/mockwallet.h \
    data/passwordanalyser.h \
    mwc_windows/enterseed.h \
    data/stringutils.h \
    mwc_windows/fromseedfile.h \
    mwc_windows/nodemanually.h \
    util/mwcexception.h \
    mwc_windows/accounts.h \
    mwc_windows/listening.h \
    mwc_windows/events.h \
    mwc_windows/sendcoins.h \
    mwc_windows/filetransactions.h \
    mwc_windows/outputs.h \
    mwc_windows/transactions.h \
    mwc_windows/contacts.h \
    mwc_windows/hodl.h \
    mwc_windows/claiming.h \
    mwc_windows/claimchallendge.h

FORMS += \
    mwc_windows/choosewallet.ui \
    mwc_windows/confirmseed.ui \
    mwc_windows/connect2server.ui \
    mwc_windows/newseed.ui \
    mwc_windows/newwallet.ui \
    mwc_windows/nodestatus.ui \
    mwc_windows/passwordforseed.ui \
    wnd_core/mainwindow.ui \
    wnd_core/versiondialog.ui \
    mwc_windows/enterseed.ui \
    mwc_windows/fromseedfile.ui \
    mwc_windows/nodemanually.ui \
    mwc_windows/listening.ui \
    mwc_windows/events.ui \
    mwc_windows/sendcoins.ui \
    mwc_windows/filetransactions.ui \
    mwc_windows/outputs.ui \
    mwc_windows/transactions.ui \
    mwc_windows/contacts.ui \
    mwc_windows/hodl.ui \
    mwc_windows/claiming.ui \
    mwc_windows/claimchallendge.ui \
    mwc_windows/accounts.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES += \
    mw-logo.icns
