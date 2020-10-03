#include "moviedetail.h"
#include "ui_moviedetail.h"


#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDesktopServices>

MovieDetail::MovieDetail(QWidget *parent, double movieId, QString key, QString lang, QStandardItemModel *langModel) :
    QWidget(parent),
    ui(new Ui::MovieDetail)
{
    ui->setupUi(this);
    this->movieId = movieId;
    this->key = key;
    this->lang = lang;
    this->langModel = langModel; // to convert iso_639_1 language code to language names.

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


    circularProgressBar = new CircularProgessBar();
    circularProgressBar->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Maximum);
    circularProgressBar->setArcPenBrush(QBrush("#336B95"));
    circularProgressBar->setCircleBrush(QBrush("#31363B"),QBrush("#31363B"));
    QFont f;
    f.setFamily(f.defaultFamily());
    f.setPixelSize(11);
    f.setBold(true);
    circularProgressBar->setTextProperty(QColor("#C0C0C0"),f);
    circularProgressBar->setMinimumSize(QSize(90,90));
    ui->ratiingLayout->insertWidget(0,circularProgressBar);

    circularProgressBar->setText("-");
    circularProgressBar->setProgressValue(0.0);

    ui->upperWidget->installEventFilter(this);

    loadMovie();
    creditRetry = 3;
}


QString MovieDetail::getLangByCode(QString iso_639_1)
{
    QString langName;
    for (int i = 0; i < langModel->rowCount(); ++i) {
        QStandardItem*	lpItemLang	=  langModel->item(i);
        QString         szLang      =  lpItemLang->data().toString();
        if(szLang==iso_639_1){
            langName = lpItemLang->text();
            break;
        }
    }
    return langName;
}

void MovieDetail::closeEvent(QCloseEvent *event)
{
    QString path = utils::returnPath("discover/backdrop/");
    QDir d(path);
    d.removeRecursively();
    QWidget::closeEvent(event);
}

void MovieDetail::loadMovie()
{
    QString url = QString("https://api.themoviedb.org/3/movie/%1?api_key=%2").arg(movieId).arg(key)+"&language="+lang;
    Request *_request = new Request(this);
    connect(_request,&Request::requestStarted,[=]()
    {
        _loader->start();
    });
//    qDebug()<<"MOVIE DETAILS: "<<url;
    connect(_request,&Request::requestFinished,[=](QString reply)
    {
        loadCredits();  // we are loading creadits after loading moview details
        ui->results->clear();
        //load to view
        loadMovieToView(reply);
        _loader->stop();
        setCursor(Qt::ArrowCursor);
    });
    connect(_request,&Request::downloadError,[=](QString errorString)
    {
        _loader->stop();
        showError(errorString);
    });
    setCursor(Qt::WaitCursor);
    _request->get(url);
}

void MovieDetail::loadCredits()
{
    qDebug()<<"LOADING CREDITS";
    QString url = QString("https://api.themoviedb.org/3/movie/%1/credits?api_key=%2").arg(movieId).arg(key);
    Request *_request = new Request(this);
    connect(_request,&Request::requestStarted,[=]()
    {
        _loader->start();
    });
    connect(_request,&Request::requestFinished,[=](QString reply)
    {
        ui->results->clear();
        //load to view
        loadCastToView(reply);
        _loader->stop();
        setCursor(Qt::ArrowCursor);
    });
    connect(_request,&Request::downloadError,[=](QString errorString)
    {
        _loader->stop();
        if(creditRetry>0){
            _request->clearCache(url);
            loadCredits();
            creditRetry--;
            qDebug()<<"RELODING CREDIT FOR MOVIE"<<movieId<<", ON NETWORK FAIL"<<creditRetry;
            _request->deleteLater();
            return;
        }
        showError(errorString);
    });
    setCursor(Qt::BusyCursor);
    currentCreditUrl = url;
    _request->get(url);
}

