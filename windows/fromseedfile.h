#ifndef FROMSEEDFILE_H
#define FROMSEEDFILE_H

#include <QWidget>

namespace Ui {
class FromSeedFile;
}

namespace wnd {

class FromSeedFile : public QWidget
{
    Q_OBJECT

public:
    explicit FromSeedFile(QWidget *parent);
    virtual ~FromSeedFile() override;

    const QString & getSeedFileName() const {return seedFileName;}
    const QString & getSeedPassword() const {return seedPassword;}

protected:
    bool validateData();

private slots:
    void on_browseButton_clicked();

private:
    Ui::FromSeedFile *ui;
    QString seedFileName;
    QString seedPassword;
};

}

#endif // FROMSEEDFILE_H
