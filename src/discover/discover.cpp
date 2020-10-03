#include "discover.h"
#include "ui_discover.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <QStandardItem>
#include <QGraphicsOpacityEffect>
#include <QUrlQuery>

Discover::Discover(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Discover)
{
    ui->setupUi(this);

    ui->promo->setText("This app is part of <a href='snap://orion-desktop'>Orion Torrent Streamer</a>");

    Item *track_widget = new Item(0);
    track_widget->resize(track_widget->minimumSizeHint());
    ui->results->setMinimumHeight(track_widget->height()+6);
    track_widget->deleteLater();

    ui->yearCheckbox->toggle();
    ui->yearCheckbox->toggle();

    _loader = new WaitingSpinnerWidget(this,true,true);
    _loader->setRoundness(70.0);
    _loader->setMinimumTrailOpacity(15.0);
    _loader->setTrailFadePercentage(70.0);
    _loader->setNumberOfLines(10);
    _loader->setLineLength(8);
    _loader->setLineWidth(2);
    _loader->setInnerRadius(2);
    _loader->setRevolutionsPerSecond(3);
    _loader->setColor(QColor("#1e90ff"));

    key = "707885705415ccc10153f65383a09c2a";

    m_lpGenresModel				= new QStandardItemModel(0, 1);
    QStringList	headerLabels	= QStringList() << tr("Name");
    m_lpGenresModel->setHorizontalHeaderLabels(headerLabels);
    ui->genrePresets->setModel(m_lpGenresModel);
    ui->genrePresets->setItemDelegate(new cCheckBoxItemDelegate());
    ui->searchQuery->addAction(QIcon(":/icons/youtube-line.png"),QLineEdit::LeadingPosition);
    ui->yearSpinBox->setValue(QDate::currentDate().year());

    init_rangeSlider();
    getGenre();
    loadRegions();
    loadLangs();
    loadUrl(mpm+key);
}

void Discover::resizeEvent(QResizeEvent *event)
{
    MovieDetail *m_details = this->findChild<MovieDetail*>();
    if(m_details){
        m_details->setGeometry(this->rect());
    }
    QWidget::resizeEvent(event);
}

void Discover::init_rangeSlider()
{
    rangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, nullptr);
    connect(rangeSlider, &RangeSlider::lowerValueChanged,[=](int lower){
        ui->m_lpVotingFrom->setText(QString::number((qreal)lower/10));
    });
    connect(rangeSlider, &RangeSlider::upperValueChanged,[=](int upper){
        ui->m_lpVotingTo->setText(QString::number((qreal)upper/10));
    });
    rangeSlider->SetRange(0,100);
    rangeSlider->setLowerValue(0);
    rangeSlider->setUpperValue(100);
    ui->rangeSliderLayout->addWidget(rangeSlider);
    rangeSlider->setStyleSheet("border:none;");
}

// START genre====================================================================
void Discover::getGenre()
{
    QString url = "https://api.themoviedb.org/3/genre/movie/list?api_key="+key;
    Request *_request = new Request(this);
    connect(_request,&Request::requestStarted,[=]()
    {
        _loader->start();
    });
    connect(_request,&Request::requestFinished,[=](QString reply)
    {
        //setStatus("Request finished, Loading results...");
        //load to view
        loadGenre(reply);
        _loader->stop();
        setCursor(Qt::ArrowCursor);
    });
    connect(_request,&Request::downloadError,[=](QString errorString)
    {
        //load genre from local backup
        QFileInfo genreFile(utils::returnPath("discover")+"genre.json");
        if(genreFile.isFile()&&genreFile.exists()){
            loadGenre(utils::loadJson(genreFile.filePath()).toJson());
            qDebug()<<"genre loaded from local backup";
        }else{
            setStatus("An error occured while processing your request.");
            _loader->stop();
            showError(errorString);
        }
    });
    setCursor(Qt::WaitCursor);
    _request->get(url);
}

void Discover::loadGenre(QString reply)
{
    QJsonDocument	jsonResponse	= QJsonDocument::fromJson(reply.toUtf8());
    QJsonObject		jsonObj			= jsonResponse.object();
    QJsonArray		jsonArray		= jsonObj["genres"].toArray();

    for(int z = 0;z < jsonArray.count();z++)
    {
        QJsonObject	genre	= jsonArray[z].toObject();
        qint32  id          = genre["id"].toInt();
        QString name        = genre["name"].toString();

        QStandardItem*	lpItem	= new QStandardItem(name);
        lpItem->setData(id);                
        lpItem->setCheckable(true);
        lpItem->setToolTip(lpItem->text());
        m_lpGenresModel->appendRow(lpItem);
    }
    //save genre data to local file
    utils::saveJson(jsonResponse,utils::returnPath("discover")+"genre.json");
    //setStatus("Loaded genres, Ready.");
}
// END genre====================================================================

