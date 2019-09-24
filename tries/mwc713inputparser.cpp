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

#include "mwc713inputparser.h"
#include "simpletriesection.h"
#include <QDebug>
#include <QMetaMethod>
#include <core/global.h>
#include "../util/Log.h"
#include "../core/Config.h"

namespace tries {

Mwc713InputParser::Mwc713InputParser() {
    initWalletReady();
    initWelcome();
    initInit();
    initNeedUnlock();
    initPasswordError(); // notification about the wrong password.
    initBoxListener();
    initGenericError(); // All error messages
    initMwcMqAddress();//
    initInitWalletWorkflow();
    initAccount();
    initSend();
    initTransactions();

    initListening();
    initRecovery();
}

Mwc713InputParser::~Mwc713InputParser() {}

// Main routine processing with backed wallet printed
// Resilting will be delieved async through signals
void Mwc713InputParser::processInput(QString message) {
    qDebug() << "Processing wallet input: '" << message << "'";

    QVector<ParsingResult> results = parser.processInput(message);

    for (auto & res : results) {
        qDebug() << "Getting results: " << res;

        QString message;
        for (auto & pr : res.result.parseResult) {
            if (message.length()>0)
                message += "|";
            message += pr.strData;
        }

        wallet::WALLET_EVENTS evt = (wallet::WALLET_EVENTS) res.parserId;

        logger::logParsingEvent( evt, message );

        emit sgGenericEvent( evt, message);
    }
}

void Mwc713InputParser::initWalletReady() {
    parser.appendLineParser( new TrieLineParser( wallet::WALLET_EVENTS::S_READY,
          QVector<BaseTrieSection*>{
                new TriePhraseSection( mwc::PROMPTS_MWC713)} ));
}

// Register callbacks for event that we are going to process:
void Mwc713InputParser::initWelcome() {
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_WELCOME,
          QVector<BaseTrieSection*>{
                new TriePhraseSection("Welcome to wallet713 for MWC v"),
                new TrieVersionSection(1), // Let's get version back to verify consistency
                new TrieNewLineSection()} ));
}

void Mwc713InputParser::initInit() {
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_INIT,
          QVector<BaseTrieSection*>{
                new TriePhraseSection("Please choose an option"),
                new TrieNewLineSection(),
                new TriePhraseSection(" 1) init a new wallet"),
                new TrieNewLineSection(),
                new TriePhraseSection(" 2) recover from mnemonic"),
                new TrieNewLineSection(),
                new TriePhraseSection(" 3) exit"),
                new TrieNewLineSection()} ));
}

void Mwc713InputParser::initPasswordError()  // notification about the wrong password.
{
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_PASSWORD_ERROR,
          QVector<BaseTrieSection*>{
                new TriePhraseSection("could not unlock wallet! are you using the correct passphrase?"),
                new TrieNewLineSection()} ));
}

void Mwc713InputParser::initGenericError() {
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_GENERIC_ERROR,
          QVector<BaseTrieSection*>{
                new TrieNewLineSection(),
                new TriePhraseSection("ERROR: "),
                new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE,"","", 1)
                } ));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_GENERIC_WARNING,
                                                QVector<BaseTrieSection*>{
                                                        new TrieNewLineSection(),
                                                        new TriePhraseSection("WARNING: "),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE, "","", 1)
                                                        } ));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_GENERIC_INFO,
                                                QVector<BaseTrieSection*>{
                                                        new TrieNewLineSection(),
                                                        new TriePhraseSection("INFO: "),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE, "","", 1)
                                                        } ));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_ERROR,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("error: "),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE, "","", 1)
                                                } ));
}


void Mwc713InputParser::initNeedUnlock() {
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_NEED2UNLOCK,
          QVector<BaseTrieSection*>{
                new TriePhraseSection("Unlock your existing wallet or type `init` to initiate a new one"),
                new TrieNewLineSection() }));
}

