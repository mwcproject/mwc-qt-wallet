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
        case NODE_OUTPUT_EVENT::NONE: return "NONE";
        case NODE_OUTPUT_EVENT::WAITING_FOR_PEERS: return "WAITING_FOR_PEERS";
        case NODE_OUTPUT_EVENT::INITIAL_CHAIN_HEIGHT: return "INITIAL_CHAIN_HEIGHT";
        case NODE_OUTPUT_EVENT::MWC_NODE_STARTED:  return "MWC_NODE_STARTED";
        // Very first sync events. Archive processing
        case NODE_OUTPUT_EVENT::MWC_NODE_RECEIVE_HEADER:  return "MWC_NODE_RECEIVE_HEADER";
        case NODE_OUTPUT_EVENT::MWC_NODE_RECEIVE_PIBD_BLOCK: return "MWC_NODE_RECEIVE_PIBD_BLOCK";
        case NODE_OUTPUT_EVENT::VERIFY_RANGEPROOFS_FOR_TXHASHSET:  return "VERIFY_RANGEPROOFS_FOR_TXHASHSET";
        case NODE_OUTPUT_EVENT::VERIFY_KERNEL_SIGNATURES:  return "VERIFY_KERNEL_SIGNATURES";
        // End of sync up (no archive)
        case NODE_OUTPUT_EVENT::RECEIVE_BLOCK_START:  return "RECEIVE_BLOCK_START";
        case NODE_OUTPUT_EVENT::SYNC_IS_DONE: return "SYNC_IS_DONE";
        // Normal Workflow
        case NODE_OUTPUT_EVENT::RECEIVE_BLOCK_LISTEN:  return "RECEIVE_BLOCK_LISTEN";
        // Errors
        case NODE_OUTPUT_EVENT::ADDRESS_ALREADY_IN_USE:  return "ADDRESS_ALREADY_IN_USE";
    }
    return "UNKNOWN_NODE_OUTPUT_EVENT";
}


NodeOutputParser::NodeOutputParser() {

    // let's init all tries that we have
    // 20241203 11:52:43.938 WARN mwc_servers::mwc::server - MWC server started.
    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::MWC_NODE_STARTED,
                                                    QVector<BaseTrieSection *>{
                                                            new TriePhraseSection("mwc::server - MWC server started")
                                     }));

    // 20191011 19:13:56.842 INFO grin_servers::grin::sync::syncer - Waiting for the peers
    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::WAITING_FOR_PEERS,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection("sync::syncer - Waiting for the peers")
                                                 }));

    // 20191011 22:43:38.969 INFO mwc_chain::chain - init: sync_head: 365479725 @ 117749 [0099c40fb902]
    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::INITIAL_CHAIN_HEIGHT,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection("mwc_chain::chain - init: sync_head: "),
                                                         new TrieAnySection(50, TrieAnySection::NOT_NEW_LINE, "","", 2) // 365479725 @ 117749 [0099c40fb902]
                                                 }));


    // 20241205 15:52:51.034 INFO mwc_servers::mwc::sync::header_sync - Received 512 block headers from tor://vstdjxrzh67udhm3fedanul2sy7fwudasjmwxy54pady6dxclty2zmqd.onion, height 1375233
    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::MWC_NODE_RECEIVE_HEADER,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection("sync::header_sync - Received "),
                                                         new TrieAnySection(20, TrieAnySection::NUMBERS, "","", 1),
                                                         new TriePhraseSection(" block headers from"),
                                                         new TrieAnySection(50, TrieAnySection::NOT_NEW_LINE, "","", 2) // 3.226.135.253:13414, height 31361
                                                 }));

    // 20240909 20:36:09.141 INFO grin_chain::types - PIBD sync progress: 65536 from 4069364
    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::MWC_NODE_RECEIVE_PIBD_BLOCK,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection("desegmenter - PIBD sync progress: "),
                                                         new TrieAnySection(20, TrieAnySection::NUMBERS, "","", 1),
                                                         new TriePhraseSection(" from "),
                                                         new TrieAnySection(50, TrieAnySection::NUMBERS, "","", 2)
                                                 }));


    // 20240910 17:07:25.609 INFO grin_chain::txhashset::txhashset - txhashset: verify_rangeproofs: verified 29000 rangeproofs from 8734534576
    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::VERIFY_RANGEPROOFS_FOR_TXHASHSET,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection("txhashset::txhashset - txhashset: verify_rangeproofs: verified "),
                                                         new TrieAnySection(20, TrieAnySection::NUMBERS, "","", 1),
                                                         new TriePhraseSection(" rangeproofs from "),
                                                         new TrieAnySection(20, TrieAnySection::NUMBERS, "","", 2),
                                                 }));

    // 20191011 18:07:37.377 INFO grin_chain::txhashset::txhashset - txhashset: verify_kernel_signatures: verified 61000 signatures from 9483754384
    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::VERIFY_KERNEL_SIGNATURES,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection("txhashset::txhashset - txhashset: verify_kernel_signatures: verified "),
                                                         new TrieAnySection(20, TrieAnySection::NUMBERS, "","", 1),
                                                         new TriePhraseSection(" signatures from "),
                                                         new TrieAnySection(20, TrieAnySection::NUMBERS, "","", 2)
                                                 }));

    // THose blocks come after archive
    // 20191011 18:09:52.536 INFO grin_servers::common::adapters - Received block 114601 of 45343587 hash 140e019e22d0 from 52.13.204.202:13414 [in/out/kern: 0/1/1] going to process.
    // Really don't case about details. Just need to be aware that block was received
    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::RECEIVE_BLOCK_START,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection("common::adapters - Received block "),
                                                         new TrieAnySection(200, TrieAnySection::NUMBERS, "","", 1),
                                                         new TriePhraseSection(" of "),
                                                         new TrieAnySection(200, TrieAnySection::NUMBERS, "","", 2),
                                                 }));

    // Sync is done
    // 20191011 18:15:44.002 INFO grin_servers::grin::sync::syncer - synchronized at 365444412 @ 117485 [0d4879faafaa]
    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::SYNC_IS_DONE,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection("body_sync - synchronized at "),
                                                         new TrieAnySection(200, TrieAnySection::NOT_NEW_LINE, "","", 2), // 365444412 @ 117485 [0d4879faafaa]
                                                 }));


    // common::hooks - Received block 2a695957b396 at 102204 from 34.238.121.224:13414 [in/out/kern: 0/1/1] going to process.
    // don't case about details. Just need to be aware that block was received
    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::RECEIVE_BLOCK_LISTEN,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection("common::hooks - Received block "),
                                                         new TrieAnySection(200, TrieAnySection::NOT_NEW_LINE, "","", 2)
                                                 }));

    // ERROR grin_servers::grin::server - P2P server failed with erorr: Connection(Os { code: 48, kind: AddrInUse, message: "Address already in use" })
    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::ADDRESS_ALREADY_IN_USE,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection("P2P server failed with erorr")
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
