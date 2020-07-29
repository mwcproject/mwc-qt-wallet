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
#endif

#include <QApplication>
#include "wallet/mwc713.h"
#include "wallet/MockWallet.h"
#include "state/state.h"
#include "state/statemachine.h"
#include "core/appcontext.h"
#include "util/ioutils.h"
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include "core/global.h"
#include "util/ioutils.h"
#include "util/Log.h"
#include "core/Config.h"
#include "core/HodlStatus.h"
#include "util/ConfigReader.h"
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
#include <QFileDialog>
#include <QGuiApplication>
#include <QScreen>
#include <QDesktopWidget>
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
#include "bridge/wnd/g_send_b.h"
#include "bridge/wnd/g_finalize_b.h"
#include "bridge/wnd/e_transactions_b.h"
#include "bridge/wnd/a_initaccount_b.h"
#include "bridge/wnd/c_newseed_b.h"
#include "qtandroidservice.h"

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

    QString mwc713conf = confPath.second + "/wallet713v2.toml";
    QString mwcGuiWalletConf = confPath.second + "/mwc-gui-wallet-v3.conf";

    bool ok = true;

    if ( !QFile::exists(mwc713conf)) {
        ok = ok && QFile::copy(mwc::MWC713_DEFAULT_CONFIG, mwc713conf);
        if (ok)
            QFile::setPermissions(mwc713conf, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup);
    }

    if ( !QFile::exists(mwcGuiWalletConf)) {
        ok = ok && QFile::copy(mwc::QT_WALLET_DEFAULT_CONFIG, mwcGuiWalletConf);
        if (ok)
            QFile::setPermissions(mwcGuiWalletConf, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup);
    }

    // Set default values
    config::setMwc713conf(mwc713conf);
    config::setMwcGuiWalletConf(mwcGuiWalletConf);

    return ok;
}

// Read configs
// first - success flag
// second - error message
QPair<bool, QString> readConfig(QApplication & app) {
    QCoreApplication::setApplicationName("mwc-qt-wallet");
    QCoreApplication::setApplicationVersion("v0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("GUI wallet for MWC (MimbleWimbleCoin) https://www.mwc.mw/");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOptions({
                              {{"c", "config"},
                                      "Path to the mwc-gui-wallet config ",
                                      "mwc713 path",
                                      ""},
                      });

    parser.process(app);

    QString config = parser.value("config");
    if (config.isEmpty()) {
        config = config::getMwcGuiWalletConf();
    }
    else {
        config::setMwcGuiWalletConf(config);
    }

    util::ConfigReader reader;
    if ( !reader.readConfig(config) ) {
        qDebug() << "Failed to read config file " << config;
        return QPair<bool, QString>(false, "Unable to parse config file " + config);
    }

    QString mwc_path = reader.getString("mwc_path");
    QString wallet713_path = reader.getString("wallet713_path");
    QString mwczip_path = reader.getString("mwczip_path");
    QString airdropUrlMainNet = reader.getString("airdrop_url_mainnet");
    QString airdropUrlTestNet = reader.getString("airdrop_url_testnet");

    QString hodlUrlMainnet = reader.getString("hodl_url_mainnet");
    QString hodlUrlTestnet = reader.getString("hodl_url_testnet");

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

    if (airdropUrlMainNet.isEmpty())
        airdropUrlMainNet = "https://wallet.mwc.mw";

    if (airdropUrlTestNet.isEmpty())
        airdropUrlTestNet = "https://seed2.mwc.mw:8443";

    if (hodlUrlMainnet.isEmpty())
        hodlUrlMainnet = "https://wallet.mwc.mw";

    if (hodlUrlTestnet.isEmpty())
        hodlUrlTestnet = "https://seed2.mwc.mw:8443";

    if ( mwc_path.isEmpty() || wallet713_path.isEmpty() || airdropUrlMainNet.isEmpty() || airdropUrlTestNet.isEmpty() || hodlUrlMainnet.isEmpty() || hodlUrlTestnet.isEmpty() ) {
        qDebug() << "Failed to read all expected data from config file " << config;
        return QPair<bool, QString>(false, "Not found all expected fields at config file " + config);
    }

#ifdef WALLET_DESKTOP
    if (wallet713_path == "build in") {
        wallet713_path = QCoreApplication::applicationDirPath() + "/" + "mwc713";
#ifdef Q_OS_WIN
        wallet713_path += ".exe";
#endif
    }

    if (mwc_path == "build in") {
        mwc_path = QCoreApplication::applicationDirPath() + "/" + "mwc";
#ifdef Q_OS_WIN
        mwc_path += ".exe";
#endif
    }

    if (mwczip_path == "build in") {
        mwczip_path = QCoreApplication::applicationDirPath() + "/" + "mwczip";
#ifdef Q_OS_WIN
        mwc_path += ".exe";
#endif
    }
#endif