void Mwc713InputParser::initBoxListener() {
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_MWC_MQ_LISTENER,
          QVector<BaseTrieSection*>{
                config::getUseMwcMqS() ? new TriePhraseSection("starting mwcmqs listener...") : new TriePhraseSection("starting mwcmq listener..."),
                new TrieNewLineSection(),
                config::getUseMwcMqS() ? new TriePhraseSection("mwcmqs listener started for [") : new TriePhraseSection("listener started for ["),
                new TrieAnySection(100, TrieAnySection::NUMBERS | TrieAnySection::LOW_CASE | TrieAnySection::UPPER_CASE,"","", 1), // mwc MQ address
                new TriePhraseSection("]")
            }));
}

void Mwc713InputParser::initMwcMqAddress() {
        parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_YOUR_MWC_ADDRESS,
              QVector<BaseTrieSection*>{
                      new TriePhraseSection("Your mwcmq address: "),   // 'Your mwcmq address' cover both mq & mqs cases
                  new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE,"","", 1) // mwc MQ address
              }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_MWC_ADDRESS_INDEX,
                                                QVector<BaseTrieSection*>{
                                                        new TrieNewLineSection(),
                                                        new TriePhraseSection("Derived with index ["),
                                                        new TrieAnySection(100, TrieAnySection::NUMBERS,"","", 1), // mwc MQ address
                                                        new TriePhraseSection("]"),
                                                }));
}

void Mwc713InputParser::initInitWalletWorkflow() {

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_INIT_WANT_ENTER,
         QVector<BaseTrieSection*>{
              new TriePhraseSection("Press ENTER when you have done so")
         }));
}

