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

#include "MwcNodeConfig.h"
#include "../util/ioutils.h"
#include <QDir>
#include "../util/Files.h"
#include "../util/ConfigReader.h"
#include <QtGlobal>
#include <QTime>
#include "../core/global.h"

namespace node {

static QString getMwcNodePath(const QString & network) {
    QString nwPath = network.toLower().contains("main") ? "main" : "floo";
    return ioutils::getAppDataPath("mwc-node") + QDir::separator() + nwPath + QDir::separator();
}

void MwcNodeConfig::setData(QString _network, QString _host, QString _port, QString _secret) {
    network = _network;
    host    = _host;
    port    = _port;
    secret  = _secret;
}


MwcNodeConfig getCurrentMwcNodeConfig(const QString & network) {
    QString walletPath = getMwcNodePath(network);

    // Note, asserts are disbaled because of the first run.

    QStringList lines = util::readTextFile( walletPath + "api_secret" );
    //Q_ASSERT( lines.size()>0 && !lines[0].isEmpty() );

    QString secret;
    if ( lines.size()>0 && !lines[0].isEmpty() )
        secret = lines[0];

    util::ConfigReader reader;
    reader.readConfig(walletPath + "mwc-server.toml");
    //Q_ASSERT( reader.readConfig("chain_type") == network );

    MwcNodeConfig result;
    result.setData( network, reader.getString("host"), reader.getString("port"), secret);
    return result;
}

void updateMwcNodeConfig( const QString & network ) {
    QString walletPath = getMwcNodePath(network);

    if ( ! QDir(walletPath).exists() ) {
        QDir().mkpath(walletPath);
    }

    QString apiSecretFN = walletPath + "api_secret";
    if (!QFile::exists(apiSecretFN) ) {
        QString secret;
        // Since we are targeting 5.9, we can use QRandomGenerator
        // Note: qsrand is not secure at All. For this case it is fine because Local node not expected to be secure,
        // in any case HTTP is used. Secret is mostly for preventing of node usage from wrong network.
        qsrand( static_cast<quint64>( QTime::currentTime().msecsSinceStartOfDay() ) );
        int alphabet = 'z' - 'a' +1;
        for ( int t=0;t<20; t++) {
            int chIdx = qrand() % (alphabet*2);
            if (chIdx < alphabet)
                secret.append( 'a' + chIdx );
            else
                secret.append( 'A' + (chIdx-alphabet) );
        }
        util::writeTextFile(apiSecretFN, {secret} );
    }

    QString mwcServerTomlFN = walletPath + "mwc-server.toml";
    if (! QFile::exists(mwcServerTomlFN) ) {
        QFile::copy( network.toLower().contains("main") ? mwc::MWC_NODE_CONFIG_MAIN : mwc::MWC_NODE_CONFIG_FLOO, mwcServerTomlFN );
        QFile::setPermissions( mwcServerTomlFN, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup );
    }
}

}