void Discover::setStatus(QString message)
{
    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
    ui->status->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
    a->setDuration(500);
    a->setStartValue(0);
    a->setEndValue(1);
    a->setEasingCurve(QEasingCurve::InCurve);
    a->start(QPropertyAnimation::DeleteWhenStopped);
    ui->status->setText(message);
    ui->status->show();
}

void Discover::showError(QString message)
{
    setCursor(Qt::ArrowCursor);
    //prevent error reporting if webtorrent plugin is disabled
    if(settings.value("discover_plugin",false).toBool()==false)
        return;
    //init error
    _error = new Error(this);
    _error->setAttribute(Qt::WA_DeleteOnClose);
    _error->setWindowTitle(QApplication::applicationName()+" | Error dialog");
    _error->setWindowFlags(Qt::Dialog);
    _error->setWindowModality(Qt::NonModal);
    _error->setError("An Error ocurred while processing your request!",
                     message);
    _error->show();
}

Discover::~Discover()
{
    delete ui;
}

void Discover::on_discover_clicked()
{
    ui->results->clear();
    backStack.clear();
    forwardStack.clear();

    QString			szText	= "";
    bool			bAdult	= ui->adult->isChecked();
    qint32			iYear	= ui->yearSpinBox->value();
    qreal			voteMin	= (qreal)rangeSlider->GetLowerValue()/10;
    qreal			voteMax	= (qreal)rangeSlider->GetUpperValue()/10;

    int region = ui->regionComboBox->currentIndex();
    QStandardItem*	lpItemRegion	=  m_lpRegionModel->item(region);
    QString         szRegion        =  lpItemRegion->data().toString().toUpper();

    int lang = ui->langComboBox->currentIndex();
    QStandardItem*	lpItemLang	=  m_lpLangModel->item(lang);
    QString         szLang      =  lpItemLang->data().toString();

    QList<qint32>	genres;
    if(!ui->yearCheckbox->isChecked())
        iYear		= -1;

    for(int x = 0;x < m_lpGenresModel->rowCount();x++)
    {
        QStandardItem*	lpItem	= m_lpGenresModel->item(x);
        if(lpItem->checkState() == Qt::Checked)
            genres.append(lpItem->data().toInt());
    }
    discoverMovie(szText, bAdult, iYear, genres, voteMin, voteMax, szLang,szRegion);
}


// START movie ==================================================================
void Discover::discoverMovie(const QString &szText, const bool &bAdult, const qint32 &iYear, const QList<qint32> &genres, const qreal &voteMin, const qreal &voteMax, const QString &szLanguage, const QString &szRegion)
{
    QString	 szRequest	= QString("https://api.themoviedb.org/3/discover/movie?api_key=%1").arg(key);

    qint32					page		= 1;

    if(!szRegion.contains("xx",Qt::CaseInsensitive))
        szRequest.append(QString("&region=%1").arg(szRegion));

    if(!szLanguage.contains("en-EN"))
        szRequest.append(QString("&language=%1").arg(szLanguage));

    if(!szText.isEmpty())
        szRequest.append(QString("&with_keywords=%1").arg(szText));
    szRequest.append(QString("&include_adult=%1").arg(bAdult?"true":"false"));
    szRequest.append(QString("&include_video=false"));
    if(iYear != -1)
        szRequest.append(QString("&year=%1").arg(iYear));
    if(genres.count())
    {
        QString	tmp	= QString::number(genres[0]);
        for(int x = 1;x < genres.count();x++)
            tmp.append(QString(",%1").arg(genres[x]));
        szRequest.append(QString("&with_genres=%1").arg(tmp));
    }
    szRequest.append(QString("&vote_average.gte=%1").arg(voteMin));
    szRequest.append(QString("&vote_average.lte=%1").arg(voteMax));

    navigated(currentReqUrl);
    Request *_request = new Request(this);
    connect(_request,&Request::requestStarted,[=]()
    {
        _loader->start();
    });
    connect(_request,&Request::requestFinished,[=](QString reply)
    {
        setStatus("Request finished, Loading results...");
        //load to view
        loadMovies(reply);
        _loader->stop();
    });
    connect(_request,&Request::downloadError,[=](QString errorString)
    {
        setStatus("An error occured while processing your request.");
        _loader->stop();
        showError(errorString);
    });
    setCursor(Qt::WaitCursor);
    currentReqUrl = QString("%1&page=%2").arg(szRequest).arg(page);
    _request->get(currentReqUrl);

}