void Mwc713InputParser::initListening() {
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_MQ_STARTING,
                                                QVector<BaseTrieSection*>{
                                                        config::getUseMwcMqS() ? new TriePhraseSection("starting mwcmqs listener...") : new TriePhraseSection("starting mwcmq listener...")
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_KB_STARTING,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("starting keybase listener...")
                                                }));

    // mwc713 emitting messages 'listener started for: ['  and 'listener started for ['
    // mwcmqs listener started for: [xmjJGkX9U75Vo8Ro26gTm2i4k4CD39Q24qvQqAPeQVeWuo36YVFh] tid=[xa5ktaMRCEmj151Rfxr7a]
    if (config::getUseMwcMqS()) {
        parser.appendLineParser(new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_ON,
                                                   QVector<BaseTrieSection *>{
                                                           new TriePhraseSection("mwcmqs listener started for: ["),
                                                           new TrieAnySection(100, TrieAnySection::NUMBERS | TrieAnySection::LOW_CASE | TrieAnySection::UPPER_CASE, "", "", 1), // mwc MQ address
                                                           new TriePhraseSection("] tid=["),
                                                           new TrieAnySection(100, TrieAnySection::NOT_SPACES, "", "]", 3), // thread ID
                                                           new TriePhraseSection("]")
                                                   }));

        // Keybase separately
        parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_ON,
                                                    QVector<BaseTrieSection*>{
                                                            new TriePhraseSection("listener started for ["),
                                                            new TriePhraseSection("keybase]", 1) // keybase need go to accumulators, last symbol will be skipped
                                                    }));

    }
    else {
        parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_ON,
                                                    QVector<BaseTrieSection*>{
                                                            new TriePhraseSection("listener started for ["),
                                                            new TrieAnySection(100, TrieAnySection::NUMBERS | TrieAnySection::LOW_CASE | TrieAnySection::UPPER_CASE,"","", 1), // mwc MQ address
                                                            new TriePhraseSection("]")
                                                    }));
    }


    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_MQ_STOPPING,
                                                QVector<BaseTrieSection*>{
                                                        config::getUseMwcMqS() ? new TriePhraseSection("stopping mwcmqs listener...") : new TriePhraseSection("stopping mwcmq listener...")
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_KB_STOPPING,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("stopping keybase listener...")
                                                }));


    if (config::getUseMwcMqS()) {
        // mwcmqs listener [xmjJGkX9U75Vo8Ro26gTm2i4k4CD39Q24qvQqAPeQVeWuo36YVFh] stopped. tid=[xa5ktaMRCEmj151Rfxr7a]
        parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_OFF,
                                                    QVector<BaseTrieSection*>{
                                                            new TriePhraseSection("mwcmqs listener ["),
                                                            new TrieAnySection(100, TrieAnySection::NUMBERS | TrieAnySection::LOW_CASE | TrieAnySection::UPPER_CASE,"","", 1), // mwc MQ address
                                                            new TriePhraseSection("] stopped. tid=["),
                                                            new TrieAnySection(100, TrieAnySection::NOT_SPACES, "", "]", 3), // thread ID
                                                            new TriePhraseSection("]")
                                                    }));

        // Keysbase separately
        parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_OFF,
                                                    QVector<BaseTrieSection*>{
                                                            new TriePhraseSection("listener ["),
                                                            new TriePhraseSection("keybase]", 1), // keybase need go to accumulators, last symbol will be skipped
                                                            new TriePhraseSection(" stopped")
                                                    }));
    }
    else {
        // Both keybase & mwc mq
        parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_OFF,
                                                    QVector<BaseTrieSection*>{
                                                            new TriePhraseSection("listener ["),
                                                            new TrieAnySection(100, TrieAnySection::NUMBERS | TrieAnySection::LOW_CASE | TrieAnySection::UPPER_CASE,"","", 1), // mwc MQ address
                                                            new TriePhraseSection("] stopped")
                                                    }));
    }


    /////////////////  Listeners

    if (config::getUseMwcMqS()) {
        // WARNING: mwcmqs listener [xmjJGkX9U75Vo8Ro26gTm2i4k4CD39Q24qvQqAPeQVeWuo36YVFh] lost connection. Will try to restore in the background. tid=[ToMFBchztyUT0OgPTzeK6]
        parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_MQ_LOST_CONNECTION,
                                                    QVector<BaseTrieSection*>{
                                                            new TriePhraseSection("WARNING: mwcmqs listener ["),
                                                            new TrieAnySection(100, TrieAnySection::NUMBERS | TrieAnySection::LOW_CASE | TrieAnySection::UPPER_CASE,"","", 1), // mwc MQ address
                                                            new TriePhraseSection("] lost connection. Will try to restore in the background. tid=["),
                                                            new TrieAnySection(100, TrieAnySection::NOT_SPACES, "", "]", 3), // thread ID
                                                            new TriePhraseSection("]")
                                                    }));

        // INFO: mwcmqs listener [xmjJGkX9U75Vo8Ro26gTm2i4k4CD39Q24qvQqAPeQVeWuo36YVFh] reestablished connection. tid=[ToMFBchztyUT0OgPTzeK6]
        parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_MQ_GET_CONNECTION,
                                                    QVector<BaseTrieSection*>{
                                                            new TriePhraseSection("INFO: mwcmqs listener ["),
                                                            new TrieAnySection(100, TrieAnySection::NUMBERS | TrieAnySection::LOW_CASE | TrieAnySection::UPPER_CASE,"","", 1), // mwc MQ address
                                                            new TriePhraseSection("] reestablished connection. tid=["),
                                                            new TrieAnySection(100, TrieAnySection::NOT_SPACES, "", "]", 3), // thread ID
                                                            new TriePhraseSection("]")
                                                    }));
    }
    else {
        // MWC MQ
        // WARNING: mwcmqs listener [xmjJGkX9U75Vo8Ro26gTm2i4k4CD39Q24qvQqAPeQVeWuo36YVFh] lost connection. Will try to restore in the background.
        parser.appendLineParser(new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_MQ_LOST_CONNECTION,
                                                   QVector<BaseTrieSection *>{
                                                           new TriePhraseSection("WARNING: listener ["),
                                                           new TrieAnySection(100, TrieAnySection::NUMBERS | TrieAnySection::LOW_CASE | TrieAnySection::UPPER_CASE, "", "", 1), // mwc MQ address
                                                           new TriePhraseSection("] lost connection")
                                                   }));

        // INFO: mwcmqs listener [xmjJGkX9U75Vo8Ro26gTm2i4k4CD39Q24qvQqAPeQVeWuo36YVFh] reestablished connection.
        parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_MQ_GET_CONNECTION,
                                                    QVector<BaseTrieSection*>{
                                                            new TriePhraseSection("INFO: listener ["),
                                                            new TrieAnySection(100, TrieAnySection::NUMBERS | TrieAnySection::LOW_CASE | TrieAnySection::UPPER_CASE,"","", 1), // mwc MQ address
                                                            new TriePhraseSection("] reestablished connection.")
                                                    }));
    }



    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_KB_LOST_CONNECTION,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("listener [keybase] lost connection.")
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_KB_GET_CONNECTION,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("listener [keybase] reestablished connection.")
                                                }));

    // Need to detect a case when listening collision heppans. Then we will emit message about that
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_MQ_COLLISION,
                          QVector<BaseTrieSection*>{
                                   new TriePhraseSection("new login detected. mwcmqs listener will stop!")
                          }));
    // Failed to start detection needed because it will stop to reconnection. Need to handle that
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_MQ_FAILED_TO_START,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("Failed to start mwcmqs subscriber.")    // Full message: ERROR: Failed to start mwcmqs subscriber. Error connecting to mqs.mwc.mw:443
                                                }));
}

