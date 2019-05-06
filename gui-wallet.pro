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
    state/accounts.cpp \
    state/createwithseed.cpp \
    state/events.cpp \
    state/hodl.cpp \
    state/nodestatus.cpp \
    state/sendcoins.cpp \
    windows/accounts_w.cpp \
    windows/confirmseed.cpp \
    windows/connect2server.cpp \
    windows/events_w.cpp \
    windows/hodl_w.cpp \
    windows/nodestatus_w.cpp \
    windows/passwordforseed.cpp \
    core/mainwindow.cpp \
    core/versiondialog.cpp \
    core/windowmanager.cpp \
    wallet/wallet.cpp \
    wallet/mockwallet.cpp \
    windows/enterseed.cpp \
    windows/fromseedfile.cpp \
    windows/nodemanually.cpp \
    windows/listening.cpp \
    windows/filetransactions.cpp \
    windows/outputs.cpp \
    windows/selectcontact.cpp \
    windows/sendcoins_w.cpp \
    windows/sendcoinsparamsdialog.cpp \
    windows/transactions.cpp \
    windows/contacts.cpp \
    windows/claiming.cpp \
    windows/claimchallendge.cpp \
    windows/walletconfig.cpp \
    util/ioutils.cpp \
    util/passwordanalyser.cpp \
    util/stringutils.cpp \
    state/state.cpp \
    state/statemachine.cpp \
    state/initaccount.cpp \
    core/appcontext.cpp \
    core/mwcexception.cpp \
    state/inputpassword.cpp \
    windows/initaccount_w.cpp \
    windows/inputpassword_w.cpp \
    state/newwallet.cpp \
    windows/newwallet_w.cpp \
    state/newseed.cpp \
    windows/newseed_w.cpp \
    state/newseedshow.cpp \
    state/newseedtest.cpp \
    windows/newseedtest_w.cpp

HEADERS += \
    state/accounts.h \
    state/createwithseed.h \
    state/events.h \
    state/hodl.h \
    state/nodestatus.h \
    state/sendcoins.h \
    windows/accounts_w.h \
    windows/confirmseed.h \
    windows/connect2server.h \
    windows/events_w.h \
    windows/hodl_w.h \
    windows/nodestatus_w.h \
    windows/passwordforseed.h \
    core/mainwindow.h \
    core/versiondialog.h \
    core/windowmanager.h \
    wallet/wallet.h \
    wallet/mockwallet.h \
    windows/enterseed.h \
    windows/fromseedfile.h \
    windows/nodemanually.h \
    windows/listening.h \
    windows/filetransactions.h \
    windows/outputs.h \
    windows/selectcontact.h \
    windows/sendcoins_w.h \
    windows/sendcoinsparamsdialog.h \
    windows/transactions.h \
    windows/contacts.h \
    windows/claiming.h \
    windows/claimchallendge.h \
    windows/walletconfig.h \
    util/passwordanalyser.h \
    util/ioutils.h \
    util/stringutils.h \
    state/state.h \
    state/statemachine.h \
    state/initaccount.h \
    core/appcontext.h \
    core/mwcexception.h \
    state/inputpassword.h \
    windows/initaccount_w.h \
    windows/inputpassword_w.h \
    state/newwallet.h \
    windows/newwallet_w.h \
    state/newseed.h \
    windows/newseed_w.h \
    state/newseedshow.h \
    state/newseedtest.h \
    windows/newseedtest_w.h

FORMS += \
    windows/confirmseed.ui \
    windows/connect2server.ui \
    windows/newseed.ui \
    windows/newwallet.ui \
    windows/nodestatus.ui \
    windows/passwordforseed.ui \
    core/mainwindow.ui \
    core/versiondialog.ui \
    windows/enterseed.ui \
    windows/fromseedfile.ui \
    windows/nodemanually.ui \
    windows/listening.ui \
    windows/events.ui \
    windows/selectcontact.ui \
    windows/sendcoins.ui \
    windows/filetransactions.ui \
    windows/outputs.ui \
    windows/sendcoinsparamsdialog.ui \
    windows/transactions.ui \
    windows/contacts.ui \
    windows/hodl.ui \
    windows/claiming.ui \
    windows/claimchallendge.ui \
    windows/accounts.ui \
    windows/walletconfig.ui \
    windows/initaccount.ui \
    windows/inputpassword.ui \
    windows/newseedtest.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES += \
    mw-logo.icns
