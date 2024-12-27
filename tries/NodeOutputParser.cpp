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
        case NODE_OUTPUT_EVENT::MWC_NODE_STARTED:  return "MWC_NODE_STARTED";
        case NODE_OUTPUT_EVENT::MWC_NODE_SYNC:  return "MWC_NODE_SYNC";
        // Very first sync events. Archive processing
        // End of sync up (no archive)
        case NODE_OUTPUT_EVENT::RECEIVE_BLOCK_START:  return "RECEIVE_BLOCK_START";
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

    // node Sync process tracking
    // 20241226 10:32:01.802 INFO mwc_servers::mwc::sync::syncer - mwc-node sync status: NoSync
    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::MWC_NODE_SYNC,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection(" - mwc-node sync status: "),
                                                         new TrieAnySection(50, TrieAnySection::LOW_CASE | TrieAnySection::UPPER_CASE, "","", 1),
                                                         new TrieNewLineSection(),
                                                 }));

    // node Sync process tracking
    // 20241226 10:32:01.802 INFO mwc_servers::mwc::sync::syncer - mwc-node sync status: HeaderSync { current_height: 0, archive_height: 2664000 }
    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::MWC_NODE_SYNC,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection(" - mwc-node sync status: "),
                                                         new TrieAnySection(50, TrieAnySection::LOW_CASE | TrieAnySection::UPPER_CASE, "","{", 1), // Type of Sync,
                                                         new TrieAnySection(200, TrieAnySection::NOT_NEW_LINE, "","", 2), // Data, should be in Json like form
                                                         new TrieNewLineSection(),
                                                 }));

    // 20191011 18:09:52.536 INFO grin_servers::common::adapters - Received block 114601 of 45343587 hash 140e019e22d0 from 52.13.204.202:13414 [in/out/kern: 0/1/1] going to process.
    // Really don't case about details. Just need to be aware that block was received
    parser.appendLineParser( new TrieLineParser( (int)NODE_OUTPUT_EVENT::RECEIVE_BLOCK_START,
                                                 QVector<BaseTrieSection *>{
                                                         new TriePhraseSection("common::adapters - Received block "),
                                                         new TrieAnySection(200, TrieAnySection::NUMBERS, "","", 1),
                                                         new TriePhraseSection(" of "),
                                                         new TrieAnySection(200, TrieAnySection::NUMBERS, "","", 2),
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
