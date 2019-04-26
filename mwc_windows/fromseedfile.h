#ifndef FROMSEEDFILE_H
#define FROMSEEDFILE_H

#include "../wnd_core/walletwindow.h"

namespace Ui {
class FromSeedFile;
}

class FromSeedFile : public WalletWindow
{
    Q_OBJECT

public:
    explicit FromSeedFile(WalletWindowType wwType, QWidget *parent);
    virtual ~FromSeedFile() override;

    const QString & getSeedFileName() const {return seedFileName;}
    const QString & getSeedPassword() const {return seedPassword;}

protected:
    bool validateData() override;

private slots:
    void on_browseButton_clicked();

private:
    Ui::FromSeedFile *ui;
    QString seedFileName;
    QString seedPassword;
};

#endif // FROMSEEDFILE_H