void Discover::loadMovies(QString reply)
{
    QJsonDocument	jsonResponse	= QJsonDocument::fromJson(reply.toUtf8());
    QJsonObject		jsonObj			= jsonResponse.object();
    QJsonArray		jsonArray		= jsonObj["results"].toArray();

    if(jsonArray.isEmpty()){
        setStatus("Request finished. No results found <a href='forceReload://'>Force reload</a>");
    }
    bool hasMorePages = false;
    double currentPage = jsonObj.value("page").toDouble();
    double totalPages = jsonObj.value("total_pages").toDouble();
    double nextPage = 1;
    if(totalPages>currentPage){
        hasMorePages = true;
        nextPage = currentPage+1;
    }
    for(int z = 0;z < jsonArray.count();z++)
    {
        Item *track_widget = new Item(ui->results,false);
        track_widget->setContentsMargins(0,0,0,0);
        track_widget->setStyleSheet("QWidget#"+track_widget->objectName()+"{background-color: transparent;}");
        connect(track_widget,&Item::searchWithOrion,[=](QString term){
           emit search(term);
        });
        connect(track_widget,&Item::loadRecommended,[=](double movieId){
           loadRecommended(movieId);
        });

        connect(track_widget,&Item::showMovieInfo,[=](double movieId){
           showMovieInfo(movieId,track_widget->currentTheme);
        });
        QJsonObject obj = jsonArray.at(z).toObject();
        QString title =  obj.value("title").toString();
        QString releaseDate = obj.value("release_date").toString();
        double rating =obj.value("vote_average").toDouble();
        double movieId =obj.value("id").toDouble();
        QString posterUrl = obj.value("poster_path").toString();
        QString summery = obj.value("overview").toString();
        bool is_video = obj.value("video").toBool();
        if(is_video==false){
            // set data
            track_widget->setTitle(title,summery);
            track_widget->setReleaseDate(releaseDate);
            track_widget->setPoster("https://image.tmdb.org/t/p/w200"+posterUrl);
            track_widget->setRating(rating);
            track_widget->setMovieId(movieId);

            //add widget
            QListWidgetItem* item;
            item = new QListWidgetItem(ui->results);
            item->setSizeHint(track_widget->minimumSizeHint());
            ui->results->setItemWidget(item,track_widget);
            if(z==0){
                QString style = "QListWidget#results{"
                                "border-image: url(:/icons/others/trending-bg.png);"
                                "}";
                ui->results->setStyleSheet(style);
            }
            ui->results->addItem(item);
            if(z==0){
                ui->results->scrollToItem(item);
                ui->results->setCurrentItem(item);
            }
        }
    }
    if(hasMorePages){
        QString statusSubString = "<a href='data://"+QString::number(nextPage)+"'> Load more results from page "+QString::number(nextPage)+" of total "+QString::number(totalPages)+"</a>";
        setStatus("Loaded page "+QString::number(currentPage)+","+statusSubString);
    }else{
        setStatus("Request finished, Loaded results.");
    }
    setCursor(Qt::ArrowCursor);
}
// END movie ==================================================================


void Discover::on_yearCheckbox_toggled(bool checked)
{
    ui->yearSpinBox->setEnabled(checked);
}

void Discover::loadUrl(QString url)
{
    Request *_request = new Request(this);
    connect(_request,&Request::requestStarted,[=]()
    {
        _loader->start();
    });
    connect(_request,&Request::requestFinished,[=](QString reply)
    {
        ui->results->clear();
        ui->back->setEnabled(!backStack.isEmpty());
        ui->forward->setEnabled(!forwardStack.isEmpty());
        setStatus("Request finished, Loading results...");
        //load to view
        loadMovies(reply);
        _loader->stop();
    });
    connect(_request,&Request::downloadError,[=](QString errorString)
    {
        setStatus("An error occured while processing your request.");
        _loader->stop();
        showError(errorString);
    });
    setCursor(Qt::WaitCursor);
    currentReqUrl = url;
    _request->get(currentReqUrl);

}

