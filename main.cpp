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

#include "core/mainwindow.h"
#include <QApplication>
#include "core/windowmanager.h"
#include "wallet/mwc713.h"
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
#include "util/ConfigReader.h"
#include <QFileDevice>
#include <QJsonDocument>
#include <QJsonObject>
#include "util/address.h"
#include "dialogs/helpdlg.h"
#include <QSystemSemaphore>
#include <QThread>
#include <control/messagebox.h>
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
#include "misk/DictionaryInit.h"
#include "util/stringutils.h"
#include "build_version.h"

#ifdef Q_OS_DARWIN
namespace Cocoa
{
bool isRetinaDisplay();
}
#endif

// Very first run - init everything
bool deployWalletFilesFromResources() {
    QString confPath = ioutils::getAppDataPath();

    QString mwc713conf = confPath + "/wallet713v2.toml";
    QString mwcGuiWalletConf = confPath + "/mwc-gui-wallet-v3.conf";

    bool ok = true;

    if ( !QFile::exists(mwc713conf)) {
        ok = ok && QFile::copy(mwc::MWC713_DEFAULT_CONFIG, mwc713conf);
        if (ok)
            QFile::setPermissions(mwc713conf, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup);
    }

    if ( !QFile::exists(mwcGuiWalletConf)) {
        ok = ok && QFile::copy(":/resource/mwc-gui-wallet.conf", mwcGuiWalletConf);
        if (ok)
            QFile::setPermissions(mwcGuiWalletConf, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup);
    }

    // Set default values
    config::setMwc713conf(mwc713conf);
    config::setMwcGuiWalletConf(mwcGuiWalletConf);

    return ok;
}

// Read configs
bool readConfig(QApplication & app) {
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
        return false;
    }

    QString mwc_path = reader.getString("mwc_path");
    QString wallet713_path = reader.getString("wallet713_path");
    QString main_style_sheet = reader.getString("main_style_sheet");
    QString dialogs_style_sheet = reader.getString("dialogs_style_sheet");
    QString airdropUrlMainNet = reader.getString("airdrop_url_mainnet");
    QString airdropUrlTestNet = reader.getString("airdrop_url_testnet");
    QString logoutTimeoutStr = reader.getString("logoutTimeout");
    QString timeoutMultiplier = reader.getString("timeoutMultiplier");
    bool useMwcMqS = reader.getString("useMwcMqS") != "false";  // Default expected to be 'true'
    QString sendTimeoutMsStr = reader.getString("send_online_timeout_ms");

    int sendTimeoutMs = sendTimeoutMsStr.toInt();
    if (sendTimeoutMs<=0)
        sendTimeoutMs = 60000; // 1 minutes should be good enough

    if (main_style_sheet.isEmpty())
        main_style_sheet = ":/resource/mwcwallet_style.css";

    if (dialogs_style_sheet.isEmpty())
        dialogs_style_sheet = ":/resource/dialogs_style.css";

    bool logoutTimeoutOk = false;
    int     logoutTimeout = logoutTimeoutStr.toInt(&logoutTimeoutOk);
    if (!logoutTimeoutOk || logoutTimeoutStr.isEmpty() )
        logoutTimeout = 15*60;

    double timeoutMultiplierVal = timeoutMultiplier.isEmpty() ? 2.0 : timeoutMultiplier.toDouble();
    if ( timeoutMultiplierVal < 0.01 )
        timeoutMultiplierVal = 1.0;

    if ( mwc_path.isEmpty() || wallet713_path.isEmpty() || main_style_sheet.isEmpty() || dialogs_style_sheet.isEmpty() || airdropUrlMainNet.isEmpty() || airdropUrlTestNet.isEmpty() ) {
        qDebug() << "Failed to read all expected data from config file " << config;
        return false;
    }

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

    config::setConfigData( mwc_path, wallet713_path, main_style_sheet, dialogs_style_sheet, airdropUrlMainNet, airdropUrlTestNet, logoutTimeout*1000L, timeoutMultiplierVal, useMwcMqS, sendTimeoutMs );
    return true;
}