void Mwc713InputParser::initRecovery() {
    // For very forst run
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_RECOVERY_MNEMONIC,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("Recovering from mnemonic"),
                                                        new TrieNewLineSection(),
                                                        new TriePhraseSection("Mnemonic:")
                                                }));

    // for normal run
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_RECOVERY_STARTING,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("recovering... please wait as this could take a few minutes to complete")
                                                }));
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_RECOVERY_DONE,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("wallet restoration done!")
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_RECOVERY_PROGRESS,
                                                // Example: Checking 1000 outputs, up to index 13433. (Highest index: 12235)
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("Checking "),
                                                        new TrieAnySection(100, TrieAnySection::NUMBERS,"",""), // skipping
                                                        new TriePhraseSection(" outputs, up to index "),
                                                        new TrieAnySection(100, TrieAnySection::NUMBERS,"","", 1), // mav
                                                        new TriePhraseSection(". (Highest index: "),
                                                        new TrieAnySection(100, TrieAnySection::NUMBERS,"","", 2), // current position
                                                        new TriePhraseSection(")")
                                                }));

    //
}

void Mwc713InputParser::initAccount() {
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_ACCOUNTS_INFO_SUM,
                                                QVector<BaseTrieSection*>{
                                                        new TrieNewLineSection(),
                                                        new TriePhraseSection("____ Wallet Summary Info - Account '"),
                                                        // account name,
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE | TrieAnySection::START_NEXT_EVERY_TRY, "","", 1),
                                                        new TriePhraseSection("' as of height "),
                                                        // Height
                                                        new TrieAnySection(100, TrieAnySection::NUMBERS, "","", 2 ),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE, "","")
                                                }));
}

