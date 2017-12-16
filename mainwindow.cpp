#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtNetwork/QHostInfo>
#include <QtNetwork/QUdpSocket>
#include <QDateTime>
#include <QTimer>
#include <QMessageBox>

//设置系统时间的接口与操作系统有关
#if defined(Q_OS_WIN32)
    #include<Windows.h>
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //创建绑定启动定时器,这个定时器一直都在运行
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(timerUpdate()));
    connect(timer,SIGNAL(timeout()),this,SLOT(nettimerUpdate()));
    timer->start(1000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timerUpdate()
{
    QDateTime time = QDateTime::currentDateTime();
    QString str = time.toString("yyyy-MM-dd hh:mm:ss dddd");
    ui->lineEdit_3->setText(str);
}

void MainWindow::connectsucess()
{
    ui->lineEdit_2->setText(QString("正在查询时间服务器......"));
    qint8 LI=0;
    qint8 VN=3;
    qint8 MODE=3;
    qint8 STRATUM=0;
    qint8 POLL=4;
    qint8 PREC=-6;
    QDateTime Epoch(QDate(1900, 1, 1));
    qint32 second=quint32(Epoch.secsTo(QDateTime::currentDateTime()));
    qint32 temp=0;

    //封装QByteArray
    QByteArray timeRequest(48, 0);
    timeRequest[0]=(LI <<6) | (VN <<3) | (MODE);
    timeRequest[1]=STRATUM;
    timeRequest[2]=POLL;
    timeRequest[3]=PREC & 0xff;
    timeRequest[5]=1;
    timeRequest[9]=1;
    timeRequest[40]=(temp=(second&0xff000000)>>24);
    temp=0;
    timeRequest[41]=(temp=(second&0x00ff0000)>>16);
    temp=0;
    timeRequest[42]=(temp=(second&0x0000ff00)>>8);
    temp=0;
    timeRequest[43]=((second&0x000000ff));

    //发送时间服务请求
    udpsocket->flush();
    udpsocket->write(timeRequest);
    udpsocket->flush();

    //设定一个定时器，如果一直没有触发，就代表网络不可达

}

void MainWindow::readingDataGrams()
{
    QByteArray newTime;
    QDateTime Epoch(QDate(1900, 1, 1));
    QDateTime unixStart(QDate(1970, 1, 1));
    do
    {
        newTime.resize(udpsocket->pendingDatagramSize());
        udpsocket->read(newTime.data(), newTime.size());
    }while(udpsocket->hasPendingDatagrams());
    QByteArray TransmitTimeStamp ;
    TransmitTimeStamp=newTime.right(8);
    quint32 seconds=TransmitTimeStamp[0];
    quint8 temp=0;
    for(int j=1;j<=3;j++)
    {
        seconds=seconds<<8;
        temp=TransmitTimeStamp[j];
        seconds=seconds+temp;
    }
    QDateTime time;//读取时间并转换成QDateTime
    time.setTime_t(seconds-Epoch.secsTo(unixStart));
    ui->lineEdit_2->setText(time.toString("yyyy-MM-dd hh:mm:ss dddd"));
    this->udpsocket->disconnectFromHost();
    this->udpsocket->close();


}

void MainWindow::nettimerUpdate()
{
    QString timestr = ui->lineEdit_2->text();
    if(!timestr.isNull())//有数据就更新
    {
        QDateTime newnettime = QDateTime::fromString(timestr,"yyyy-MM-dd hh:mm:ss dddd");
        ui->lineEdit_2->setText(newnettime.addMSecs(1000).toString("yyyy-MM-dd hh:mm:ss dddd"));
    }
}

/* “查询”被点击 */
void MainWindow::on_pushButton_clicked()
{
    //QHostInfo info=QHostInfo::fromName("time.windows.com");
    QHostInfo info=QHostInfo::fromName("ntp1.aliyun.com");
       QString ipAddress=info.addresses().first().toString();
       //qDebug()<<"ipAddress="<<ipAddress;
        udpsocket=new QUdpSocket(this);
        connect(udpsocket,SIGNAL(connected()),this,SLOT(connectsucess()));
        connect(udpsocket,SIGNAL(readyRead()),this,SLOT(readingDataGrams()));
        udpsocket->connectToHost("time.windows.com",123);
}

void MainWindow::on_pushButton_2_clicked()
{
    //on_pushButton_clicked();

    QString timestr = ui->lineEdit_2->text();
    QDateTime nettime = QDateTime::fromString(timestr,"yyyy-MM-dd hh:mm:ss dddd");

    //验证nettime的合法性【？】

    //跨平台校时
    #if defined(Q_OS_WIN32)
        setWIN32Time(nettime);
    #endif


}

void MainWindow::setWIN32Time(const QDateTime& qdt)
{
    qDebug()<<qdt.toString("yyyy-MM-dd hh:mm:ss dddd")<<endl;
    QDate dpart = qdt.date();
    QTime tpart = qdt.time();

    //填充windows的时间数据结构
    SYSTEMTIME    stNew ;
    stNew.wYear         = dpart.year();qDebug()<<stNew.wYear<<endl;
    stNew.wMonth        = dpart.month() ;qDebug()<<stNew.wMonth<<endl;
    stNew.wDay          = dpart.day() ;qDebug()<<stNew.wDay<<endl;
    stNew.wHour         = tpart.hour() ;qDebug()<<stNew.wHour<<endl;
    stNew.wMinute       = tpart.minute();qDebug()<<stNew.wMinute <<endl;
    stNew.wSecond       = tpart.second() ;qDebug()<<stNew.wSecond<<endl;
    stNew.wMilliseconds = 0 ;qDebug()<<stNew.wMilliseconds<<endl;

    // 校时
    if(SetSystemTime(&stNew))
        qDebug()<<"校时成功" ;
    else
        qDebug()<<"Error code:"<<GetLastError() ;
}