void Discover::on_status_linkActivated(const QString &link)
{
    if(link.contains("data://")){
        QString page = link.split("data://").last().trimmed();
        navigated(currentReqUrl);
        Request *_request = new Request(this);
        connect(_request,&Request::requestStarted,[=]()
        {
            _loader->start();
        });
        connect(_request,&Request::requestFinished,[=](QString reply)
        {
            setStatus("Request finished, Loading results...");
            //load to view
            loadMovies(reply);
            _loader->stop();
        });
        connect(_request,&Request::downloadError,[=](QString errorString)
        {
            setStatus("An error occured while processing your request.");
            _loader->stop();
            showError(errorString);
        });
        setCursor(Qt::BusyCursor);

        QUrlQuery q(currentReqUrl);
        if(q.hasQueryItem("page")){
            currentReqUrl = currentReqUrl.replace(QRegExp("page=\\d*"),"page="+page);
                    //QString("%1&page=%2").arg(currentReqUrl).arg(page);
        }else {
            currentReqUrl = QString("%1&page=%2").arg(currentReqUrl).arg(page);
        }
        _request->get(currentReqUrl);

    }

    if(link.contains("forceReload://"))
    {
        Request *req = new Request(this);
        req->clearCache(currentReqUrl);
        req->deleteLater();
        loadUrl(currentReqUrl);
    }
}

void Discover::loadRecommended(double movieId)
{
    backStack.clear();
    forwardStack.clear();
    navigated(currentReqUrl);

    int lang = ui->langComboBox->currentIndex();
    QStandardItem*	lpItemLang	=  m_lpLangModel->item(lang);
    QString         szLang      =  lpItemLang->data().toString();

    currentReqUrl = "https://api.themoviedb.org/3/movie/"+QString::number(movieId)+"/recommendations?api_key="+key+"&language="+szLang+"&page=1";
    QString page;
    if(currentReqUrl.isEmpty()){
        page = "1";
    }else{
        QUrlQuery q(currentReqUrl);
        page = q.queryItemValue("page");
    }

    Request *_request = new Request(this);
    connect(_request,&Request::requestStarted,[=]()
    {
        _loader->start();
    });
    connect(_request,&Request::requestFinished,[=](QString reply)
    {
        ui->results->clear();
        setStatus("Request finished, loaded results.");
        //load to view
        loadMovies(reply);
        _loader->stop();
    });
    connect(_request,&Request::downloadError,[=](QString errorString)
    {
        setStatus("An error occured while processing your request.");
        _loader->stop();
        showError(errorString);
    });
    setCursor(Qt::WaitCursor);
    currentReqUrl = QString("%1&page=%2").arg(currentReqUrl).arg(page);
    _request->get(currentReqUrl);

}

void Discover::showMovieInfo(double movieId,QColor itemTheme)
{
    int lang = ui->langComboBox->currentIndex();
    QStandardItem*	lpItemLang	=  m_lpLangModel->item(lang);
    QString         szLang      =  lpItemLang->data().toString();

    MovieDetail *m_details = new MovieDetail(this,movieId,key,szLang,m_lpLangModel);
    eff =  new QGraphicsOpacityEffect(m_details);

    m_details->setAttribute(Qt::WA_DeleteOnClose);
    m_details->setWindowFlags(Qt::Widget);
    m_details->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    m_details->setWidgetTheme(itemTheme);
    m_details->setGeometry(this->rect());

    //load movies by search term
    connect(m_details,&MovieDetail::search,[=](QString term){
        emit search(term);
    });
    //load movies by artist by papularity
    connect(m_details,&MovieDetail::showMoviesBy,[=](double artistId)
    {
        int lang = ui->langComboBox->currentIndex();
        QStandardItem*	lpItemLang	=  m_lpLangModel->item(lang);
        QString         szLang      =  lpItemLang->data().toString();
        QString urlStr = QString(base+"/discover/movie?api_key="+key+"&language="+szLang+"&sort_by=popularity.desc&with_people="+QString::number(artistId)+"&include_adult=%1").arg(ui->adult->isChecked()?"true":"false");
        navigated(currentReqUrl);//save history
        loadUrl(urlStr);
    });

    if(eff!=nullptr){
        m_details->setGraphicsEffect(eff);
        QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
        a->setDuration(250);
        a->setStartValue(0);
        a->setEndValue(1);
        a->setEasingCurve(QEasingCurve::Linear);
        connect(a,&QPropertyAnimation::finished,[=](){
           eff->deleteLater();
        });
        a->start(QPropertyAnimation::DeleteWhenStopped);
        m_details->show();
    }
}

void Discover::navigated(QString url)
{
    if(url.isEmpty()==false && !backStack.contains(url,Qt::CaseInsensitive)){
        qDebug()<<"ADDED TO BACK STACK:"<<url;
        backStack.removeAll(url);
        backStack.append(url);
    }
    ui->back->setEnabled(!backStack.isEmpty());
    ui->forward->setEnabled(!forwardStack.isEmpty());
}

