#ifndef DISCOVER_H
#define DISCOVER_H

#include <QWidget>
#include <QSettings>
#include <QStandardItemModel>
#include <QDebug>
#include <QDesktopServices>
#include <QGraphicsOpacityEffect>

#include "widgets/RangeSlider.h"
#include "request.h"
#include "widgets/waitingspinnerwidget.h"
#include "error.h"
#include "widgets/ccheckboxitemdelegate.h"
#include "item.h"
#include "utils.h"
#include "moviedetail.h"

namespace Ui {
class Discover;
}

class Discover : public QWidget
{
    Q_OBJECT
signals:
    void search(QString term);

public:
    explicit Discover(QWidget *parent = nullptr);
    ~Discover();

protected slots:
    void resizeEvent(QResizeEvent *event);
private slots:
    void init_rangeSlider();

    void showError(QString message);
    void setStatus(QString message);

    void loadGenre(QString reply);
    void getGenre();

    void on_yearCheckbox_toggled(bool checked);

    void on_discover_clicked();
    void discoverMovie(const QString& szText,
                       const bool& bAdult,
                       const qint32& iYear,
                       const QList<qint32>& genres,
                       const qreal& voteMin,
                       const qreal& voteMax,
                       const QString& szLanguage = "en-EN",
                       const QString& szRegion = "US");
    void loadMovies(QString reply);

    void on_status_linkActivated(const QString &link);

    void loadUrl(QString url);
    void loadRecommended(double movieId);

    void on_back_clicked();
    void navigated(QString url);
    void on_forward_clicked();
    void showMovieInfo(double movieId, QColor itemTheme);
    void loadRegions();
    void on_searchQuery_textChanged(const QString &arg1);

    void on_saerchMovie_clicked();

    void loadLangs();
    void on_searchQuery_returnPressed();

    void on_reset_clicked();

    void on_promo_linkActivated(const QString &link);

private:
    Ui::Discover *ui;
    RangeSlider *rangeSlider;
    QString key;
    WaitingSpinnerWidget *_loader = nullptr;
    Error *_error = nullptr;
    QSettings settings;
    QStandardItemModel*	m_lpGenresModel;
    QStandardItemModel* m_lpRegionModel;
    QStandardItemModel* m_lpLangModel;
    QString currentReqUrl;

    QGraphicsOpacityEffect *eff = nullptr;

    //navigation
    QStringList backStack, forwardStack;

    //presets;
    QString base = "https://api.themoviedb.org/3";
    //most popular movies
    QString mpm = base+"/discover/movie?sort_by=popularity.desc&api_key=";
    //most popular kids movies
    QString mpkm = base+"/discover/movie?certification_country=IN&sort_by=vote_average.desc&certification.lte=G&api_key=";
};

#endif // DISCOVER_H