#ifdef WALLET_MOBILE
    // At Binary our binary are part of the data.
    if (wallet713_path == "build in") {
        wallet713_path = QCoreApplication::applicationDirPath() + "/mwc713.so";
    }

    if (mwc_path == "build in") {
        mwc_path = QCoreApplication::applicationDirPath() + "/mwc.so";
    }

    if (mwczip_path == "build in") {
        mwczip_path = QCoreApplication::applicationDirPath() + "/mwczip.so";
    }
#endif


    Q_ASSERT(runMode.first);
    config::setConfigData( runMode.second, mwc_path, wallet713_path, mwczip_path, airdropUrlMainNet, airdropUrlTestNet, hodlUrlMainnet, hodlUrlTestnet, logoutTimeout*1000L, timeoutMultiplierVal, sendTimeoutMs );

    return QPair<bool, QString>(true, "");
}

int main(int argc, char *argv[])
{
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
    qmlRegisterType<bridge::InputPassword>("InputPasswordBridge", 1, 0, "InputPasswordBridge");
    qmlRegisterType<bridge::Receive>("ReceiveBridge", 1, 0, "ReceiveBridge");
    qmlRegisterType<ClipboardProxy>("Clipboard", 1, 0, "Clipboard");
    qmlRegisterType<bridge::Send>("SendBridge", 1, 0, "SendBridge");
    qmlRegisterType<bridge::Finalize>("FinalizeBridge", 1, 0, "FinalizeBridge");
    qmlRegisterType<bridge::Transactions>("TransactionsBridge", 1, 0, "TransactionsBridge");
    qmlRegisterType<bridge::InitAccount>("InitAccountBridge", 1, 0, "InitAccountBridge");
    qmlRegisterType<bridge::NewSeed>("NewSeedBridge", 1, 0, "NewSeedBridge");

    core::MobileWndManager * wndManager = new core::MobileWndManager();
#endif

    core::setWndManager(wndManager);

    while (true)
    {
        // QApplication instance is needed to show possible errors
   //     QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);  Disabled because it looks badly on 4k Linux and Windows with Scale 200% (default for 4k)
        QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

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
#endif


#ifdef WALLET_DESKTOP
        // Update scale if screen resolution is low...
        // Unfortunatelly we can't do that before QApplication inited because Scree res API doesn't work
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

        logger::logInfo("mwc-qt-wallet", QString("Starting mwc-gui-wallet version ") + BUILD_VERSION + " with config:\n" + config::toString() );
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
                core::getWndManager()->messageTextDlg("Setup Issue", "Your home directory\n" + homePath.second + "\ncontains Unicode symbols. Unfortunatelly mwc713 unable to handle that.\n\n"
                                         "Please reinstall mwc-qt-wallet under different user name with basic ASCII (Latin1) symbols only.");
                return 1;
            }
        }

        // Checking for Build Architecture.
        // NOTE!!! Checking is needed for mwc713, not for this app.
        // We assuming that everything runs from normal install and architectures of mwc713 and mwc-qt-wallet
        // are the same.
        QString runningArc = util::getBuildArch();
        wallet::WalletConfig walletConfig = wallet::MWC713::readWalletConfig();
        QString walletDataPath = walletConfig.getDataPath();

        if (!config::isOnlineNode())
        {
            // Check if wallet point to the right location
            if ( walletDataPath.contains("tmp") && walletDataPath.contains("online_node_wallet") ) {
                // need to switch to the normal wallet path

                // Try to restore the last valid wallet path and a network...
                QString network;
                if (!appContext.getWallet713DataPathWithNetwork(walletDataPath, network) )
                {
                    walletDataPath = "gui_wallet713_data";
                    network = walletConfig.getNetwork();
                }
                wallet::MwcNodeConnection mwcNodeConnection = appContext.getNodeConnection( network );
                if ( !mwcNodeConnection.isCloudNode() ) {
                    mwcNodeConnection.setAsCloud();
                    appContext.updateMwcNodeConnection(network, mwcNodeConnection );
                }
            }
        }

        while (true) {
            QPair<QString,QString> network_arch = wallet::WalletConfig::readNetworkArchFromDataPath(walletDataPath);

            if (!network_arch.first.isEmpty())
                walletConfig.updateNetwork(network_arch.first);

            QString arh = network_arch.second;

            if (arh != runningArc) {
                if ( core::WndManager::RETURN_CODE::BTN1 == core::getWndManager()->questionTextDlg("Wallet data architecture mismatch",
                                             "Your mwc713 seed at '"+ walletDataPath +"' was created with "+arh+" bits version of the wallet. "
                                             "Please exit and use original version of the wallet, or specify another folder for the seed",
                                             "Exit", "Select Folder",
                                             "Exit QT Wallet, I have another version to open this wallet",
                                             "Select another wallet with compatible architecture",
                                             false, true) ) {
                    // Exit was selected
                    return 1;
                }

                QPair<bool,QString> basePath = ioutils::getAppDataPath();
                if (!basePath.first) {
                    core::getWndManager()->messageTextDlg("Error", basePath.second);
                    return 1;
                }

                QString dir = QFileDialog::getExistingDirectory(
                        nullptr,
                        "Select your wallet folder name",
                        basePath.second);
                if (dir.isEmpty())
                    return 1; // Exiting

                auto dirOk = util::validateMwc713Str(dir);
                if (!dirOk.first) {
                    core::getWndManager()->messageTextDlg("Directory Name",
                              "This directory name is not acceptable.\n" + dirOk.second);
                        // Exit was selected
                        return 1;
                }

                QDir baseDir(basePath.second);
                walletDataPath = baseDir.relativeFilePath(dir);
            }
            else {
                break; // good to go
            }
        }

        // Prepare wallet to run as online Node
        if (config::isOnlineNode())
        {
            QString network = walletConfig.getNetwork();
            // mwc713 should run without any password with some account.
            // Account not expected for use, so no protection is needed. Instead will use the special directory for that
            walletDataPath = QString("tmp") + QDir::separator() + "online_node_wallet"  + QDir::separator() + network;

            // Node must be embedded local.
            wallet::MwcNodeConnection mwcNodeConnection = appContext.getNodeConnection( network );
            if ( !mwcNodeConnection.isLocalNode() ) {
                mwcNodeConnection.setAsLocal( "mwc-node" );
                appContext.updateMwcNodeConnection(network, mwcNodeConnection );
            }

            // Start page will be always the node status
            appContext.setActiveWndState( state::STATE::NODE_INFO );
        }


        if (config::isColdWallet()) {
            // Node must be local
            QString network = walletConfig.getNetwork();
            wallet::MwcNodeConnection mwcNodeConnection = appContext.getNodeConnection( network );
            if ( !mwcNodeConnection.isLocalNode() ) {
                mwcNodeConnection.setAsLocal( "mwc-node" );
                appContext.updateMwcNodeConnection(network, mwcNodeConnection );
            }

            // Start page will be always the node status
            appContext.setActiveWndState( state::STATE::NODE_INFO );
        }


        if (walletDataPath != walletConfig.getDataPath()) {
            walletConfig.updateDataPath(walletDataPath);
        }

        if (!util::acquireAppGlobalLock() )
        {
            // Seems like we are blocked on global semaphore. It is mean that second instance does exist
            core::getWndManager()->messageTextDlg("Second mwc-qt-wallet instance is detected",
                    "There is another instance of mwc-qt-wallet is already running. It is impossible to run more than one instance of the wallet at the same time.");
            return 1;
        }

        // Checking if TOR is active. Then we will activate Foreign API.  Or if Foreign API active wrong way, we will disable the TOR
        if (appContext.isAutoStartTorEnabled()) {
            if (!walletConfig.hasForeignApi()) {
                // Expected to do that silently. It is a migration case
                walletConfig.setForeignApi(true,"127.0.0.1:3415","", "","");
            }
            else {
                // Check if foreign API has HTTPs. TOR doesn't support it
                if (walletConfig.hasTls()) {
                    core::getWndManager()->messageTextDlg("Unable to start TOR",
                                                          "Your Foreign API is configured to use TLS certificated. TOR doesn't support https connection.\n\n"
                                                          "Because of that TOR will not be started. You can review your configuration at Wallet Settings page.");
                    appContext.setAutoStartTorEnabled(false);
                }
            }
        }

        // Update Node
        node::MwcNode * mwcNode = new node::MwcNode( config::getMwcPath(), &appContext );

        wallet::MWC713::saveWalletConfig( walletConfig, &appContext, mwcNode );

