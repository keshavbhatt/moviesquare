#ifndef ITEM_H
#define ITEM_H

#include <QWidget>
#include <QGraphicsOpacityEffect>
#include "widgets/circularprogessbar.h"
#include "remotepixmaplabel.h"
#include "ui_title_action.h"

namespace Ui {
class Item;
}

class Item : public QWidget
{
    Q_OBJECT
signals:
    void searchWithOrion(QString term);
    void loadRecommended(double movieId);
    void showMovieInfo(double movieId);
    void showMoviesBy(double artistId);
public:
    explicit Item(QWidget *parent = nullptr,bool isPeople= false);
    QColor currentTheme;
    ~Item();
public slots:
    void setTitle(QString str, QString summary);
    void setReleaseDate(QString str);
    void setPoster(QString url);
    void setRating(double vote_average = 0);
    void setMovieId(double id);
    void adjustForPeoples();
protected slots:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void resizeEvent(QResizeEvent *event);
private slots:
    void setWidgetTheme(QColor rgb);
    void init_MovieActionWidget(bool isPeople);
private:
    Ui::Item *ui;
    Ui::actionWidgetN _ui_action;
    CircularProgessBar *circularProgressBar = nullptr;
    double movieId;
    bool isPeople = false;
    QPixmap currentPixmap;
    QGraphicsOpacityEffect *eff =  new QGraphicsOpacityEffect;


//=======DOMINANT COLOR
private:
    struct HsvRange
    {
        int h;
        int s;
        int v;
        int c;
    };
    static QColor computeColor(const QImage &image);
    static QColor dominantColor(const QVector<HsvRange> &ranges);
    static QColor adjustColor(const QColor &color);
    QPixmap pixmap(int size = 0) const;
    QColor color() const;
    enum ScalePolicy {Fast, Smooth};
    static QPixmap scale(const QPixmap &pixmap, int size, ScalePolicy policy = ScalePolicy::Smooth);
};

#endif // ITEM_H
