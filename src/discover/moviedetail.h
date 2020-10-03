#ifndef MOVIEDETAIL_H
#define MOVIEDETAIL_H

#include <QWidget>
#include <QDebug>
#include <QSettings>
#include <QStandardItemModel>
#include "utils.h"
#include "error.h"
#include "request.h"
#include "widgets/waitingspinnerwidget.h"
#include "widgets/circularprogessbar.h"
#include "discover/item.h"

namespace Ui {
class MovieDetail;
}

class MovieDetail : public QWidget
{
    Q_OBJECT
signals:
    void search(QString term);
    void showMoviesBy(double artistId);

public:
    explicit MovieDetail(QWidget *parent = nullptr,double movieId=0.0,QString key="",QString lang="",QStandardItemModel *langModel = nullptr);
    ~MovieDetail();

public slots:
    void setWidgetTheme(QColor rgb);
    void closeRequested();
protected slots:
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
private slots:
    void loadCredits();
    void loadCastToView(QString reply);
    void showError(QString message);
    void on_close_clicked();

    void loadMovie();
    void loadMovieToView(QString reply);
    void on_search_clicked();

    QString getLangByCode(QString iso_639_1);
private:
    Ui::MovieDetail *ui;
    double movieId;
    QString key;
    WaitingSpinnerWidget *_loader = nullptr;
    CircularProgessBar * circularProgressBar = nullptr;
    Error *_error = nullptr;
    QSettings settings;
    QPixmap backdropPix;
    QColor currentTheme;
    int creditRetry = 0;
    QString currentCreditUrl;
    QString lang;
    QStandardItemModel *langModel;
};

#endif // MOVIEDETAIL_H