void MovieDetail::loadMovieToView(QString reply)
{
    //qDebug()<<"REPLY : "<<reply;

    QJsonDocument	jsonResponse	= QJsonDocument::fromJson(reply.toUtf8());
    QJsonObject		jsonObj			= jsonResponse.object();
    QJsonArray		jsonGenreArray		= jsonObj["genres"].toArray();
    QJsonArray		jsonPCArray		= jsonObj["production_companies"].toArray();
    QJsonArray      jsonProduction_companiesArray = jsonObj["production_companies"].toArray();
    QJsonArray      jsonPConArray  = jsonObj["production_countries"].toArray();

    QString pcStr;
    foreach(QJsonValue pcObj,jsonPCArray){
        pcStr.append(", "+pcObj.toObject().value("name").toString());
    }

    QString pconStr;
    foreach(QJsonValue pconObj,jsonPConArray){
        pconStr.append(", "+pconObj.toObject().value("name").toString());
    }

    QString genreStr;
    foreach(QJsonValue genreObj,jsonGenreArray){
        genreStr.append(", "+genreObj.toObject().value("name").toString());
    }
    QString proCompStr;
    foreach(QJsonValue proComObj,jsonProduction_companiesArray){
        proCompStr.append(proComObj.toObject().value("name").toString());
    }

    QString backdrop_path = "https://image.tmdb.org/t/p/w500"+jsonObj.value("backdrop_path").toString();
    QString title = jsonObj.value("title").toString();
    QString status = jsonObj.value("status").toString();
    QString tagline = jsonObj.value("tagline").toString();
    QString release_date  = jsonObj.value("release_date").toString();
    QString poster_path = "https://image.tmdb.org/t/p/w300" + jsonObj.value("poster_path").toString();
    double vote = jsonObj.value("vote_average").toDouble();
    QString overview = jsonObj.value("overview").toString();
    double runtime = jsonObj.value("runtime").toDouble();
    QString imdb_id = jsonObj.value("imdb_id").toString();
    QTime time = QTime::fromMSecsSinceStartOfDay(runtime*60*1000);
    QString original_language = jsonObj.value("original_language").toString();

    QString langName = getLangByCode(original_language);

    QString meta =
            QString(langName.isEmpty()?"":langName+" | ")+
            QString(release_date.isEmpty()?"":release_date+" | ")+
            QString(genreStr.remove(0,1).isEmpty()?"":genreStr+" | ")+
            QString(time.toString("hh:mm:ss").isEmpty()?"":time.toString("hh:mm:ss"));

    QDate date =QDate::fromString(release_date,Qt::ISODate);


    ui->title->setText(title.toUpper()+" ("+QString::number(date.year())+")");
    if(tagline.isEmpty() || tagline.isNull()){
        ui->tagline->hide();
    }
    ui->tagline->setText(tagline);
    ui->meta->setText(meta);
    ui->summery->setText("<b>Summery:</b> "+overview);
    ui->status->setText("<b>Status:</b> "+status+QString(pcStr.isEmpty()?"":"<br><br><b>"
                                         "Production:</b> "+pcStr.remove(0,1))
                        +QString(pconStr.isEmpty()?"":"<br><br><b>"
                         "Country:</b> "+pconStr.remove(0,1)));

    double ratio  = 120.0/187.0;
    double height = 350.0;
    double width  = ratio * height;
    ui->poster->setFixedSize(width,height);
    QString net_cache_path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QNetworkDiskCache *diskCache = new QNetworkDiskCache(0);
    diskCache->setCacheDirectory(net_cache_path);
    ui->poster->setRemotePixmap(poster_path,diskCache);

    if(imdb_id.isEmpty()==false){
        connect(ui->imdb,&controlButton::clicked,[=](){
           QDesktopServices::openUrl(QUrl("https://www.imdb.com/title/"+imdb_id));
        });
    }else{
        ui->imdb->hide();
    }
    RemotePixmapLabel *bp = new RemotePixmapLabel(this);
    bp->hide();
    connect(bp,&RemotePixmapLabel::pixmapLoaded,[=](QByteArray data)
    {
        QPixmap pixmap;
        pixmap.loadFromData(data);
        if(pixmap.isNull()==false){
            backdropPix = pixmap;
            ui->upperWidget->repaint();
            ui->upperWidget->update();
        }
    });
    QNetworkDiskCache *diskCache2 = new QNetworkDiskCache(0);
    diskCache->setCacheDirectory(net_cache_path);
    bp->setRemotePixmap(backdrop_path,diskCache2);

    double vote_average = vote * 100.0;
    qreal tmpValue = vote_average/10.0;
    QString text = QString::number(tmpValue/10.0,'f',1);
    circularProgressBar->setProgressValue(tmpValue);
    circularProgressBar->setText(text);

    ui->scrollAreaWidgetContents->setMinimumWidth(ui->scrollAreaWidgetContents->minimumSizeHint().width());
}

