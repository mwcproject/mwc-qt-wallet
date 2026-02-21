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

#ifdef WALLET_DESKTOP
#include "core_desktop/mainwindow.h"
#include "core_desktop/windowmanager.h"
#include "core_desktop/DesktopWndManager.h"
#endif
#ifdef WALLET_MOBILE
#include "core_mobile/MobileWndManager.h"
#include <QSysInfo>
#include "core_mobile/qtandroidservice.h"
#include <QAndroidService>
#endif

#include <QApplication>
#include "state/state.h"
#include "state/statemachine.h"
#include "core/appcontext.h"
#include "util/ioutils.h"
#include "util/Files.h"
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include "core/global.h"
#include "util/ioutils.h"
#include "util/Log.h"
#include "core/Config.h"
#include "util/ConfigReader.h"
#include "util/Files.h"
#include <QFileDevice>
#include <QJsonDocument>
#include <QJsonObject>
#include "util/address.h"
#include <QSystemSemaphore>
#include <QThread>
#include "util/execute.h"
#include "util/Process.h"
#include "tests/testStringUtils.h"
#include <QtGlobal>
#include <QGuiApplication>
#include <QScreen>
#include <tgmath.h>
#include "node/MwcNodeConfig.h"
#include "node/MwcNode.h"
#include "tests/testWordSequenser.h"
#include "tests/testWordDictionary.h"
#include "tests/testPasswordAnalyser.h"
#include "tests/testCalcOutputsToSpend.h"
#include "tests/testLogs.h"
#include "misk/DictionaryInit.h"
#include "util/stringutils.h"
#include "build_version.h"
#include "core/WalletApp.h"
#include "core/WndManager.h"
#include "bridge/wnd/a_inputpassword_b.h"
#include "bridge/wallet_b.h"
#include "bridge/util_b.h"
#include "bridge/corewindow_b.h"
#include "bridge/wnd/e_receive_b.h"
#include "bridge/config_b.h"
#include "bridge/clipboard_b.h"
#include "bridge/statemachine_b.h"
#include "bridge/notification_b.h"
#include "bridge/wnd/g_send_b.h"
#include "bridge/wnd/g_finalize_b.h"
#include "bridge/wnd/e_transactions_b.h"
#include "bridge/wnd/a_initaccount_b.h"
#include "bridge/wnd/c_newseed_b.h"
#include "bridge/wnd/x_events_b.h"
#include "bridge/wnd/a_startwallet_b.h"
#include "bridge/wnd/z_progresswnd_b.h"
#include "bridge/wnd/e_outputs_b.h"
#include "bridge/wnd/x_walletconfig_b.h"
#include "bridge/wnd/k_accounts_b.h"
#include "bridge/wnd/k_accounttransfer_b.h"
#include "bridge/wnd/u_nodeInfo_b.h"
#include "core/MessageMapper.h"
#include "node/TorProcess.h"
#include "node/NodeWalletLogs.h"


#ifdef WALLET_MOBILE
#include <QQmlApplicationEngine>
#endif

#ifdef Q_OS_DARWIN
namespace Cocoa
{
bool isRetinaDisplay();
}
#endif

// Very first run - init everything
bool deployWalletFilesFromResources() {
    QPair<bool,QString> confPath = ioutils::getAppDataPath();
    if (!confPath.first) {
        core::getWndManager()->messageTextDlg("Error", confPath.second);
        return false;
    }

    QString mwcGuiWalletConf = confPath.second + "/mwc-gui-wallet-v3.conf";

    bool ok = true;

    if ( !QFile::exists(mwcGuiWalletConf)) {
        ok = ok && QFile::copy(mwc::QT_WALLET_DEFAULT_CONFIG, mwcGuiWalletConf);
        if (ok)
            QFile::setPermissions(mwcGuiWalletConf, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup);
    }

    // Set default values
    config::setMwcGuiWalletConf(mwcGuiWalletConf);

    return ok;
}

