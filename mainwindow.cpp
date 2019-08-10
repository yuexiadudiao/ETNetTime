#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtNetwork/QHostInfo>
#include <QtNetwork/QUdpSocket>
#include <QDateTime>
#include <QTimer>
#include <QMessageBox>
#include <QSettings>

//设置系统时间的接口与操作系统有关
#if defined(Q_OS_WIN32)
    #include<Windows.h>
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //设置默认NTP服务器
    auto_servers.append(QString("ntp1.aliyun.com"));

    //采用定时器，每秒更新一次localtime和nettime文本框
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(timerUpdate()));
    connect(timer,SIGNAL(timeout()),this,SLOT(nettimerUpdate()));
    timer->start(1000);

    settings =new QSettings("config.ini",QSettings::IniFormat,this);

    updateComboBox();
}

MainWindow::~MainWindow()
{
    delete ui;
}


/* 连接校时服务器 */
void MainWindow::connectsucess()
{
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
    //在这段时间内，不允许再次查询
   QTimer::singleShot(1*1000,this,SLOT(quirytimeout()));
   // ui->pushButton->setEnabled(false);
}
/* 读取校时服务器时间 */
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

/* localtime文本框回调 */
void MainWindow::timerUpdate()
{
    QDateTime time = QDateTime::currentDateTime();
    QString str = time.toString("yyyy-MM-dd hh:mm:ss dddd");
    ui->lineEdit_3->setText(str);
}
/* nettime文本框回调 */
void MainWindow::nettimerUpdate()
{
    QString timestr = ui->lineEdit_2->text();
    if(!timestr.isNull())//有数据就更新
    {
        QDateTime newnettime = QDateTime::fromString(timestr,"yyyy-MM-dd hh:mm:ss dddd");
        ui->lineEdit_2->setText(newnettime.addMSecs(1000).toString("yyyy-MM-dd hh:mm:ss dddd"));
    }
}
/* “查询”按钮回调 */
void MainWindow::on_pushButton_clicked()
{
    if(ui->comboBox->count() == 0)//下拉列表为空
    {
        QMessageBox::information(NULL, "错误", "请添加服务器", QMessageBox::Yes, QMessageBox::Yes);
        return ;
    }

    QHostInfo info = QHostInfo::fromName(ui->comboBox->currentText());
    if (info.addresses().isEmpty())//无法获得host的ip，host错误或没有网络
    {
        qDebug()<<"error = "<<info.error();
        QMessageBox::information(NULL, "错误", "无法获取NTP服务器ip", QMessageBox::Yes, QMessageBox::Yes);
        return ;
    }
       QString ipAddress=info.addresses().first().toString();
       qDebug()<<"ipAddress="<<ipAddress;
        udpsocket=new QUdpSocket(this);
        connect(udpsocket,SIGNAL(connected()),this,SLOT(connectsucess()));
        connect(udpsocket,SIGNAL(readyRead()),this,SLOT(readingDataGrams()));
        udpsocket->connectToHost("time.windows.com",123);
}
/* “校准”按钮回调 */
void MainWindow::on_pushButton_2_clicked()
{
    //验证nettime的合法性【？】
    QString timestr = ui->lineEdit_2->text();

    if(timestr.isEmpty())
    {
       QMessageBox::information(NULL, "错误", "请先查询", QMessageBox::Yes, QMessageBox::Yes);
        return ;
    }
    QDateTime nettime = QDateTime::fromString(timestr,"yyyy-MM-dd hh:mm:ss dddd");



    //跨平台校时
    #if defined(Q_OS_WIN32)
        setWIN32Time(nettime);
    #endif

}
/* “添加”按钮回调 */
void MainWindow::on_pushButton_3_clicked()
{
    //修改配置
    QString addserver = ui->lineEdit->text();
    if(!addserver.isEmpty())
    {
        QStringList servers = settings->value("server").toStringList();
        servers.append(addserver);
        settings->setValue("server",servers);

    //更新combo
    ui->comboBox->addItem(addserver);

    ui->lineEdit->clear();
    }
}
/* “移除”按钮回调 */
void MainWindow::on_pushButton_4_clicked()
{
    int index = ui->comboBox->currentIndex();
    if(ui->comboBox->count())//列表中有数据
    {
        ui->comboBox->removeItem(index);

        QStringList servers = settings->value("server").toStringList();
        servers.removeAt(index);
        settings->setValue("server",servers);

        if(ui->comboBox->count() == 0)//说明是最后一个数据
            settings->remove("server");
    }
}
/* 下拉框显示回调 */
void MainWindow::updateComboBox()
{

    if(!settings->contains("server"))//没有
        settings->setValue("server",auto_servers);
    ui->comboBox->addItems( settings->value("server").toStringList() );
}

/* 最长查询等待时间到了 */
void MainWindow::quirytimeout()
{
    ui->lineEdit_2->setEnabled(true);
}
/* win平台校准时间 */
#if defined(Q_OS_WIN32)
void MainWindow::setWIN32Time(const QDateTime& dt)
{
    //QMessageBox::information(NULL, "即将校正......", dt.toString("yyyy-MM-dd hh:mm:ss dddd"), QMessageBox::Yes, QMessageBox::Yes);
    //因为win函数使用北京的时区自动+8了
    QDateTime qdt = dt.addSecs(-8*60*60);

    QDate dpart = qdt.date();
    QTime tpart = qdt.time();

    //填充windows的时间数据结构
    SYSTEMTIME    stNew ;
    stNew.wYear         = dpart.year();qDebug()<<stNew.wYear<<endl;
    stNew.wMonth        = dpart.month() ;qDebug()<<stNew.wMonth<<endl;
    stNew.wDay          = dpart.day() ;qDebug()<<stNew.wDay<<endl;
    stNew.wHour         = tpart.hour();qDebug()<<stNew.wHour<<endl;
    stNew.wMinute       = tpart.minute();qDebug()<<stNew.wMinute <<endl;
    stNew.wSecond       = tpart.second() ;qDebug()<<stNew.wSecond<<endl;
    stNew.wMilliseconds = 0 ;qDebug()<<stNew.wMilliseconds<<endl;

    // 校时
    if(SetSystemTime(&stNew))
        qDebug()<<"校时成功" ;
    else
        qDebug()<<"Error code:"<<GetLastError() ;
}
#endif
/* mac平台校准时间 */
