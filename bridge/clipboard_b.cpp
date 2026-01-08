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


#include "clipboard_b.h"
#include <QGuiApplication>
#include <QClipboard>
#include "../util/Log.h"

ClipboardProxy::ClipboardProxy(QObject *parent)
    : QObject(parent)
{
    logger::logInfo(logger::BRIDGE, "Call ClipboardProxy::ClipboardProxy");
    QClipboard *clipboard = QGuiApplication::clipboard();
    connect(clipboard, &QClipboard::dataChanged,
                this, &ClipboardProxy::dataChanged);
        connect(clipboard, &QClipboard::selectionChanged,
                this, &ClipboardProxy::selectionChanged);
}

void ClipboardProxy::setDataText(const QString &text)
{
    logger::logInfo(logger::BRIDGE, "Call ClipboardProxy::setDataText with text length " + QString::number(text.length()));
    QGuiApplication::clipboard()->setText(text, QClipboard::Clipboard);
}

QString ClipboardProxy::dataText() const
{
    logger::logInfo(logger::BRIDGE, "Call ClipboardProxy::dataText");
    return QGuiApplication::clipboard()->text(QClipboard::Clipboard);
}

void ClipboardProxy::setSelectionText(const QString &text)
{
    logger::logInfo(logger::BRIDGE, "Call ClipboardProxy::setSelectionText with tex length " + QString::number(text.length()));
    QGuiApplication::clipboard()->setText(text, QClipboard::Selection);
}

QString ClipboardProxy::selectionText() const
{
    logger::logInfo(logger::BRIDGE, "Call ClipboardProxy::selectionText");
    return QGuiApplication::clipboard()->text(QClipboard::Selection);
}