// Read configs
// first - success flag
// second - error message
QPair<bool, QString> readConfig(QApplication & app) {
    Q_UNUSED(app);
    QCoreApplication::setApplicationName("mwc-qt-wallet");
    QCoreApplication::setApplicationVersion("v0.1");

    QString config = config::getMwcGuiWalletConf();

    util::ConfigReader reader;
    if ( !reader.readConfig(config) ) {
        qDebug() << "Failed to read config file " << config;
        return QPair<bool, QString>(false, "Unable to parse config file " + config);
    }

    //QString mwc_path = reader.getString("mwc_path");
    QString logoutTimeoutStr = reader.getString("logoutTimeout");
    QString timeoutMultiplier = reader.getString("timeoutMultiplier");
    QString sendTimeoutMsStr = reader.getString("send_online_timeout_ms");

    QString runningMode = reader.getString("running_mode");
    if (runningMode.isEmpty())
        runningMode = "online_wallet";

    QPair<bool, config::WALLET_RUN_MODE> runMode = config::runModeFromString(runningMode);
    if (!runMode.first) {
        return QPair<bool, QString>(false, "Found invalid value for 'running_mode'");
    }

    int sendTimeoutMs = sendTimeoutMsStr.toInt();
    if (sendTimeoutMs<=0)
        sendTimeoutMs = 60000; // 1 minutes should be good enough

    bool logoutTimeoutOk = false;
    int     logoutTimeout = logoutTimeoutStr.toInt(&logoutTimeoutOk);
    if (!logoutTimeoutOk || logoutTimeoutStr.isEmpty() )
        logoutTimeout = 15*60;

    double timeoutMultiplierVal = timeoutMultiplier.isEmpty() ? 2.0 : timeoutMultiplier.toDouble();
    if ( timeoutMultiplierVal < 0.01 )
        timeoutMultiplierVal = 1.0;

    Q_ASSERT(runMode.first);
    config::setConfigData( runMode.second, /*mwc_path,*/ logoutTimeout*1000L, timeoutMultiplierVal, sendTimeoutMs );

    return QPair<bool, QString>(true, "");
}

