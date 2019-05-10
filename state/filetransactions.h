#ifndef FILETRANSACTIONS_H
#define FILETRANSACTIONS_H

#include "state.h"
#include "../wallet/wallet.h"

namespace state {


class FileTransactions : public State
{
public:
    FileTransactions(const StateContext & context);
    virtual ~FileTransactions() override;

    // Generate a file with send coins transaction
    QPair<bool, QString> generateSendCoinsFile( long nanoCoins, QString filename );

    // Sign transaction, process response file
    QPair<bool, QString> signTransaction( QString inFileName, QString resultFileName );

    // Processing response file. Close transaction and publish it.
    QPair<bool, QString> processResponseFile( QString responseFileName );

    QString getFileGenerationPath();
    void updateFileGenerationPath(QString path);

protected:
    virtual NextStateRespond execute() override;

};

}

#endif // FILETRANSACTIONS_H
