#include "item.h"
#include "ui_item.h"
#include <QPixmap>

Item::Item(QWidget *parent, bool isPeople) :
    QWidget(parent),
    ui(new Ui::Item)
{
    ui->setupUi(this);

    circularProgressBar = new CircularProgessBar(/*ui->poster*/);
    circularProgressBar->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Maximum);
    circularProgressBar->setArcPenBrush(QBrush("#336B95"));
    circularProgressBar->setCircleBrush(QBrush("#31363B"),QBrush("#31363B"));
    QFont f;
    f.setFamily(f.defaultFamily());
    f.setPixelSize(11);
    f.setBold(true);
    circularProgressBar->setTextProperty(QColor("#C0C0C0"),f);
    circularProgressBar->setMinimumSize(QSize(56,56));

    ui->ratingLayout->addWidget(circularProgressBar);

    circularProgressBar->setText("-");
    circularProgressBar->setProgressValue(0.0);
    setWidgetTheme(QColor("#166280"));

    connect(ui->poster,&RemotePixmapLabel::pixmapLoaded,[=](QByteArray data){
        currentPixmap.loadFromData(data);
        setWidgetTheme(color());
    });
    double ratio  = 120.0/187.0;
    double height = 225.0;
    double width  = ratio * height;
    ui->poster->setFixedSize(width,height);
    init_MovieActionWidget(isPeople);
}

void Item::resizeEvent(QResizeEvent *event)
{
    QWidget *actionWidget = findChild<QWidget*>("actions_view");
    if(isPeople){
       actionWidget = findChild<QWidget*>("actions_view_people");
    }
    actionWidget->setGeometry(this->rect());
    QWidget::resizeEvent(event);
}

void Item::adjustForPeoples()
{
    //sets this widget for loading people(movie crew etc)
    isPeople = true;
    double ratio  = 120.0/187.0;
    double height = 170.0;
    double width  = ratio * height;
    ui->poster->setFixedSize(width,height);
    circularProgressBar->hide();
    ui->releaseDate->setWordWrap(true);
}

void Item::init_MovieActionWidget(bool isPeople)
{
    // _ui_action is the child of wall_view
    QWidget *actionWidget = new QWidget(this);
    _ui_action.setupUi(actionWidget);
    if(isPeople){
           actionWidget->setObjectName("actions_view_people");
           _ui_action.search->hide();
           _ui_action.moreInfo->hide();
           _ui_action.recommended->hide();
           connect(_ui_action.show_movies,&QToolButton::clicked,[=](){
               actionWidget->hide();
               emit showMoviesBy(movieId); // movie id is set as artistId in case of poeple
           });
    }else{
         actionWidget->setObjectName("actions_view");
        _ui_action.show_movies->hide();
        connect(_ui_action.search,&QToolButton::clicked,[=](){
            actionWidget->hide();
            emit searchWithOrion(ui->title->text()+" "+ui->releaseDate->text().split("-").first());
        });
        connect(_ui_action.moreInfo,&QToolButton::clicked,[=](){
            actionWidget->hide();
            emit showMovieInfo(movieId);
        });
        connect(_ui_action.recommended,&QToolButton::clicked,[=](){
            actionWidget->hide();
            emit loadRecommended(movieId);
        });
    }
    actionWidget->setStyleSheet("QWidget#"+actionWidget->objectName()+"{background-color: transparent;}");
    actionWidget->setGeometry(this->rect());
    actionWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    actionWidget->hide();
}


Item::~Item()
{
    delete ui;
}

void Item::setTitle(QString str,QString summary)
{
    ui->title->setText(str);
    setToolTip("<b>"+str+"</b>"+QString(summary.isEmpty()?"":"<br><br>"+summary));
    ui->title->pause();
}

void Item::setReleaseDate(QString str)
{
    ui->releaseDate->setText(str);
}

void Item::setPoster(QString url)
{
    QString net_cache_path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QNetworkDiskCache *diskCache = new QNetworkDiskCache(this);
    diskCache->setCacheDirectory(net_cache_path);
    ui->poster->setRemotePixmap(url,diskCache);
}

void Item::setRating(double vote_average)
{
    vote_average = vote_average * 100.0;
    qreal tmpValue = vote_average/10.0;
    QString text = QString::number(tmpValue/10.0,'f',1);
    circularProgressBar->setProgressValue(tmpValue);
    circularProgressBar->setText(text);
}

void Item::setMovieId(double id)
{
    // we set movie id and artist id using this
    // so that we can use this class for both movie
    // and people
    movieId = id;
}

void Item::enterEvent(QEvent * event)
{    
    //prevent crashes on listWidget clear
    if(!parentWidget()->isEnabled())
        return;
    QWidget *actionWidget = findChild<QWidget*>("actions_view");
    if(isPeople){
       actionWidget = findChild<QWidget*>("actions_view_people");
    }
    if(actionWidget == nullptr)
        return;
    ui->title->resume();
    actionWidget->setGeometry(this->rect());
    if(eff!=nullptr){
        actionWidget->setGraphicsEffect(eff);
        QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
        a->setDuration(250);
        a->setStartValue(0);
        a->setEndValue(1);
        a->setEasingCurve(QEasingCurve::Linear);
        a->start(QPropertyAnimation::DeleteWhenStopped);
        actionWidget->show();
    }
    QWidget::enterEvent(event);
}