int main(int argc, char *argv[])
{
#ifdef WALLET_MOBILE
    if (argc > 1 && strcmp(argv[1], "-service") == 0) {
        qWarning() << "Service starting with BroadcastReceiver from same .so file";
        QAndroidService app(argc, argv);

        return app.exec();
    }
#endif
    int retVal = 0;

    double uiScale = 1.0;

#ifdef WALLET_DESKTOP
    core::DesktopWndManager * wndManager = new core::DesktopWndManager();
#endif
#ifdef WALLET_MOBILE
    qmlRegisterType<bridge::StateMachine>("StateMachineBridge", 1, 0, "StateMachineBridge");
    qmlRegisterType<bridge::CoreWindow>("CoreWindowBridge", 1, 0, "CoreWindowBridge");
    qmlRegisterType<bridge::Wallet>("WalletBridge", 1, 0, "WalletBridge");
    qmlRegisterType<bridge::Util>("UtilBridge", 1, 0, "UtilBridge");
    qmlRegisterType<bridge::Config>("ConfigBridge", 1, 0, "ConfigBridge");
    qmlRegisterType<bridge::Notification>("NotificationBridge", 1, 0, "NotificationBridge");
    qmlRegisterType<bridge::InputPassword>("InputPasswordBridge", 1, 0, "InputPasswordBridge");
    qmlRegisterType<bridge::Receive>("ReceiveBridge", 1, 0, "ReceiveBridge");
    qmlRegisterType<ClipboardProxy>("Clipboard", 1, 0, "Clipboard");
    qmlRegisterType<bridge::Send>("SendBridge", 1, 0, "SendBridge");
    qmlRegisterType<bridge::Finalize>("FinalizeBridge", 1, 0, "FinalizeBridge");
    qmlRegisterType<bridge::Transactions>("TransactionsBridge", 1, 0, "TransactionsBridge");
    qmlRegisterType<bridge::InitAccount>("InitAccountBridge", 1, 0, "InitAccountBridge");
    qmlRegisterType<bridge::NewSeed>("NewSeedBridge", 1, 0, "NewSeedBridge");
    qmlRegisterType<bridge::Events>("EventsBridge", 1, 0, "EventsBridge");
    qmlRegisterType<bridge::StartWallet>("StartWalletBridge", 1, 0, "StartWalletBridge");
    qmlRegisterType<bridge::ProgressWnd>("ProgressWndBridge", 1, 0, "ProgressWndBridge");
    qmlRegisterType<bridge::Outputs>("OutputsBridge", 1, 0, "OutputsBridge");
    qmlRegisterType<bridge::WalletConfig>("WalletConfigBridge", 1, 0, "WalletConfigBridge");
    qmlRegisterType<bridge::Accounts>("AccountsBridge", 1, 0, "AccountsBridge");
    qmlRegisterType<bridge::AccountTransfer>("AccountTransferBridge", 1, 0, "AccountTransferBridge");
    qmlRegisterType<bridge::NodeInfo>("NodeInfoBridge", 1, 0, "NodeInfoBridge");
    qmlRegisterType<QtAndroidService>("QtAndroidService", 1, 0, "QtAndroidService");

    core::MobileWndManager * wndManager = new core::MobileWndManager();

    util::clearTempDir();
#endif

    core::setWndManager(wndManager);

    while (true)
    {
        // QApplication instance is needed to show possible errors
   //     QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);  Disabled because it looks badly on 4k Linux and Windows with Scale 200% (default for 4k)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

        Q_ASSERT(argc>=1);
        // Process arglist.
        // Furst argument has to be the app path
        util::setMwcQtWalletPath(argv[0]);

        // !!! Note !!!  Custom arguments must be last in the line. Otherwise all at the right will be truncated.
        core::AppContext appContext;
        for ( int t=1;t<argc-1; t++) {
            if ( strcmp("--ui_scale", argv[t])==0 ) {
                double scale = QString(argv[t+1]).toDouble();
                if (scale>0.5 && scale <= 4.0) {
                    appContext.initGuiScale(scale);
                    uiScale = scale;
                }

                argc = t;
                break;
            }
        }

        // MacOS doesn't process QT_SCALE_FACTOR correctlly. That is why it is disabled here
    #ifndef Q_OS_DARWIN
        // First let's app the UI scale factor. It must be done before QApplication will be created

        double scale = appContext.getGuiScale();
        if (scale==1.0)
            scale = 1.001;

        if (scale>0.0)
            qputenv( "QT_SCALE_FACTOR", QString::number(scale).toLatin1() );

    #else
        #ifdef WALLET_DESKTOP
            double scale = 1.0; // Mac OS, not applicable, mean 1.0
            // But scale factor still needed to fix the non retina cases on mac OS

            if (! Cocoa::isRetinaDisplay()) {
                qputenv("QT_SCALE_FACTOR", "1.001");
            }
        #endif
    #endif

#if defined(QT_DEBUG) && defined(WALLET_DESKTOP)
// Windows debugger so bad, it can even profile the code
#ifndef Q_OS_WIN
    // Generation of the dictionaries.
    // Don't uncomment it!
    // misk::provisionDictionary();

    // tests are quick, let's run them in debug
//    test::testCalcOutputsToSpend();  // This test is long and show about 8 Message boxes.
//    test::testLogsRotation();
    test::testLongLong2ShortStr();
    test::testUtils();
    test::testWordSequences();
    test::testWordDictionary();
    test::testPasswordAnalyser();
    test::testMessageMapper();
#endif
#endif

#ifdef WALLET_DESKTOP
        // Update scale if screen resolution is low...
        // Unfortunatelly we can't do that before QApplication inited because Screen res API doesn't work
        // That is why update now is pretty costly, we will need to testart the all because of that.
        {
            int minWidth = 10000;
            int minHeight = 10000;

            QList<QScreen *> screens = QGuiApplication::screens();
            for ( QScreen * s : screens) {
                QSize scrSz = s->availableSize() * scale;
                minWidth = std::min( minWidth, scrSz.width() );
                minHeight = std::min( minHeight, scrSz.height() );
            }

            double prevScale = scale;

            if ( minWidth <= 1000 || minHeight <=600 )
                scale = 1.0;
            else if ( minWidth <= 1300 || minHeight <=700 )
                scale = std::min(scale, 1.2);
            else if ( minWidth <= 1500 || minHeight <=800 )
                scale = std::min(scale, 1.4);
            else if ( minWidth <= 1800 || minHeight <=950 )
                scale = std::min(scale, 1.6);

            if ( std::fabs(scale-prevScale)>0.01 ) {
                // need to update
                appContext.setGuiScale(scale);
                retVal = 1;
                util::requestRestartMwcQtWallet();
                break;
            }
        }
#endif

        core::WalletApp app(argc, argv);

        // Check if swaps we want to enable
        {
            Qt::KeyboardModifiers modifiers = QGuiApplication::queryKeyboardModifiers();
            if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier) {
                mwc::setSwapActive(true);
            }
        }


        if (!deployWalletFilesFromResources() ) {
            QMessageBox::critical(nullptr, "Error", "Unable to provision or verify resource files during the first run");
            return 1;
        }

        QPair<bool, QString> readRes = readConfig(app);
        if (!readRes.first) {
            QMessageBox::critical(nullptr, "Error", "MWC GUI Wallet unable to read configuration.\n" + readRes.second);
            return 1;
        }

        // Logger must be start AFTER readConfig because logger require mwczip location and it is defined at the configs
        logger::initLogger(appContext.isLogsEnabled());


/*        if (true) {
            test::testMwcNodeApi();
            return 1;
        }*/

        logger::logInfo( logger::QT_WALLET, QString("Starting mwc-gui-wallet version ") + BUILD_VERSION + " with config:\n" + config::toString() );
        qDebug().noquote() << "Starting mwc-gui-wallet with config:\n" << config::toString();

#ifdef WALLET_DESKTOP
        { // Apply style sheet
            QFile file(":/resource_desktop/mwcwallet_style.css" );
            if (file.open(QFile::ReadOnly | QFile::Text)) {
                   QTextStream ts(&file);
                   app.setStyleSheet(ts.readAll());
            }
            else {
                QMessageBox::critical(nullptr, "Error", "MWC GUI Wallet unable to read the stylesheet.");
                return 1;
            }
        }
#endif

        app.reportAppAsInitialized();

        {
            // Checking if home path is ascii (Latin1) symbols only
            QPair<bool,QString> homePath = ioutils::getAppDataPath();
            if (!homePath.first) {
                core::getWndManager()->messageTextDlg("Error", homePath.second);
                return 1;
            }

            int idx = homePath.second.indexOf("mwc-qt-wallet");
            if (idx<0)
                idx = homePath.second.length();

            homePath.second = homePath.second.left(idx-1);

            if ( !util::validateMwc713Str(homePath.second, false).first ) {
                core::getWndManager()->messageTextDlg("Setup Issue", "Your home directory\n" + homePath.second + "\ncontains Unicode symbols. Unfortunately, the wallet cannot handle that.\n\n"
                                         "Please reinstall MWC Qt Wallet under a different user name with basic ASCII (Latin1) symbols only.");
                return 1;
            }
        }

        if (!util::acquireAppGlobalLock() )
        {
            // Seems like we are blocked on global semaphore. It is mean that second instance does exist
            core::getWndManager()->messageTextDlg("Second MWC Qt Wallet instance detected",
                    "Another instance of MWC Qt Wallet is already running. You can run only one instance of the wallet at a time.");
            return 1;
        }

        // Let's start Tor core and init core logs
        rust_logs::initRustLogs("Info");

        QFuture<QString> torStarter = wallet::startEmbeddedTor();

#ifdef WALLET_DESKTOP
        wallet::Wallet * wallet = new wallet::Wallet(&torStarter);
#else
        // wallet::MockWallet * wallet = new wallet::MockWallet(&appContext);
        wallet::MWC713 * wallet = new wallet::MWC713();
        QtAndroidService *qtAndroidService = new QtAndroidService(&app);
        qtAndroidService->sendToService("Start Service");
#endif

        state::StateContext context( &appContext, wallet, &torStarter );

        state::setStateContext(&context);

        state::StateMachine::initStateMachine();

#ifdef WALLET_DESKTOP
        //main window has delete on close flag. That is why need to
        // create dynamically. Window will be deleted on close
        core::MainWindow * mainWnd = new core::MainWindow(nullptr);

        mwc::setApplication(&app, mainWnd);

        core::WindowManager * windowManager = new core::WindowManager( mainWnd, mainWnd->getMainWindow() );

        mainWnd->show();

        wndManager->init(windowManager, mainWnd);
#endif
#ifdef WALLET_MOBILE
        // Mobile can only imit engine after app instance is created.
        QQmlApplicationEngine engine;
        wndManager->init(&engine);
#endif

        if (mwc::isAppNonClosed()) {
            state::getStateMachine()->start();

            retVal = app.exec();
        }

        core::WalletApp::startExiting();
        node::shutdown_tor();

        // Now we have to stop other object nicely.
        // Note, the order is different from creation.
        // mainWnd expected to be dead here.
        state::StateMachine::destroyStateMachine();
        delete wallet;  wallet = nullptr;
#ifdef WALLET_DESKTOP
        delete windowManager; windowManager=nullptr;
#endif

        torStarter.waitForFinished();

        util::releaseAppGlobalLock();

        break;
    }

    // All objets are expected to be released at this point
    util::restartMwcQtWalletIfRequested(uiScale);

#ifdef WALLET_DESKTOP
    delete wndManager;
#endif

    return retVal;
  }

