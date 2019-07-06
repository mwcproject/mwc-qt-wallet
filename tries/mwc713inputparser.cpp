#include "mwc713inputparser.h"
#include "simpletriesection.h"
#include <QDebug>
#include <QMetaMethod>
#include <core/global.h>
#include "../util/Log.h"

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
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_BOX_LISTENER,
          QVector<BaseTrieSection*>{
                new TriePhraseSection("starting mwc listener..."),
                new TrieNewLineSection(),
                new TriePhraseSection("listener started for ["),
                new TrieAnySection(100, TrieAnySection::NUMBERS | TrieAnySection::LOW_CASE | TrieAnySection::UPPER_CASE,"","", 1), // mwc MQ address
                new TriePhraseSection("]")
            }));
}

void Mwc713InputParser::initMwcMqAddress() {
        parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_YOUR_MWC_ADDRESS,
              QVector<BaseTrieSection*>{
                 new TriePhraseSection("Your mwc address: "),
                 new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE,"","", 1) // mwc MQ address
              }));

        parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_YOUR_MWC_ADDRESS,
              QVector<BaseTrieSection*>{
                  new TriePhraseSection("Your mwcmq address: "),
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

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_PASSWORD_EXPECTED,
         QVector<BaseTrieSection*>{
              new TriePhraseSection("Set an optional password to secure your wallet with. Leave blank for no password.")
         }));




    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_PASS_PHRASE,
         QVector<BaseTrieSection*>{
              new TriePhraseSection("Your recovery phrase is:"),
              new TrieAnySection(7, TrieAnySection::NEW_LINE | TrieAnySection::SPACES,"",""), // EMpty lines
              new TrieAnySection(512, TrieAnySection::NOT_NEW_LINE,"","", 100), // Pass phrase separated by spaces
              new TrieAnySection(7, TrieAnySection::NEW_LINE | TrieAnySection::SPACES,"",""), // EMpty lines
              new TriePhraseSection("Please back-up these words in a non-digital format"),
         }));



    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_INIT_WANT_ENTER,
         QVector<BaseTrieSection*>{
              new TriePhraseSection("Press ENTER when you have done so")
         }));
}

void Mwc713InputParser::initListening() {
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_MQ_STARTING,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("starting mwcmq listener...")
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_KB_STARTING,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("starting keybase listener...")
                                                }));

    // for testnet
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_ON,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("listener started for ["),
                                                        new TrieAnySection(100, TrieAnySection::NUMBERS | TrieAnySection::LOW_CASE | TrieAnySection::UPPER_CASE,"","", 1), // mwc MQ address
                                                        new TriePhraseSection("]")
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_MQ_STOPPING,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("stopping mwcmq listener...")
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_KB_STOPPING,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("stopping keybase listener...")
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_OFF,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("listener ["),
                                                        new TrieAnySection(100, TrieAnySection::NUMBERS | TrieAnySection::LOW_CASE | TrieAnySection::UPPER_CASE,"","", 1), // mwc MQ address
                                                        new TriePhraseSection("] stopped")
                                                }));

    /////////////////  Listeners
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_MQ_LOST_CONNECTION,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("WARNING: listener ["),
                                                        new TrieAnySection(100, TrieAnySection::NUMBERS | TrieAnySection::LOW_CASE | TrieAnySection::UPPER_CASE,"","", 1), // mwc MQ address
                                                        new TriePhraseSection("] lost connection")
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_LISTENER_MQ_GET_CONNECTION,
                                                QVector<BaseTrieSection*>{
                                                        new TriePhraseSection("INFO: listener ["),
                                                        new TrieAnySection(100, TrieAnySection::NUMBERS | TrieAnySection::LOW_CASE | TrieAnySection::UPPER_CASE,"","", 1), // mwc MQ address
                                                        new TriePhraseSection("] reestablished connection.")
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
                                                        new TrieAnySection(100, TrieAnySection::NOT_NEW_LINE, "","'", 1),
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

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_SLATE_WAS_SENT,
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

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_SLATE_WAS_RECEIVED,
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
                                                        new TrieAnySection(200, TrieAnySection::NOT_NEW_LINE, "","'", 1),
                                                        new TriePhraseSection("' - Block Height: "),
                                                        new TrieAnySection(30, TrieAnySection::NUMBERS, "","", 2),
                                                        new TrieNewLineSection()
                                                }));

    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_TRANSACTION_LOG,
                                                QVector<BaseTrieSection*>{
                                                        new TrieNewLineSection(),
                                                        new TriePhraseSection("Transaction Log - Account '"),
                                                        new TrieAnySection(200, TrieAnySection::NOT_NEW_LINE, "","'", 1),
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
