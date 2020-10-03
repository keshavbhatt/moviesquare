#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QSettings>
#include <QMessageBox>
#include <QStyleFactory>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QScreen>


#include "discover/discover.h"
#include "error.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected slots:
    void closeEvent(QCloseEvent *event);
private slots:
    void init_Discover();
    void setStyle(QString fname);
private:
    Ui::MainWindow *ui;
    Discover *discover = nullptr;
    Error *_error = nullptr;
    QSettings settings;

};

#endif // MAINWINDOW_H
