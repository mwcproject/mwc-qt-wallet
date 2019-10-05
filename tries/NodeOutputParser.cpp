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

#include "NodeOutputParser.h"
#include "../util/Log.h"
#include "../tries/simpletriesection.h"
#include "../tries/baseparser.h"

namespace tries {

QString toString(NODE_OUTPUT_EVENT event) {
    switch (event) {
        case NODE_OUTPUT_EVENT::MWC_NODE_STARTED:  return "MWC_NODE_STARTED";
        // Very first sync events. Archive processing
        case NODE_OUTPUT_EVENT::MWC_NODE_RECEIVE_HEADER:  return "MWC_NODE_RECEIVE_HEADER";
        case NODE_OUTPUT_EVENT::ASK_FOR_TXHASHSET_ARCHIVE:  return "ASK_FOR_TXHASHSET_ARCHIVE";
        case NODE_OUTPUT_EVENT::HANDLE_TXHASHSET_ARCHIVE:  return "HANDLE_TXHASHSET_ARCHIVE";
        case NODE_OUTPUT_EVENT::VERIFY_RANGEPROOFS_FOR_TXHASHSET:  return "VERIFY_RANGEPROOFS_FOR_TXHASHSET";
        case NODE_OUTPUT_EVENT::VERIFY_KERNEL_SIGNATURES:  return "VERIFY_KERNEL_SIGNATURES";
        // End of sync up (no archive)
        case NODE_OUTPUT_EVENT::RECIEVE_BLOCK_HEADERS_START:  return "RECIEVE_BLOCK_HEADERS_START";
        case NODE_OUTPUT_EVENT::RECEIVE_BLOCK_START:  return "RECEIVE_BLOCK_START";
        // Normal Workflow
        case NODE_OUTPUT_EVENT::RECEIVE_BLOCK_LISTEN:  return "RECEIVE_BLOCK_LISTEN";
        // Errors
        case NODE_OUTPUT_EVENT::NETWORK_ISSUES:  return "NETWORK_ISSUES";
        case NODE_OUTPUT_EVENT::ADDRESS_ALREADY_IN_USE:  return "ADDRESS_ALREADY_IN_USE";
    }
}


NodeOutputParser::NodeOutputParser() {

    // let's init all tries that we have
    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::MWC_NODE_STARTED,
                                                    QVector<BaseTrieSection *>{
                                                            new TriePhraseSection("grin::server - MWC server started")
                                     }));

    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::MWC_NODE_RECEIVE_HEADER,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection("common::adapters - Received "),
                                                         new TrieAnySection(20, TrieAnySection::NUMBERS, "","", 1),
                                                         new TriePhraseSection(" block headers from")
                                                 }));

    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::ASK_FOR_TXHASHSET_ARCHIVE,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection("peer - Asking 34.238.121.224:13414 for txhashset archive at "),
                                                         new TrieAnySection(20, TrieAnySection::NOT_SPACES, "","", 1),
                                                         new TriePhraseSection(" for txhashset archive at")
                                                 }));

    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::HANDLE_TXHASHSET_ARCHIVE,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection("handle_payload: txhashset archive for")
                                                 }));

    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::VERIFY_RANGEPROOFS_FOR_TXHASHSET,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection("txhashset::txhashset - txhashset: verify_rangeproofs: verified "),
                                                         new TrieAnySection(20, TrieAnySection::NUMBERS, "","", 1),
                                                         new TriePhraseSection(" rangeproofs")
                                                 }));

    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::VERIFY_KERNEL_SIGNATURES,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection("txhashset::txhashset - txhashset: verify_kernel_signatures: verified "),
                                                         new TrieAnySection(20, TrieAnySection::NUMBERS, "","", 1),
                                                         new TriePhraseSection(" signatures")
                                                 }));

    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::RECIEVE_BLOCK_HEADERS_START,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection("common::adapters - Received "),
                                                         new TrieAnySection(20, TrieAnySection::NUMBERS, "","", 1),
                                                         new TriePhraseSection(" block headers from")
                                                 }));

    // grin_servers::common::adapters - Received block 03bd17d68a32 at 99310 from 52.13.204.202:13414 [in/out/kern: 0/1/1] going to process.
    // Really don't case about details. Just need to be aware that block was received
    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::RECEIVE_BLOCK_START,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection("common::adapters - Received block ")
                                                 }));

    // common::hooks - Received block 2a695957b396 at 102204 from 34.238.121.224:13414 [in/out/kern: 0/1/1] going to process.
    // don't case about details. Just need to be aware that block was received
    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::RECEIVE_BLOCK_LISTEN,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection("common::hooks - Received block ")
                                                 }));

    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::NETWORK_ISSUES,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection("sync::syncer - sync: no peers available, disabling sync")
                                                 }));

    // ERROR grin_servers::grin::server - P2P server failed with erorr: Connection(Os { code: 48, kind: AddrInUse, message: "Address already in use" })
    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::ADDRESS_ALREADY_IN_USE,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection("Address already in use")
                                                 }));

}

NodeOutputParser::~NodeOutputParser() {
}

// Main routine processing with backed wallet printed
// Results will be delieved async through signals
void NodeOutputParser::processInput(QString message) {
    qDebug() << "Processing wallet input: '" << message << "'";

    QVector<ParsingResult> results = parser.processInput(message);

    for (auto &res : results) {
        qDebug() << "Getting results: " << res;

        QString message;
        for (auto &pr : res.result.parseResult) {
            if (message.length() > 0)
                message += "|";
            message += pr.strData;
        }

        NODE_OUTPUT_EVENT evt = (NODE_OUTPUT_EVENT) res.parserId;

        logger::logNodeEvent(evt, message);

        emit nodeOutputGenericEvent(evt, message);
    }
}


}