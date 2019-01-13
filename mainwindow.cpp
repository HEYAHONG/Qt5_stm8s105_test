#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SerialPort=new QSerialPort(this);
    Stm8_addr=0x01;
    setWindowTitle("STM8S105");//设置标题
    connect(ui->OpenCom,SIGNAL(clicked()),this,SLOT(OpenCom())); //配置打开按钮
    connect(ui->CloseCom,SIGNAL(clicked()),this,SLOT(CloseCom()));//配置关闭按钮
    connect(SerialPort,SIGNAL(readyRead()),this,SLOT(readyRead()));//配置准备读信号
    connect(ui->Write,SIGNAL(clicked()),this,SLOT(WriteBtn()));
    connect(ui->Read,SIGNAL(clicked()),this,SLOT(ReadBtn()));
    UpdateComInfo();
    {
     //限制文本框输入内容
     ui->Stm8_addr->setValidator(new QIntValidator(0,0xff,this));
     ui->address->setValidator(new QIntValidator(0,1199, this));
     ui->value->setValidator(new QIntValidator(0,0xff, this));
     //textedit设置滚动条
     ui->log->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
     ui->log->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
     ui->log->verticalScrollBar()->setValue(ui->log->verticalScrollBar()->maximumHeight());

    }
    ui->frame->setEnabled(false);
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
     SerialPort->setBaudRate(QSerialPort::Baud9600);
     //设置数据位
     SerialPort->setDataBits(QSerialPort::Data8);
     //设置校验位
     SerialPort->setParity(QSerialPort::NoParity);
     //设置流控制
     SerialPort->setFlowControl(QSerialPort::NoFlowControl);
     //设置停止位
     SerialPort->setStopBits(QSerialPort::OneStop);
    /*
     {   //测试modbus
         unsigned char data[8]={0x01,0x03,0x03,0xff,0x00,0x01,0x00,0x00};
         SerialPort->write((char *)data,8);

     }
     */

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
     ui->frame->setEnabled(true);
 }
 else
 {
     ui->OpenCom->setEnabled(true);
     ui->CloseCom->setEnabled(false);
     ui->frame->setEnabled(false);
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
       ui->frame->setEnabled(true);
   }
   else
   {
       ui->OpenCom->setEnabled(true);
       ui->CloseCom->setEnabled(false);
       ui->frame->setEnabled(false);
   }
}

void MainWindow::readyRead()
{


//读取串口数据
QByteArray Data=SerialPort->readAll();

 //将接收的数据在控制台DeBug打印。
if(Data.size()!=0)
{

    qDebug() <<Data.toHex().toStdString().data();
    ui->log->appendPlainText("接收:");
    ui->log->appendPlainText(Data.toHex().data());
    if(Data.size()>=6)
    {
        unsigned char * temp=(unsigned char *)Data.data();
        Stm8_addr=temp[0];
        if(temp[1]==03)//03 代码
        {
          data_buff[curr_address]=temp[4];
          char temp_char[5];
          sprintf(temp_char,"%d",temp[4]);
          ui->value->setText(temp_char);
        }
        if(temp[1]==06)//06 代码
        {
          data_buff[curr_address]=temp[5];
          char temp_char[5];
          sprintf(temp_char,"%d",temp[5]);
          ui->value->setText(temp_char);
        }
        IsReceived=true;

    }
}
Data.clear();

}
unsigned int MainWindow::CRC16(unsigned char *arr_buff,unsigned char len)
{

    unsigned int crc=0xFFFF;
    unsigned char i, j;
     for ( j=0; j<len;j++)
     {
       crc=crc ^*arr_buff++;
       for ( i=0; i<8; i++)
      {
           if( ( crc&0x0001) >0)
           {
               crc=crc>>1;
               crc=crc^ 0xa001;
            }
          else
              crc=crc>>1;
       }

}
     return ( crc);
}
//读写STM8的EEPROM
unsigned char MainWindow::WriteToStm8(unsigned int address,unsigned char data)
{
    unsigned char buff[8];
    //填写从机地址
    buff[0]=Stm8_addr;
    //填写功能代码
    buff[1]=0x06;
    // 填写寄存器地址
    buff[2]=(address & 0xff00) >>8;
    buff[3]=(address&0xff);
    //填写值
    buff[4]=0x00;
    buff[5]=data;
    //填写校验（低字节在前）
    buff[7]=(CRC16(buff,6) & 0xff00) >>8;
    buff[6]=(CRC16(buff,6)&0xff);


    //发送数据
     SerialPort->write((char *)buff,8);
     {//显示log
       QByteArray logData((char *)buff,8);
       ui->log->appendPlainText("发送:");
       ui->log->appendPlainText(logData.toHex());
     }
      curr_address=address;
      IsReceived=false;

     return 0;
}
unsigned char MainWindow::ReadFromStm8(unsigned int address)
{
   unsigned char buff[8];
   //填写从机地址
   buff[0]=Stm8_addr;
   //填写功能代码
   buff[1]=0x03;
   // 填写寄存器地址
   buff[2]=(address & 0xff00) >>8;
   buff[3]=(address&0xff);
   //填写数量
   buff[4]=0x00;
   buff[5]=0x01;
   //填写校验（低字节在前）
   buff[7]=(CRC16(buff,6) & 0xff00) >>8;
   buff[6]=(CRC16(buff,6)&0xff);

   //发送数据
    SerialPort->write((char *)buff,8);
    {//显示log
      QByteArray logData((char *)buff,8);
      ui->log->appendPlainText("发送:");
      ui->log->appendPlainText(logData.toHex());
    }

    curr_address=address;
    IsReceived=false;

    return 0;
}
void MainWindow::ReadBtn()
{
Stm8_addr=atoi(ui->Stm8_addr->text().toStdString().data());
unsigned int address=atoi(ui->address->text().toStdString().data());
ReadFromStm8(address);
}
void MainWindow::WriteBtn()
{
    Stm8_addr=atoi(ui->Stm8_addr->text().toStdString().data());
    unsigned int address=atoi(ui->address->text().toStdString().data());
    unsigned char value=atoi(ui->value->text().toStdString().data());
    WriteToStm8(address,value);
}
