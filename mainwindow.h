#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QUdpSocket>
#include <QTimer>
#include <QSettings>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();  

private slots:
    //button回调函数
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();

    void readingDataGrams();
    void connectsucess();
    void timerUpdate();
    void nettimerUpdate();
    void quirytimeout();

private:
    void updateComboBox();
    #if defined(Q_OS_WIN32)
        void setWIN32Time(const QDateTime& qdt);
    #endif
    Ui::MainWindow *ui;
    QUdpSocket *udpsocket;
    QTimer *timer;
    QSettings* settings;
    QStringList auto_servers;
};

#endif // MAINWINDOW_H
