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

Mwc713InputParser::Mwc713InputParser() :
        msgMapper(":/resource/mwc713_mappers.txt")
{
    initWalletReady();
    initWelcome();
    initInit();
    initNeedUnlock();
    initPasswordError(); // notification about the wrong password.
    initGenericError(); // All error messages
    initMwcMqAddress();//
    initInitWalletWorkflow();
    initAccount();
    initSend();
    initTransactions();
    initListening();
    initRecovery();
    initSyncProgress();
    initSwaps();
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

        if (evt == wallet::WALLET_EVENTS::S_GENERIC_ERROR || evt == wallet::WALLET_EVENTS::S_GENERIC_WARNING || evt == wallet::WALLET_EVENTS::S_GENERIC_INFO) {
            QString m = msgMapper.processMessage(message);
            if (m != message) {
                qDebug() << "Message is updated to " << m;
                message = m;
            }
        }

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
                                                        new TriePhraseSection("could not unlock wallet! "),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE, "","", 1),
                                                        new TrieNewLineSection()} ));
}

void Mwc713InputParser::initGenericError() {
    // Keeping single context because we want to parce once something like this: 'error: another error: API error: not able to connect'
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_GENERIC_ERROR,
          QVector<BaseTrieSection*>{
                new TrieNewLineSection(),
                new TriePhraseSection("ERROR: ", true),
                new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE,"","", 1)
                } ), true);

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_GENERIC_WARNING,
                                                QVector<BaseTrieSection*>{
                                                        new TrieNewLineSection(),
                                                        new TriePhraseSection("WARNING: ", true),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE, "","", 1)
                                                        } ), true);

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_GENERIC_INFO,
                                                QVector<BaseTrieSection*>{
                                                        new TrieNewLineSection(),
                                                        new TriePhraseSection("INFO: ", true),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE, "","", 1)
                                                        } ), true);

    // Post TX Error: Request error: Error code: 500 Internal Server Error; Description: failed: Internal error: Failed to update pool
    // API errors:  Request error: Error code: 500 Internal Server Error; Description: failed: Internal error: Failed to update pool
    // Error code: {}; Description: {}
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_NODE_API_ERROR,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("Error code: "),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE | TrieAnySection::START_NEXT_EVERY_TRY, "","", 1),
                                                        new TriePhraseSection("; Description: "),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE, "","", 2),
                                                } ));
}


void Mwc713InputParser::initNeedUnlock() {
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_NEED2UNLOCK,
          QVector<BaseTrieSection*>{
                new TriePhraseSection("Unlock your existing wallet or type `init` to initiate a new one"),
                new TrieNewLineSection() }));
}

