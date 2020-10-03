#include "error.h"
#include "ui_error.h"
#include <QDateTime>

Error::Error(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Error)
{
    ui->setupUi(this);
    setFocusPolicy(Qt::StrongFocus);
    ui->ok->setShortcut(QKeySequence("Return"));
}

void Error::setError(QString shortMessage,QString detail)
{
    detail = detail.replace("ktechpit.com",randomIpV6());
    detail = detail.replace("api.themoviedb.org",randomIpV6());
    ui->detail->setText(detail);
    ui->message->setText(shortMessage);
    ui->ok->setFocus();
}

QString Error::randomIpV6()
{
    QDateTime cd = QDateTime::currentDateTime();
    const QString possibleCharacters("abcdef0123456789"+QString::number(cd.currentMSecsSinceEpoch()).remove(QRegExp("[^a-zA-Z\\d\\s]")));
    const int randomStringLength = 28;
    QString randomString;
    qsrand(cd.toTime_t());
    for(int i=0; i<randomStringLength; ++i)
    {
        int index = qrand() % possibleCharacters.length();
        QChar nextChar = possibleCharacters.at(index);
        if(i==4||i==8||i==12||i==16||i==20||i==24){
            randomString.append(":");
        }
        randomString.append(nextChar);
    }
   return randomString.trimmed().simplified().remove(" ");
}

Error::~Error()
{
    delete ui;
}

void Error::on_ok_clicked()
{
    this->close();
}