int main(int argc, char *argv[])
{
#ifdef QT_DEBUG
    // Generation of the dictionaries.
    // Don't uncomment it!
    // misk::provisionDictionary();


    // tests are quick, let's run them in debug
    test::testLongLong2ShortStr();
    test::testUtils();
    test::testWordSequences();
    test::testWordDictionary();
    test::testPasswordAnalyser();
#endif

    int retVal = 0;

    while (true)
    {

        Q_ASSERT(argc>=1);
        // Process arglist.
        // Furst argument has to be the app path
        util::setMwcQtWalletPath(argv[0]);

        // !!! Note !!!  Custor arguments must be last in the line. Otherwise all at the right will be truncated.
        double scale = -1.0;
        for ( int t=1;t<argc-1; t++) {
            if ( strcmp("--ui_scale", argv[t])==0 ) {
                scale = QString(argv[t+1]).toDouble();
                argc = t;
                break;
            }
        }
        core::AppContext appContext;

        if (scale>0.0)
            appContext.initGuiScale(scale);


        // MacOS doesn't process QT_SCALE_FACTOR correctlly. That is why it is disabled here
    #ifndef Q_OS_DARWIN
        // First let's app the UI scale factor. It must be done before QApplication will be created

        scale = appContext.getGuiScale();
        if (scale==1.0)
            scale = 1.001;

        if (scale>0.0)
            qputenv( "QT_SCALE_FACTOR", QString::number(scale).toLatin1() );

    #else
        scale = 1.0; // Mac OS, not applicable, mean 1.0
        // But scale factor still needed to fix the non retina cases on mac OS

        if (! Cocoa::isRetinaDisplay()) {
            qputenv("QT_SCALE_FACTOR", "1.001");
        }
    #endif

        QApplication app(argc, argv);
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

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


        logger::initLogger(appContext.isLogsEnabled());

        if (!deployWalletFilesFromResources() ) {
            QMessageBox::critical(nullptr, "Error", "Unable to provision or verify resource files during the first run");
            return 1;
        }

        if (!readConfig(app) ) {
            QMessageBox::critical(nullptr, "Error", "MWC GUI Wallet unable to read configuration");
            return 1;
        }

        logger::logInfo("mwc-qt-wallet", QString("Starting mwc-gui-wallet version ") + BUILD_VERSION );
        logger::logInfo("mwc-qt-wallet", config::toString());
        qDebug().noquote() << "Starting mwc-gui-wallet with config:\n" << config::toString();

        { // Apply style sheet
            QFile file( config::getMainStyleSheetPath() );
            if (file.open(QFile::ReadOnly | QFile::Text)) {
                   QTextStream ts(&file);
                   app.setStyleSheet(ts.readAll());
            }
            else {
                QMessageBox::critical(nullptr, "Error", "MWC GUI Wallet unable to read the stylesheet.");
                return 1;
            }
        }

        {
            // Checking if home path is ascii (Latin1) symbols only
            QString homePath = ioutils::getAppDataPath();
            int idx = homePath.indexOf("mwc-qt-wallet");
            if (idx<0)
                idx = homePath.length();

            homePath = homePath.left(idx-1);

            if ( !util::validateMwc713Str(homePath, false).first ) {
                control::MessageBox::messageText(nullptr, "Setup Issue", "Your home directory\n" + homePath + "\ncontains Unicode symbols. Unfortunatelly mwc713 unable to handle that.\n\n"
                                         "Please reinstall mwc-qt-wallet under different user name with basic ASCII (Latin1) symbols only.");
                return 1;
            }
        }

        // Checking for Build Architecture.
        // NOTE!!! Checking is needed for mwc713, not for this app.
        // We assuming that everything runs from normal install and architectures of mwc713 and mwc-qt-wallet
        // are the same.
        QString runningArc = util::getBuildArch();
        wallet::WalletConfig config = wallet::MWC713::readWalletConfig();
        QString walletDataPath = config.getDataPath();
        while (true) {
            QString arch = wallet::WalletConfig::readNetworkArchFromDataPath(walletDataPath).second;

            if (arch != runningArc) {
                if ( control::MessageBox::RETURN_CODE::BTN1 == control::MessageBox::questionText(nullptr, "Wallet data architecture mismatch",
                                             "Your mwc713 seed at '"+ walletDataPath +"' was created with "+arch+" bits version of the wallet. "
                                             "Please exit and use original version of the wallet, or specify another folder for the seed",
                                             "Exit", "Select Folder", false, true) ) {
                    // Exit was selected
                    return 1;
                }

                QString basePath = ioutils::getAppDataPath();
                QString dir = QFileDialog::getExistingDirectory(
                        nullptr,
                        "Select your wallet folder name",
                        basePath);
                if (dir.isEmpty())
                    return 1; // Exiting

                QDir baseDir(basePath);
                walletDataPath = baseDir.relativeFilePath(dir);
            }
            else {
                break; // good to go
            }
        }

        if (walletDataPath != config.getDataPath()) {
            config.updateDataPath(walletDataPath);
        }

        if (!util::acquireAppGlobalLock() )
        {
            // Seems like we are blocked on global semaphore. It is mean that second instance does exist
            control::MessageBox::messageText(nullptr, "Second mwc-qt-wallet instance is detected",
                                         "There is another instance of mwc-qt-wallet is already running. It is impossible to run more than one instance of the wallet at the same time.");
            return 1;
        }

        // Update Node
        node::MwcNode * mwcNode = new node::MwcNode( config::getMwcpath(), &appContext );

        wallet::MWC713::saveWalletConfig( config, &appContext, mwcNode );

        //main window has delete on close flag. That is why need to
        // create dynamically. Window will be deleted on close
        core::MainWindow * mainWnd = new core::MainWindow(nullptr);

        mwc::setApplication(&app, mainWnd);

        wallet::MWC713 * wallet = new wallet::MWC713( config::getWallet713path(), config::getMwc713conf(), &appContext );

        core::WindowManager * wndManager = new core::WindowManager( mainWnd, mainWnd->getMainWindow() );

        mainWnd->show();

        state::StateContext context( &appContext, wallet, mwcNode, wndManager, mainWnd );

        state::StateMachine * machine = new state::StateMachine(&context);
        mainWnd->setAppEnvironment( machine, wallet);
        machine->start();

        retVal = app.exec();

        // Now we have to stop other object nicely.
        // Note, the order is different from creation.
        // mainWnd expected to be dead here.
        delete machine; machine=nullptr;
        delete wallet;  wallet = nullptr;
        delete wndManager; wndManager=nullptr;

        if (mwcNode->isRunning()) {
            mwcNode->stop();
        }

        delete mwcNode; mwcNode = nullptr;

        util::releaseAppGlobalLock();

        break;
    }

    // All objets are expected to be released at this point
    util::restartMwcQtWalletIfRequested();

    return retVal;
  }