#ifdef WALLET_DESKTOP
        wallet::MWC713 * wallet = new wallet::MWC713( config::getWallet713path(), config::getMwc713conf(), &appContext );
#else
        wallet::MockWallet * wallet = new wallet::MockWallet(&appContext);
//        wallet::MWC713 * wallet = new wallet::MWC713( config::getWallet713path(), config::getMwc713conf(), &appContext );
        // QtAndroidService *qtAndroidService = new QtAndroidService(&app);
        // qtAndroidService->sendToService("Hello Buddy");
#endif

        state::StateContext context( &appContext, wallet, mwcNode );

        core::HodlStatus hodlStatus(&context);
        context.setHodlStatus(&hodlStatus);
        wallet->setHodlStatus(&hodlStatus);

//        core::WalletNotes walletNotes(&context);
//        appContext.setWalletNotes(&walletNotes);

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

        // Now we have to stop other object nicely.
        // Note, the order is different from creation.
        // mainWnd expected to be dead here.
        state::StateMachine::destroyStateMachine();
        delete wallet;  wallet = nullptr;
#ifdef WALLET_DESKTOP
        delete windowManager; windowManager=nullptr;
#endif

        if (mwcNode->isRunning()) {
            mwcNode->stop();
        }

        delete mwcNode; mwcNode = nullptr;

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

