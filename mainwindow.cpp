#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SerialPort=new QSerialPort(this);
    setWindowTitle("STM8S105");//设置标题
    connect(ui->OpenCom,SIGNAL(clicked()),this,SLOT(OpenCom())); //配置打开按钮
    connect(ui->CloseCom,SIGNAL(clicked()),this,SLOT(CloseCom()));//配置关闭按钮
    connect(SerialPort,SIGNAL(readyRead()),this,SLOT(readyRead()));//配置准备读信号
    UpdateComInfo();
}

MainWindow::~MainWindow()
{
    delete SerialPort;
    delete ui;
}

void MainWindow::UpdateComInfo() //更新串口信息
{

    //读取串口信息
       foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
       {
           qDebug()<<"Name:"<<info.portName();
           qDebug()<<"Description:"<<info.description();
           qDebug()<<"Manufacturer:"<<info.manufacturer();

           //这里相当于自动识别串口号之后添加到了cmb，如果要手动选择可以用下面列表的方式添加进去
           QSerialPort serial;
           serial.setPort(info);
           if(serial.open(QIODevice::ReadWrite))
           {
               //将串口号添加到cmb
               ui->cmbPortName->addItem(info.portName());
               //关闭串口等待人为(打开串口按钮)打开
               serial.close();
           }
       }

}

void MainWindow::OpenCom()
{

// 设置串口号
 SerialPort->setPortName(ui->cmbPortName->currentText());
 if(SerialPort->open(QIODevice::ReadWrite)) //判断是否打开
 {
     //设置串口速率
     SerialPort->setBaudRate(QSerialPort::Baud115200);
     //设置数据位
     SerialPort->setDataBits(QSerialPort::Data8);
     //设置校验位
     SerialPort->setParity(QSerialPort::NoParity);
     //设置流控制
     SerialPort->setFlowControl(QSerialPort::NoFlowControl);
     //设置停止位
     SerialPort->setStopBits(QSerialPort::OneStop);
 }
 else
 {
     QMessageBox::about(NULL, "提示", "串口没有打开！");
     return;
 }
 if(SerialPort->isOpen()) //更新按键信息
 {
     ui->OpenCom->setEnabled(false);
     ui->CloseCom->setEnabled(true);
 }
 else
 {
     ui->OpenCom->setEnabled(true);
     ui->CloseCom->setEnabled(false);
 }
}
void MainWindow::CloseCom()
{
    //关闭串口
   if(SerialPort->isOpen()) SerialPort->close();

   if(SerialPort->isOpen()) //更新按键信息
   {
       ui->OpenCom->setEnabled(false);
       ui->CloseCom->setEnabled(true);
   }
   else
   {
       ui->OpenCom->setEnabled(true);
       ui->CloseCom->setEnabled(false);
   }
}

void MainWindow::readyRead()
{
//读取串口数据
QByteArray Data=SerialPort->readAll();

 //将接收的数据在控制台DeBug打印。
if(Data.size()!=0)
{
   qDebug() <<Data.toStdString().data();
}
Data.clear();
}