void Mwc713InputParser::initMwcMqAddress() {
        parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_YOUR_MWC_ADDRESS,
              QVector<BaseTrieSection*>{
                      new TriePhraseSection("Your mwcmqs address: "),   // 'Your mwcmq address' cover both mq & mqs cases
                  new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE,"","", 1) // mwc MQ address
              }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_MWC_ADDRESS_INDEX,
                                                QVector<BaseTrieSection*>{
                                                        new TrieNewLineSection(),
                                                        new TriePhraseSection("Derived with index ["),
                                                        new TrieAnySection(100, TrieAnySection::NUMBERS,"","", 1),
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

    // mwc713 emitting messages 'listener started for: ['  and 'listener started for ['
    // mwcmqs listener started for [xmjJGkX9U75Vo8Ro26gTm2i4k4CD39Q24qvQqAPeQVeWuo36YVFh] tid=[xa5ktaMRCEmj151Rfxr7a]
    parser.appendLineParser(new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_ON,
                                                   QVector<BaseTrieSection *>{
                                                           new TriePhraseSection("mwcmqs listener started for ["),
                                                           new TrieAnySection(100, TrieAnySection::NUMBERS | TrieAnySection::LOW_CASE | TrieAnySection::UPPER_CASE, "@.", "", 1), // mwc MQ address
                                                           new TriePhraseSection("] tid=["),
                                                           new TrieAnySection(100, TrieAnySection::NOT_SPACES, "", "]", 3), // thread ID
                                                           new TriePhraseSection("]")
                                                   }));

    // Tor "(i.e. Tor listener started for [http://qx4szwqcqtzo4e9krca357hskg53pjh2uxhsdo854updvmr3o4msc3qd.onion])"
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_ON,
                                                    QVector<BaseTrieSection*>{
                                                            new TriePhraseSection("Tor listener started for ["),
                                                            new TrieAnySection(100, TrieAnySection::NUMBERS | TrieAnySection::LOW_CASE, "./:", "", 1), // Tor address
                                                            new TriePhraseSection("]")
                                                    }));

    // mwcmqs listener [xmjJGkX9U75Vo8Ro26gTm2i4k4CD39Q24qvQqAPeQVeWuo36YVFh] stopped. tid=[xa5ktaMRCEmj151Rfxr7a]
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_OFF,
                                                    QVector<BaseTrieSection*>{
                                                            new TriePhraseSection("mwcmqs listener ["),
                                                            new TrieAnySection(100, TrieAnySection::NUMBERS | TrieAnySection::LOW_CASE | TrieAnySection::UPPER_CASE,"@.","", 1), // mwc MQ address
                                                            new TriePhraseSection("] stopped. tid=["),
                                                            new TrieAnySection(100, TrieAnySection::NOT_SPACES, "", "]", 3), // thread ID
                                                            new TriePhraseSection("]")
                                                    }));

    // Stopping Tor listener...
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_OFF,
                                                    QVector<BaseTrieSection*>{
                                                            new TriePhraseSection("Stopping Tor listener")
                                                    }));

    /////////////////  Listeners
    // WARNING: mwcmqs listener [xmjJGkX9U75Vo8Ro26gTm2i4k4CD39Q24qvQqAPeQVeWuo36YVFh] lost connection. Will try to restore in the background. tid=[ToMFBchztyUT0OgPTzeK6]
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_MQ_LOST_CONNECTION,
                                                    QVector<BaseTrieSection*>{
                                                            new TriePhraseSection("WARNING: mwcmqs listener ["),
                                                            new TrieAnySection(100, TrieAnySection::NUMBERS | TrieAnySection::LOW_CASE | TrieAnySection::UPPER_CASE,"@.","", 1), // mwc MQ address
                                                            new TriePhraseSection("] lost connection. Will try to restore in the background. tid=["),
                                                            new TrieAnySection(100, TrieAnySection::NOT_SPACES, "", "]", 3), // thread ID
                                                            new TriePhraseSection("]")
                                                    }));

    // INFO: mwcmqs listener [xmjJGkX9U75Vo8Ro26gTm2i4k4CD39Q24qvQqAPeQVeWuo36YVFh] reestablished connection. tid=[ToMFBchztyUT0OgPTzeK6]
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_MQ_GET_CONNECTION,
                                                    QVector<BaseTrieSection*>{
                                                            new TriePhraseSection("INFO: mwcmqs listener ["),
                                                            new TrieAnySection(100, TrieAnySection::NUMBERS | TrieAnySection::LOW_CASE | TrieAnySection::UPPER_CASE,"@.","", 1), // mwc MQ address
                                                            new TriePhraseSection("] reestablished connection. tid=["),
                                                            new TrieAnySection(100, TrieAnySection::NOT_SPACES, "", "]", 3), // thread ID
                                                            new TriePhraseSection("]")
                                                    }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_TOR_LOST_CONNECTION,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("Tor is not responding. Will try to reconnect")
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_TOR_GET_CONNECTION,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("Tor connection reestablished")
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


    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_HTTP_STARTING,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("Starting listener for Foreign API on ["),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE,"","]", 1), // addrees that is listening on...
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_HTTP_FAILED,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("thread 'foreign-api-gotham' panicked at '"),
                                                        new TrieAnySection(500, TrieAnySection::NOT_NEW_LINE,"","'", 1), // Error message, not user friendly (rust way)
                                                }));
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_HTTP_FAILED,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("Foreign API Listener failed. "),
                                                        new TrieAnySection(500, TrieAnySection::NOT_NEW_LINE,"","'", 1), // Error message, not user friendly (rust way)
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