void Discover::on_back_clicked()
{
    if(currentReqUrl.isEmpty()==false && !forwardStack.contains(currentReqUrl,Qt::CaseInsensitive)){
        qDebug()<<"ADDED TO FORWARD STACK:"<<currentReqUrl;
        forwardStack.append(currentReqUrl);
    }
    if(backStack.count()==1){
        loadUrl(backStack.takeLast());
    }else if (backStack.count()>1) {
        loadUrl(backStack.takeAt(backStack.count()-2));
    }
    //clear search if result is not search result
    if(currentReqUrl.contains("search/movie")==false){
        ui->searchQuery->clear();
    }
}

void Discover::on_forward_clicked()
{
    if(currentReqUrl.isEmpty()==false && !backStack.contains(currentReqUrl,Qt::CaseInsensitive)){
        qDebug()<<"ADDED TO Back STACK:"<<currentReqUrl;
        backStack.append(currentReqUrl);
    }
    if(forwardStack.count()==1){
        loadUrl(forwardStack.takeLast());
    }else if (forwardStack.count()>1) {
        loadUrl(forwardStack.takeAt(forwardStack.count()-2));
    }
    //clear search if result is not search result
    if(currentReqUrl.contains("search/movie")){
        QUrlQuery q(currentReqUrl);
        ui->searchQuery->setText(q.queryItemValue("query"));
    }
}


void Discover::loadLangs()
{
    m_lpLangModel	= new QStandardItemModel(0, 1);
    ui->langComboBox->setModel(m_lpLangModel);

    QFile file("://resources/lang.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&file);
    QString reply = in.readAll();
    file.close();

    QJsonDocument	jsonResponse	= QJsonDocument::fromJson(reply.toUtf8());
    QJsonArray		jsonArray		= jsonResponse.array();

    for(int z = 0;z < jsonArray.count();z++)
    {
        QJsonObject	lang	 = jsonArray[z].toObject();
        QString   id         = lang["iso_639_1"].toString();
        QString name         = lang["name"].toString();
        QString english_name = lang["english_name"].toString();
        QString finalName    = english_name + QString(name.isEmpty()?"":" ("+name+")");
        if(name==english_name) finalName = english_name;

        QStandardItem*	lpItem	= new QStandardItem(finalName);
        lpItem->setData(id);
        lpItem->setIcon(QIcon(":/icons/translate-2.png"));
        m_lpLangModel->appendRow(lpItem);
    }
}

void Discover::loadRegions()
{
    m_lpRegionModel	= new QStandardItemModel(0, 1);
    ui->regionComboBox->setModel(m_lpRegionModel);

    QFile file("://resources/regions");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList itemdata = QString(line).split(":");
        QStandardItem*	lpItem	= new QStandardItem(itemdata.first());
        lpItem->setIcon(QIcon(":/icons/flags/"+itemdata.last()+".png"));
        lpItem->setData(itemdata.last());
        m_lpRegionModel->appendRow(lpItem);
    }
    file.close();
}

//START SEARCH MOVIE BY TITLE==========================================
void Discover::on_searchQuery_textChanged(const QString &arg1)
{
    ui->saerchMovie->setEnabled(arg1.trimmed().isEmpty()==false);
}

void Discover::on_saerchMovie_clicked()
{
    QString term = ui->searchQuery->text().trimmed();

    int lang = ui->langComboBox->currentIndex();
    QStandardItem*	lpItemLang	=  m_lpLangModel->item(lang);
    QString         szLang      =  lpItemLang->data().toString();

    QString urlStr = QString(base+"/search/movie?api_key="+key+"&language="+szLang+"&query="+term+"&include_adult=%1").arg(ui->adult->isChecked()?"true":"false");
    QUrl url(urlStr);
    navigated(currentReqUrl);//save history
    loadUrl(url.toEncoded(QUrl::EncodeSpaces));
}

void Discover::on_searchQuery_returnPressed()
{
    if(ui->saerchMovie->isEnabled())
        on_saerchMovie_clicked();
}
//END SEARCH MOVIE BY TITLE==========================================


void Discover::on_reset_clicked()
{
    ui->yearCheckbox->setChecked(false);
    ui->adult->setChecked(false);
    ui->regionComboBox->setCurrentIndex(0);
    ui->langComboBox->setCurrentIndex(0);

    rangeSlider->setLowerValue(0);
    rangeSlider->setUpperValue(100);

    for(int x = 0;x < m_lpGenresModel->rowCount();x++)
    {
        QStandardItem*	lpItem	= m_lpGenresModel->item(x);
        lpItem->setCheckState(Qt::Unchecked);
    }

    ui->searchQuery->clear();

    on_discover_clicked();
}

void Discover::on_promo_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(QUrl(link));
}
