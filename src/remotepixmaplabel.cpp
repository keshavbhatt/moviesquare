#include "remotepixmaplabel.h"

#include <QPixmap>
#include <QUrl>
#include <QDebug>
#include <QNetworkDiskCache>


void RemotePixmapLabel::setRemotePixmap(const QString& url , QNetworkDiskCache *diskCache)
{
    if (!networkManager_) {
        /* Lazy initialization of QNetworkAccessManager;
         * only 1 instance is required;
         */
        if(diskCache->cacheDirectory().isEmpty()){
            QString net_cache_path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
            diskCache->setCacheDirectory(net_cache_path);
        }
        networkManager_ = new QNetworkAccessManager(this);
        networkManager_->setCache(diskCache);
        connect(networkManager_, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(pixmapReceived(QNetworkReply*)));
    }
    /* Send GET request to obtain the desired picture */
    networkManager_->get(QNetworkRequest(QUrl(url)));
}

void RemotePixmapLabel::pixmapReceived(QNetworkReply* reply)
{
    if (QNetworkReply::NoError != reply->error()) {
        QString posterUrl = "qrc:///icons/others/default-cover.jpg";
//        qDebug() << Q_FUNC_INFO << "pixmap receiving error" << reply->error();
        reply->deleteLater();
        networkManager_->get(QNetworkRequest(QUrl(posterUrl)));
        return;
    }

    const QByteArray data(reply->readAll());
    if (!data.size())
        qDebug() << Q_FUNC_INFO << "received pixmap looks like nothing";

    QPixmap pixmap;
    pixmap.loadFromData(data);
    setPixmap(pixmap.scaled(this->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
    if(pixmap.isNull()==false){
        emit pixmapLoaded(data);
    }
    reply->deleteLater();
    networkManager_->deleteLater();
    networkManager_ = nullptr;
}