bool MovieDetail::eventFilter(QObject *obj, QEvent *event)
{
    if(obj== ui->upperWidget && event->type() ==QEvent::Paint && !backdropPix.isNull()){
        QPainter paint(ui->upperWidget);
        int widWidth = this->ui->upperWidget->width();
        int widHeight = this->ui->upperWidget->height();
        QPixmap _backdropPix = backdropPix.scaled(widWidth, widHeight, Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
        QPoint centerOfWidget = ui->upperWidget->rect().center();
        QRect rectOfPixmap = _backdropPix.rect();
        rectOfPixmap.moveCenter(centerOfWidget);
        QRect boundingRect = rectOfPixmap;
        paint.drawPixmap(rectOfPixmap.topLeft(), _backdropPix);
        QColor darker = currentTheme.darker(200);
        int r,g,b;
        r = darker.red();
        g = darker.green();
        b = darker.blue();
        paint.fillRect(QRectF(boundingRect.x()-5,boundingRect.y()-5,boundingRect.width()+10,boundingRect.height()+10), QBrush(QColor(r,g,b,180)));
    }
   return QWidget::eventFilter(obj,event);
}

void MovieDetail::loadCastToView(QString reply)
{
    QJsonDocument	jsonResponse	= QJsonDocument::fromJson(reply.toUtf8());
    QJsonObject		jsonCast		= jsonResponse.object();
    QJsonArray		jsonArrayCast	= jsonCast["cast"].toArray();
    QJsonArray		jsonArrayCrew	= jsonCast["crew"].toArray();
    QJsonObject		tmpObj;

    // check for error and retry
    if(jsonCast.isEmpty()){
        Request *_request = new Request(this);
        if(creditRetry>0){
            _request->clearCache(currentCreditUrl);
            loadCredits();
            creditRetry--;
            qDebug()<<"RELODING CREDIT FOR MOVIE"<<movieId<<creditRetry;
            _request->deleteLater();
            return;
        }
    }

    QString directors,producers,authors,screenplay;
    int limit = jsonArrayCast.count();
    if(limit > 20){
        limit = 20;
    }
    for(int x = 0;x < limit ; x++)
    {
        tmpObj	= jsonArrayCast.at(x).toObject();
        QString posterUrl = tmpObj.value("profile_path").toString();
        if(posterUrl.isEmpty()|| posterUrl.isNull()){
            posterUrl = "qrc:///icons/others/default-cover.png";
        }
        Item *track_widget = new Item(ui->results,true);
        track_widget->adjustForPeoples();
        track_widget->setContentsMargins(0,0,0,0);
        track_widget->setStyleSheet("QWidget#"+track_widget->objectName()+"{background-color: transparent;}");
        QString name = tmpObj.value("name").toString();
        QString character = tmpObj.value("character").toString();
        double id = tmpObj.value("id").toDouble();
        track_widget->setMovieId(id);
        // set data
        track_widget->setTitle(name,character);
        track_widget->setReleaseDate(character);
        track_widget->setPoster("https://image.tmdb.org/t/p/w200"+posterUrl);
        connect(track_widget,&Item::showMoviesBy,[=](double artistId){
            emit showMoviesBy(artistId);
            on_close_clicked();
        });

        //add widget
        QListWidgetItem* item = new QListWidgetItem(ui->results);
        item->setSizeHint(track_widget->minimumSizeHint());
        ui->results->setItemWidget(item,track_widget);
        ui->results->setMinimumHeight(track_widget->height()+6);
        ui->results->addItem(item);
    }

    for(int x = 0;x < jsonArrayCrew.count();x++)
    {
        tmpObj	= jsonArrayCrew.at(x).toObject();
        QString job = tmpObj.value("job").toString();
        if(job == "Director"){
            directors.append("<br>"+tmpObj.value("name").toString());
        }
        if(job == "Producer"){
            producers.append("<br>"+tmpObj.value("name").toString());
        }
        if(job == "Author"){
            authors.append("<br>"+tmpObj.value("name").toString());
        }
        if(job == "Screenplay"){
            screenplay.append("<br>"+tmpObj.value("name").toString());
        }
    }
    if(producers.isEmpty()==false){
        QLabel *producer = new QLabel(this);
        ui->crew->addWidget(producer);
        ui->crew->setAlignment(producer,Qt::AlignTop);
        producer->setStyleSheet("background:transparent");
        producer->setText("<b>Producers:</b><br>"+producers);
    }

    if(directors.isEmpty()==false){
        QLabel *director = new QLabel(this);
        ui->crew->addWidget(director);
        ui->crew->setAlignment(director,Qt::AlignTop);
        director->setStyleSheet("background:transparent");
        director->setText("<b>Directors:</b><br>"+directors);
    }

    if(authors.isEmpty()==false){
        QLabel *author = new QLabel(this);
        ui->crew->addWidget(author);
        ui->crew->setAlignment(author,Qt::AlignTop);
        author->setStyleSheet("background:transparent");
        author->setText("<b>Author:</b><br>"+authors);
    }

    if(screenplay.isEmpty()==false){
        QLabel *screenplayLab = new QLabel(this);
        ui->crew->addWidget(screenplayLab);
        ui->crew->setAlignment(screenplayLab,Qt::AlignTop);
        screenplayLab->setStyleSheet("background:transparent");
        screenplayLab->setText("<b>Screenplay:</b><br>"+screenplay);
    }

    //check for error and retry
    if(ui->results->count() < 1){
        Request *_request = new Request(this);
        if(creditRetry>0){
            _request->clearCache(currentCreditUrl);
            loadCredits();
            creditRetry--;
            qDebug()<<"RELODING CREDIT FOR MOVIE"<<movieId<<", LIST FOUND EMPTY"<<creditRetry;
            _request->deleteLater();
            return;
        }
    }
    QApplication::processEvents();
}

void MovieDetail::showError(QString message)
{
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

MovieDetail::~MovieDetail()
{
    delete _error;
    delete _loader;
    delete ui;
}

void MovieDetail::on_close_clicked()
{
    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
    this->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
    a->setDuration(300);
    a->setStartValue(1);
    a->setEndValue(0);
    a->setEasingCurve(QEasingCurve::Linear);
    connect(a,&QPropertyAnimation::finished,[=](){
        this->close();
    });
    a->start(QPropertyAnimation::DeleteWhenStopped);
}

void MovieDetail::setWidgetTheme(QColor rgb)
{
    currentTheme = rgb;

    QColor lighter = rgb.lighter(160);
    QString r = QString::number(lighter.red());
    QString g = QString::number(lighter.green());
    QString b = QString::number(lighter.blue());

    circularProgressBar->setArcPenBrush(QBrush(lighter));
    ui->results->setStyleSheet("background-color:transparent;"
                               "border:none;"
                               "QListWidget#results{"
                               "border-image: url(:/icons/others/trending-bg.png);"
                               "}");

            this->setStyleSheet(
                        "QScrollBar:horizontal{"
                            "background-color: transparent;"
                            "border-color:none;"
                            "border:none;"
                            "height: 16px;"
                            "margin:0px;"
                        "}"
                        "QScrollBar::handle:horizontal {"
                            "background-color: rgba("+r+", "+g+", "+b+",160);"
                            "min-width: 20px;"
                            "border-radius:2px;"
                            "margin:4px 2px 4px 2px;"
                        "}"
                         "QScrollBar::add-line:horizontal{"
                         "  background:none;"
                         "  width:0px;"
                         "  subcontrol-position:bottom;"
                         "  subcontrol-origin:margin;"
                         "}"
                         "QScrollBar::sub-line:horizontal{"
                         "  background:none;"
                         "  width:0px;"
                         "  subcontrol-position:top;"
                         "  subcontrol-origin:margin;"
                         "}");

    //style lines
    foreach (QFrame *frame, this->findChildren<QFrame*>()) {
        if(frame->objectName().contains("styled_line"))
        frame->setStyleSheet("QFrame#"+frame->objectName()+"{border-color:rgba("+r+", "+g+", "+b+",120)}");
    }
}

void MovieDetail::closeRequested()
{
    on_close_clicked();
}

void MovieDetail::on_search_clicked()
{
    emit search(ui->title->text().remove("(").remove(")"));
}
