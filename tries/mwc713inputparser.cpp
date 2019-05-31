#include "mwc713inputparser.h"
#include "simpletriesection.h"
#include <QDebug>
#include <QMetaMethod>

namespace tries {

Mwc713InputParser::Mwc713InputParser() {
    initWalletReady();
    initWelcome();
    initInit();
    initNeedUnlock();
    initPasswordError(); // notification about the wrong password.
    initBoxListener();
    initGenericError(); // All error messages
    initUnlockMwcAddress();//
}

Mwc713InputParser::~Mwc713InputParser() {}

// Main routine processing with backed wallet printed
// Resilting will be delieved async through signals
void Mwc713InputParser::processInput(QString message) {
    qDebug() << "Processing wallet input: '" << message << "'";

    QVector<ParsingResult> results = parser.processInput(message);

    for (auto & res : results) {
        qDebug() << "Getting results: " << res;

        Q_ASSERT( res.result.parseResult.size() <= 1 );
        QString message;
        if (res.result.parseResult.size()==1)
            message = res.result.parseResult[0].strData;

        emit sgGenericEvent((wallet::WALLET_EVENTS) res.parserId, message);
    }
}

void Mwc713InputParser::initWalletReady() {
    parser.appendLineParser( new TrieLineParser( wallet::WALLET_EVENTS::S_READY,
          QVector<BaseTrieSection*>{
                new TrieNewLineSection(),
                new TriePhaseSection("wallet713>")} ));
}

// Register callbacks for event that we are going to process:
void Mwc713InputParser::initWelcome() {
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_WELCOME,
          QVector<BaseTrieSection*>{
                new TriePhaseSection("Welcome to wallet713 for MWC v"),
                new TrieVersionSection(1), // Let's get version back to verify consistency
                new TrieNewLineSection()} ));
}

void Mwc713InputParser::initInit() {
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_INIT,
          QVector<BaseTrieSection*>{
                new TriePhaseSection("Please choose an option"),
                new TrieNewLineSection(),
                new TriePhaseSection(" 1) init a new wallet"),
                new TrieNewLineSection(),
                new TriePhaseSection(" 2) recover from mnemonic"),
                new TrieNewLineSection(),
                new TriePhaseSection(" 3) exit"),
                new TrieNewLineSection()} ));
}

void Mwc713InputParser::initPasswordError()  // notification about the wrong password.
{
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_PASSWORD_ERROR,
          QVector<BaseTrieSection*>{
                new TriePhaseSection("could not unlock wallet! are you using the correct passphrase?"),
                new TrieNewLineSection()} ));
}

void Mwc713InputParser::initGenericError() {
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_GENERIC_ERROR,
          QVector<BaseTrieSection*>{
                new TrieNewLineSection(),
                new TriePhaseSection("ERROR: "),
                new TrieAnySection(100, TrieAnySection::ANY_UNTIL_NEW_LINE, 1),
                new TrieNewLineSection()} ));
}


void Mwc713InputParser::initNeedUnlock() {
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_NEED2UNLOCK,
          QVector<BaseTrieSection*>{
                new TriePhaseSection("Unlock your existing wallet or type `init` to initiate a new one"),
                new TrieNewLineSection() }));
}

void Mwc713InputParser::initBoxListener() {
    parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_BOX_LISTENER,
          QVector<BaseTrieSection*>{
                new TriePhaseSection("starting mwc listener..."),
                new TrieNewLineSection(),
                new TriePhaseSection("listener started for ["),
                new TrieAnySection(100, TrieAnySection::NUMBERS | TrieAnySection::LOW_CASE | TrieAnySection::UPPER_CASE, 1), // mwc MQ address
                new TriePhaseSection("listener started for ]")
            }));
}

void Mwc713InputParser::initUnlockMwcAddress() {
        parser.appendLineParser( new TrieLineParser(wallet::WALLET_EVENTS::S_YOUR_MWC_ADDRESS,
              QVector<BaseTrieSection*>{
                 new TrieNewLineSection(),
                 new TriePhaseSection("Your mwc address: "),
                 new TrieAnySection(100, TrieAnySection::ANY_UNTIL_NEW_LINE, 1) // mwc MQ address
              }));
    }

}