void Mwc713InputParser::initSend() {

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_SET_RECEIVE,
                                                QVector<BaseTrieSection*>{
                                                        new TrieNewLineSection(),
                                                        new TriePhraseSection("Incoming funds will be received in account: \""),
                                                        new TrieAnySection(4000, TrieAnySection::NOT_NEW_LINE, "","\"", 1)
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_SLATE_WAS_SENT_TO,
    // slate [dd5a7ca7-a023-4fb3-b8fd-93b35ac45c8b] for [0.100000000] MWCs sent successfully to [xmgEvZ4MCCGMJnRnNXKHBbHmSGWQchNr9uZpY5J1XXnsCFS45fsU]
                                                QVector<BaseTrieSection*>{
                                                        new TrieNewLineSection(),
                                                        new TriePhraseSection("slate ["),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE|TrieAnySection::NOT_SPACES, "","]", 1),
                                                        new TriePhraseSection("] for ["),
                                                        new TrieAnySection(100, TrieAnySection::NUMBERS, ".","]", 2),
                                                        new TriePhraseSection("] MWCs sent successfully to ["),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE, "","]", 3),
                                                        new TriePhraseSection("]"),
                                                        new TrieNewLineSection()
                                                }));
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_SLATE_WAS_SENT_BACK,
            // slate [dd5a7ca7-a023-4fb3-b8fd-93b35ac45c8b] for [0.100000000] MWCs sent successfully to [xmgEvZ4MCCGMJnRnNXKHBbHmSGWQchNr9uZpY5J1XXnsCFS45fsU]
                                                QVector<BaseTrieSection*>{
                                                        new TrieNewLineSection(),
                                                        new TriePhraseSection("slate ["),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE|TrieAnySection::NOT_SPACES, "","]", 1),
                                                        new TriePhraseSection("] sent back to ["),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE, "","]", 2),
                                                        new TriePhraseSection("] successfully"),
                                                        new TrieNewLineSection()
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_SLATE_WAS_RECEIVED_BACK,
                                                QVector<BaseTrieSection*>{
                                                        new TrieNewLineSection(),
                                                        new TriePhraseSection("slate ["),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE|TrieAnySection::NOT_SPACES, "","]", 1),
                                                        new TriePhraseSection("] received back from ["),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE, "","]", 2),
                                                        new TriePhraseSection("] for ["),
                                                        new TrieAnySection(100, TrieAnySection::NUMBERS, ".","]", 3),
                                                        new TriePhraseSection("] MWCs"),
                                                        new TrieNewLineSection()
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_SLATE_WAS_RECEIVED_FROM,
     //slate [5a759d16-f6b1-41d4-8d44-49307a50e09a] received from [xmgcJYZG6eG5ajHdZZGh8gXv5Ne4rdArrKwpSajQGhenUXdJQA5V] for [0.111000000] MWCs. Message: ["L to r 0.111 mwc"]
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("slate ["),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE|TrieAnySection::NOT_SPACES, "","]", 1),
                                                        new TriePhraseSection("] received from ["),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE, "","]", 2),
                                                        new TriePhraseSection("] for ["),
                                                        new TrieAnySection(100, TrieAnySection::NUMBERS, ".","]", 3),
                                                        new TriePhraseSection("] MWCs. Message: ["),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE, "","]", 4)
                                                }));
    // Without message case. Line MUST end
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_SLATE_WAS_RECEIVED_FROM,
            // slate [b2822262-4760-4907-923f-e2459ed5d554] received from [jbyrer] for [1.000000000] MWCs
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("slate ["),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE|TrieAnySection::NOT_SPACES, "","]", 1),
                                                        new TriePhraseSection("] received from ["),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE, "","]", 2),
                                                        new TriePhraseSection("] for ["),
                                                        new TrieAnySection(100, TrieAnySection::NUMBERS, ".","]", 3),
                                                        new TriePhraseSection("] MWCs."),
                                                        new TrieNewLineSection()
                                                }));


    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_SLATE_WAS_FINALIZED,
                                                QVector<BaseTrieSection*>{
                                                        new TrieNewLineSection(),
                                                        new TriePhraseSection("slate ["),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE|TrieAnySection::NOT_SPACES, "","]", 1),
                                                        new TriePhraseSection("] finalized successfully"),
                                                        new TrieNewLineSection()
                                                }));
}

void Mwc713InputParser::initTransactions() {
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_OUTPUT_LOG,
                                                QVector<BaseTrieSection*>{
                                                        new TrieNewLineSection(),
                                                        new TriePhraseSection("Wallet Outputs - Account '"),
                                                        // Account name have extra character!
                                                        new TrieAnySection(200, TrieAnySection::NOT_NEW_LINE|TrieAnySection::START_NEXT_EVERY_TRY, "","", 1),
                                                        new TriePhraseSection("' - Block Height: "),
                                                        new TrieAnySection(30, TrieAnySection::NUMBERS, "","", 2),
                                                        new TrieNewLineSection()
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_TRANSACTION_LOG,
                                                QVector<BaseTrieSection*>{
                                                        new TrieNewLineSection(),
                                                        new TriePhraseSection("Transaction Log - Account '"),
                                                        // Account name have extra character!
                                                        new TrieAnySection(200, TrieAnySection::NOT_NEW_LINE|TrieAnySection::START_NEXT_EVERY_TRY, "","", 1),
                                                        new TriePhraseSection("' - Block Height: "),
                                                        new TrieAnySection(30, TrieAnySection::NUMBERS, "","", 2),
                                                        new TrieNewLineSection()
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LINE,
                                                QVector<BaseTrieSection*>{
                                                        new TrieNewLineSection(),
                                                        new TrieAnySection(512, TrieAnySection::NOT_NEW_LINE, "", "", 1),
                                                        new TrieNewLineSection()
                                                }));
}


}