void Mwc713InputParser::initSyncProgress() {

    //Starting UTXO scan, 0% complete
    //Checking 125 outputs, up to index 564667. (Highest index: 564667), 99% complete
    //Identified 0 wallet_outputs as belonging to this wallet, 99% complete
    //Scanning Complete

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_SYNC_PROGRESS,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("Starting UTXO scan, "),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE,"","", 1)  // 0% complete
                                                }));
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_SYNC_PROGRESS,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("Checking "),
                                                        new TrieAnySection(100, TrieAnySection::NUMBERS,"",""),
                                                        new TriePhraseSection(" outputs, up to index "),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE,"","", 1) //  564667. (Highest index: 564667), 99% complete
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_SYNC_PROGRESS,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("Validating outputs, "),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE,"","", 1) //  564667. (Highest index: 564667), 99% complete
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_SYNC_PROGRESS,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("Checking "),
                                                        new TrieAnySection(100, TrieAnySection::NUMBERS,"",""),
                                                        new TriePhraseSection(" blocks, Height: "),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE,"","", 1) //  Checking 16 blocks, Height: 331630 - 331645, 22% complete
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_SYNC_PROGRESS,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("Identified "),
                                                        new TrieAnySection(100, TrieAnySection::NUMBERS,"",""),
                                                        new TriePhraseSection(" wallet_outputs as belonging to this wallet, "),
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE,"","", 1) // 99% complete
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_SYNC_PROGRESS,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("Scanning Complete", 1)
                                                }));
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
                                                        new TriePhraseSection("Incoming funds will be received in account: "),
                                                        new TrieAnySection(4000, TrieAnySection::NOT_NEW_LINE, "","", 1)
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


void Mwc713InputParser::initSwaps() {
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_SWAP_GET_OFFER,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("You get an offer to swap "),
                                                        // Account name have extra character!
                                                        new TrieAnySection(10, TrieAnySection::NOT_SPACES | TrieAnySection::START_NEXT_EVERY_TRY, "","", 1),
                                                        new TriePhraseSection(" to MWC. SwapID is "),
                                                        new TrieAnySection(50, TrieAnySection::NOT_NEW_LINE, "","", 2),
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_SWAP_GET_MESSAGE,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("Processed income message for SwapId "),
                                                        // Account name have extra character!
                                                        new TrieAnySection(50, TrieAnySection::NOT_NEW_LINE, "","", 1),
                                                }));

    // Get accept_offer message from gxcnwi....4twad for off_223
    // Get fail_bidding message from gxcnwi....4twad for off_223
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_MKT_ACCEPT_OFFER,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("Get accept_offer message from "),
                                                        // Account name have extra character!
                                                        new TrieAnySection(100, TrieAnySection::NOT_SPACES | TrieAnySection::START_NEXT_EVERY_TRY, "","", 1),
                                                        new TriePhraseSection(" for "),
                                                        new TrieAnySection(50, TrieAnySection::NOT_NEW_LINE, "","", 2),
                                                }));
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_MKT_FAIL_BIDDING,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("Get fail_bidding message from "),
                                                        // Account name have extra character!
                                                        new TrieAnySection(100, TrieAnySection::NOT_SPACES | TrieAnySection::START_NEXT_EVERY_TRY, "","", 1),
                                                        new TriePhraseSection(" for "),
                                                        new TrieAnySection(50, TrieAnySection::NOT_NEW_LINE, "","", 2),
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_MKT_WINNINER,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("Winning Trade with SwapId "),
                                                        // Account name have extra character!
                                                        new TrieAnySection(50, TrieAnySection::NOT_SPACES | TrieAnySection::START_NEXT_EVERY_TRY, "","", 1),
                                                        new TriePhraseSection(" and tag "),
                                                        new TrieAnySection(50, TrieAnySection::NOT_NEW_LINE, "","", 2),
                                                }));
}


}
