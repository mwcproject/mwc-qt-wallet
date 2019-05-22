#-------------------------------------------------
#
# Project created by QtCreator 2019-04-08T18:49:48
#
#-------------------------------------------------

QT       += core gui widgets
requires(qtConfig(completer))

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.12

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
    control/listwithcolumns.cpp \
    core/testseedtask.cpp \
    main.cpp \
    state/accounts.cpp \
    state/airdrop.cpp \
    state/connect2node.cpp \
    state/contacts.cpp \
    state/createwithseed.cpp \
    state/events.cpp \
    state/filetransactions.cpp \
    state/hodl.cpp \
    state/listening.cpp \
    state/nodemanually.cpp \
    state/nodestatus.cpp \
    state/outputs.cpp \
    state/receivecoins.cpp \
    state/sendcoins.cpp \
    state/transactions.cpp \
    state/walletconfig.cpp \
    util/widgetutils.cpp \
    windows/accounts_w.cpp \
    windows/airdrop_w.cpp \
    windows/airdropforbtc_w.cpp \
    windows/connect2server.cpp \
    windows/contacteditdlg.cpp \
    windows/contacts_w.cpp \
    windows/events_w.cpp \
    windows/filetransactions_w.cpp \
    windows/hodl_w.cpp \
    windows/listening_w.cpp \
    windows/nodemanually_w.cpp \
    windows/nodestatus_w.cpp \
    windows/outputs_w.cpp \
    core/mainwindow.cpp \
    core/versiondialog.cpp \
    core/windowmanager.cpp \
    wallet/wallet.cpp \
    wallet/mockwallet.cpp \
    windows/enterseed.cpp \
    windows/receivecoins_w.cpp \
    windows/selectcontact.cpp \
    windows/sendcoins_w.cpp \
    windows/sendcoinsparamsdialog.cpp \
    windows/showproofdialog.cpp \
    windows/transactions_w.cpp \
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
    windows/newseedtest_w.cpp \
    windows/walletconfig_w.cpp

HEADERS += \
    control/listwithcolumns.h \
    core/testseedtask.h \
    state/accounts.h \
    state/airdrop.h \
    state/connect2node.h \
    state/contacts.h \
    state/createwithseed.h \
    state/events.h \
    state/filetransactions.h \
    state/hodl.h \
    state/listening.h \
    state/nodemanually.h \
    state/nodestatus.h \
    state/outputs.h \
    state/receivecoins.h \
    state/sendcoins.h \
    state/transactions.h \
    state/walletconfig.h \
    util/widgetutils.h \
    windows/accounts_w.h \
    windows/airdrop_w.h \
    windows/airdropforbtc_w.h \
    windows/connect2server.h \
    windows/contacteditdlg.h \
    windows/contacts_w.h \
    windows/events_w.h \
    windows/filetransactions_w.h \
    windows/hodl_w.h \
    windows/listening_w.h \
    windows/nodemanually_w.h \
    windows/nodestatus_w.h \
    windows/outputs_w.h \
    core/mainwindow.h \
    core/versiondialog.h \
    core/windowmanager.h \
    wallet/wallet.h \
    wallet/mockwallet.h \
    windows/enterseed.h \
    windows/receivecoins_w.h \
    windows/selectcontact.h \
    windows/sendcoins_w.h \
    windows/sendcoinsparamsdialog.h \
    windows/showproofdialog.h \
    windows/transactions_w.h \
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
    windows/newseedtest_w.h \
    windows/walletconfig_w.h

FORMS += \
    windows/airdrop.ui \
    windows/airdropforbtc.ui \
    windows/connect2server.ui \
    windows/contacteditdlg.ui \
    windows/newseed.ui \
    windows/newwallet.ui \
    windows/nodestatus.ui \
    core/mainwindow.ui \
    core/versiondialog.ui \
    windows/enterseed.ui \
    windows/nodemanually.ui \
    windows/listening.ui \
    windows/events.ui \
    windows/receivecoins.ui \
    windows/selectcontact.ui \
    windows/sendcoins.ui \
    windows/filetransactions.ui \
    windows/outputs.ui \
    windows/sendcoinsparamsdialog.ui \
    windows/showproofdialog.ui \
    windows/transactions.ui \
    windows/contacts.ui \
    windows/hodl.ui \
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
