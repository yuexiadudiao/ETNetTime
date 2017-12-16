#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QUdpSocket>
#include <QTimer>

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
    void on_pushButton_clicked();
    void readingDataGrams();
    void connectsucess();
    void timerUpdate();
    void nettimerUpdate();
    void on_pushButton_2_clicked();

private:
    void setWIN32Time(const QDateTime& qdt);
    Ui::MainWindow *ui;
    QUdpSocket *udpsocket;
    QTimer *timer;
};

#endif // MAINWINDOW_H