void Item::leaveEvent(QEvent * event)
{   
    //prevent crashes on listWidget clear
    if(!parentWidget()->isEnabled())
        return;
    QWidget *actionWidget =  findChild<QWidget*>("actions_view");
    if(isPeople){
       actionWidget = findChild<QWidget*>("actions_view_people");
    }
    if( actionWidget == nullptr)
        return;
    if(eff!=nullptr){
        actionWidget->setGraphicsEffect(eff);
        QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
        a->setDuration(250);
        a->setStartValue(1);
        a->setEndValue(0);
        a->setEasingCurve(QEasingCurve::Linear);
        connect(a,&QPropertyAnimation::finished,[=](){
            if(actionWidget!=nullptr)
            actionWidget->hide();
        });
        a->start(QPropertyAnimation::DeleteWhenStopped);
    }
    QWidget::leaveEvent(event);
}

//====DOMINANT COLOR
QPixmap Item::scale(const QPixmap &pixmap, int size, ScalePolicy policy)
{
    const Qt::TransformationMode mode =
        policy == ScalePolicy::Smooth
            ? Qt::SmoothTransformation
            : Qt::FastTransformation;

    return pixmap.scaled(size, size, Qt::KeepAspectRatio, mode);
}

QPixmap Item::pixmap(int size) const
{
    const QPixmap *pixmap = ui->poster->pixmap();
    return size == 0 ? *pixmap : scale(*pixmap, size);
}

QColor Item::color() const
{
    const QColor raw = computeColor(scale(pixmap(), 30, ScalePolicy::Fast).toImage());
    return raw;
}

QColor Item::computeColor(const QImage &image)
{
    static constexpr int range = 60;
    static constexpr int limit = 25;

    QVector<HsvRange> colorful(range);
    QVector<HsvRange> grey(range);

    bool isColorful = false;
    QRgb *pixels = (QRgb *)image.bits();
    for (int i = 0; i < image.height() * image.width(); ++i)
    {
        const QColor rgb(pixels[i]);
        const int r = rgb.red();
        const int g = rgb.green();
        const int b = rgb.blue();

        const QColor hsv = rgb.toHsv();
        const int h = qMax(0, hsv.hue());
        const int s = hsv.saturation();
        const int v = hsv.value();
        const int x = h / (360 / range);

        if (qAbs(r - g) > limit || qAbs(g - b) > limit || qAbs(b - r) > limit)
        {
            isColorful = true;
            colorful[x].h += h;
            colorful[x].s += s;
            colorful[x].v += v;
            colorful[x].c++;
        }
        else if (!isColorful)
        {
            grey[x].h += h;
            grey[x].s += s;
            grey[x].v += v;
            grey[x].c++;
        }
    }
    return dominantColor(isColorful ? colorful : grey);
}

QColor Item::dominantColor(const QVector<HsvRange> &ranges)
{
    int max = 0;
    HsvRange dominant = ranges.first();
    for (const HsvRange &range : ranges)
    {
        const int temp = range.h + 2 * range.s + 3 * range.v;
        if (temp > max)
        {
            dominant = range;
            max = temp;
        }
    }
    const float c = static_cast<float>(dominant.c);
    const int h = qRound(static_cast<float>(dominant.h) / c);
    const int s = qRound(static_cast<float>(dominant.s) / c);
    const int v = qRound(static_cast<float>(dominant.v) / c);

    return QColor::fromHsv(h, s, v);
}

QColor Item::adjustColor(const QColor &color)
{
    const qreal h = color.hsvHueF();
    const qreal s = qMin(color.hsvSaturationF(), 0.8);
    const qreal v = qMin(color.valueF(), 0.36);

    return QColor::fromHsvF(h, s, v);
}

void Item::setWidgetTheme(QColor rgb)
{
    currentTheme = rgb;
    QString r = QString::number(rgb.red());
    QString g = QString::number(rgb.green());
    QString b = QString::number(rgb.blue());

    circularProgressBar->setArcPenBrush(QBrush(rgb.lighter(180)));

    QString widgetStyle= "background-color:"
                         "qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,"
                         "stop:0.129213 rgba("+r+", "+g+", "+b+", 40),"
                         "stop:0.38764 rgba("+r+", "+g+", "+b+", 136),"
                         "stop:0.679775 rgba("+r+", "+g+", "+b+", 94),"
                         "stop:1 rgba("+r+", "+g+", "+b+", 30));";

    ui->poster->setStyleSheet("QLabel{border-radius: 10px;background-color:rgba("+r+", "+g+", "+b+", 0)}");

    QColor rgbToolTip = rgb.darker(210);

    QString tr = QString::number(rgbToolTip.red());
    QString tg = QString::number(rgbToolTip.green());
    QString tb = QString::number(rgbToolTip.blue());

    QString toolTipStyle ="QToolTip{background-color:"
                          "qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,"
                          "stop:0.129213 rgba("+tr+", "+tg+", "+tb+", 40),"
                          "stop:0.38764 rgba("+tr+", "+tg+", "+tb+", 136),"
                          "stop:0.679775 rgba("+tr+", "+tg+", "+tb+", 94),"
                          "stop:1 rgba("+tr+", "+tg+", "+tb+", 30));}";

    this->setStyleSheet(widgetStyle+toolTipStyle);
}
