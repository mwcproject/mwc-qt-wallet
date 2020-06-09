// Copyright 2020 The MWC Developers
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

#include "node_b.h"
#include "../node/MwcNode.h"
#include "../state/state.h"

namespace bridge {

static node::MwcNode * getNode() {
    auto res = state::getStateContext()->mwcNode;
    Q_ASSERT(res);
    return res;
}

Node::Node(QObject *parent) : QObject(parent) {

    node::MwcNode * node = getNode();

    QObject::connect(node, &node::MwcNode::onMwcOutputLine,
                     this, &Node::onMwcOutputLine, Qt::QueuedConnection);
    QObject::connect(node, &node::MwcNode::onMwcStatusUpdate,
                     this, &Node::onMwcStatusUpdate, Qt::QueuedConnection);
}

Node::~Node() {}

// Node log location
QString Node::getLogsLocation() {
    return getNode()->getLogsLocation();
}

QStringList Node::getOutputLines() {
    return getNode()->getOutputLines();
}

void Node::onMwcOutputLine(QString line) {
    emit sgnMwcOutputLine(line);
}

void Node::onMwcStatusUpdate(QString status) {
    emit sgnMwcStatusUpdate(status);
}



}