#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SerialPort=new QSerialPort(this);
    status_timer = new QTimer(this);
    status_time_update_timer=new QTimer(this);
    rule_window_timer=new QTimer(this);
    rule_read_timer=new QTimer(this);
    Stm8_addr=0x01;
    setWindowTitle("STM8S105");//设置标题
    //禁止改变窗口大小
    setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);
    setFixedSize(this->width(), this->height());

    connect(ui->OpenCom,SIGNAL(clicked()),this,SLOT(OpenCom())); //配置打开按钮
    connect(ui->CloseCom,SIGNAL(clicked()),this,SLOT(CloseCom()));//配置关闭按钮
    connect(SerialPort,SIGNAL(readyRead()),this,SLOT(readyRead()));//配置准备读信号
    connect(ui->Write,SIGNAL(clicked()),this,SLOT(WriteBtn()));
    connect(ui->Read,SIGNAL(clicked()),this,SLOT(ReadBtn()));
    connect(ui->StatusBtn,SIGNAL(clicked()),this,SLOT(StatusBtn()));
    connect(ui->status_exitbtn,SIGNAL(clicked()),this,SLOT(status_exitbtn()));
    connect(ui->status_stop_timerbtn,SIGNAL(clicked()),this,SLOT(status_stop_timerbtn()));
    connect(ui->status_start_timerbtn,SIGNAL(clicked()),this,SLOT(status_start_timerbtn()));
    connect(ui->status_beep_on,SIGNAL(clicked()),this,SLOT(status_beep_on()));
    connect(ui->status_beep_off,SIGNAL(clicked()),this,SLOT(status_beep_off()));
    connect(ui->status_relay_on,SIGNAL(clicked()),this,SLOT(status_relay_on()));
    connect(ui->status_relay_off,SIGNAL(clicked()),this,SLOT(status_relay_off()));
    connect(ui->status_time_update,SIGNAL(clicked()),this,SLOT(status_time_update()));
    connect(ui->loaddata_buff,SIGNAL(clicked()),this,SLOT(loaddata_buff()));
    connect(ui->savedata_buff,SIGNAL(clicked()),this,SLOT(savedata_buff()));
    connect(status_timer,SIGNAL(timeout()),this,SLOT(status_timer_timeout()));//关联状态窗口定时器
    connect(status_time_update_timer,SIGNAL(timeout()),this,SLOT(status_time_update_timeout()));
    connect(ui->RuleBtn,SIGNAL(clicked()),this,SLOT(RuleBtn()));
    connect(ui->Rule_exit,SIGNAL(clicked()),this,SLOT(Rule_Exit()));
    connect(rule_window_timer,SIGNAL(timeout()),this,SLOT(rule_window_timer_timeout()));
    connect(ui->rule_on,SIGNAL(clicked()),this,SLOT(rule_on()));
    connect(ui->rule_off,SIGNAL(clicked()),this,SLOT(rule_off()));
    connect(rule_read_timer,SIGNAL(timeout()),this,SLOT(rule_read_timer_timeout()));
    connect(ui->rule_read,SIGNAL(clicked()),this,SLOT(rule_read()));
    connect(ui->rule_current_on,SIGNAL(clicked()),this,SLOT(rule_current_on()));
    connect(ui->rule_current_off,SIGNAL(clicked()),this,SLOT(rule_current_off()));
    connect(ui->rule_current_beep_on,SIGNAL(clicked()),this,SLOT(rule_current_beep_on()));
    connect(ui->rule_current_beep_off,SIGNAL(clicked()),this,SLOT(rule_current_beep_off()));
    connect(ui->rule_current_relay_on,SIGNAL(clicked()),this,SLOT(rule_current_relay_on()));
    connect(ui->rule_current_relay_off,SIGNAL(clicked()),this,SLOT(rule_current_relay_off()));
    UpdateComInfo();
    {
     //限制文本框输入内容
     ui->Stm8_addr->setValidator(new QIntValidator(0,0xff,this));
     ui->address->setValidator(new QIntValidator(0,1199, this));
     ui->value->setValidator(new QIntValidator(0,0xff, this));
     ui->rule_index->setValidator(new QIntValidator(1,16,this));
     //textedit设置滚动条
     ui->log->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
     ui->log->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
     ui->log->verticalScrollBar()->setValue(ui->log->verticalScrollBar()->maximumHeight());

    }
    ui->frame->setEnabled(false);
    ui->frame_1->setEnabled(false);
    ui->frame_2->setEnabled(false);
    {//清空data_buff
     unsigned int i=0;
     for(i=0;i<sizeof(data_buff);i++)
         data_buff[i]=0;
    }
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
     ui->frame_1->setEnabled(false);
     ui->frame_2->setEnabled(false);
     status_timer->stop();
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
       ui->frame_1->setEnabled(false);
       ui->frame_2->setEnabled(false);
       status_timer->stop();
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
SerialPort->clearError();
SerialPort->clear();

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
void MainWindow::status_timer_timeout()
{
{//读取数据
static int timeoutcount=6;
if(IsReceived || !timeoutcount)//空闲时读取相关参数
{
static int count=0;
switch(count)
{
case 0:
    ReadFromStm8(1030);break;
case 1:
    ReadFromStm8(1031);break;
case 2:
    ReadFromStm8(1032);break;
case 3:
    ReadFromStm8(1040);break;
case 4:
    ReadFromStm8(1041);break;
case 5:
    ReadFromStm8(1042);break;
case 6:
    ReadFromStm8(1050);break;
case 7:
    ReadFromStm8(1051);break;
case 8:
    ReadFromStm8(1052);break;
case 9:
    ReadFromStm8(1053);break;
case 10:
    ReadFromStm8(1054);break;
case 11:
    ReadFromStm8(1060);break;
case 12:
    ReadFromStm8(1061);break;
case 13:
    ReadFromStm8(1062);break;
case 14:
    ReadFromStm8(1063);break;
case 15:
    ReadFromStm8(1064);break;
case 16:
    ReadFromStm8(1070);break;
case 17:
    ReadFromStm8(1080);break;
default:
    count=-1;break;
}
count++;
qDebug() <<"读取序号："<<count;
timeoutcount=6;
}
timeoutcount--;
}

{//更新窗口状态
 {//更新时间
 QTime temp_time;
 temp_time.setHMS(data_buff[1030]/16*10+data_buff[1030]%16,data_buff[1031]/16*10+data_buff[1031]%16,data_buff[1032]/16*10+data_buff[1032]%16);//填写时间
 {
 ui->timeEdit->setMaximumTime(QTime(23,59,59));
 ui->timeEdit->setMinimumTime(QTime(0,0,0));
 ui->timeEdit->setUpdatesEnabled(true);
 }
 ui->timeEdit->setTime(temp_time);
   qDebug()<<temp_time.toString("HH:mm:ss");
 }
 {//更新温湿度
  char temp[20];
  sprintf(temp,"%4.1f度，%4.1f%%",data_buff[1052]+0.1*(data_buff[1053] & 0xf),data_buff[1050]+0.1*(data_buff[1051] & 0xf));
  ui->label_6->setText(temp);
 }
 {//更新ADC数据
  char temp[20];
  sprintf(temp,"%d,%s",data_buff[1040]*256+data_buff[1041],(data_buff[1042]?"开":"关"));
  ui->label_8->setText(temp);

 }
 {//更新报警和继电器状态
  ui->label_10->setText(data_buff[1070]?"开":"关");
  ui->label_12->setText(data_buff[1080]?"开":"关");
 }
}
}

void MainWindow::StatusBtn()
{
    ui->frame->setEnabled(false);
    ui->frame_1->setEnabled(true);
    //status_timer->start(100);
    {
    ui->timeEdit->setMaximumTime(QTime(23,59,59));
    ui->timeEdit->setMinimumTime(QTime(0,0,0));
    ui->timeEdit->setUpdatesEnabled(true);
    }

}
void MainWindow::status_exitbtn()
{
    status_timer->stop();
    ui->frame->setEnabled(true);
    ui->frame_1->setEnabled(false);
}
void MainWindow::status_stop_timerbtn()
{
    status_timer->stop();
     ui->status_start_timerbtn->setEnabled(true);
     ui->status_stop_timerbtn->setEnabled(false);
     ui->status_beep_off->setEnabled(true);
     ui->status_beep_on->setEnabled(true);
     ui->status_relay_off->setEnabled(true);
     ui->status_relay_on->setEnabled(true);
     ui->status_time_update->setEnabled(true);
}
void MainWindow::status_start_timerbtn()
{
    status_timer->start(400);
    ui->status_start_timerbtn->setEnabled(false);
    ui->status_stop_timerbtn->setEnabled(true);
    ui->status_beep_off->setEnabled(false);
    ui->status_beep_on->setEnabled(false);
    ui->status_relay_off->setEnabled(false);
    ui->status_relay_on->setEnabled(false);
     ui->status_time_update->setEnabled(false);
}


unsigned status_changeflag=0;//状态窗口中改变的设置类型标志位
void MainWindow::status_beep_off()
{
    data_buff[1070]=0;
    status_time_update_timer->start(100);
    status_changeflag=1;

}
void MainWindow::status_beep_on()
{
    data_buff[1070]=1;
    status_time_update_timer->start(100);
    status_changeflag=1;
}
void MainWindow::status_relay_off()
{
    data_buff[1080]=0;
    status_time_update_timer->start(100);
    status_changeflag=2;
}
void MainWindow::status_relay_on()
{
    data_buff[1080]=1;
    status_time_update_timer->start(100);
    status_changeflag=2;
}
void MainWindow::status_time_update()
{
    unsigned char nhour,nminute,nsecond;
    QTime temp_time=ui->timeEdit->time();
    nhour=temp_time.hour()/10*16+temp_time.hour()%10;
    nminute=temp_time.minute()/10*16+temp_time.minute()%10;
    nsecond=temp_time.second()/10*16+temp_time.second()%10;
    qDebug()<<nhour<<' '<<nminute<<' '<<nsecond;
    data_buff[1030]=nhour;
    data_buff[1031]=nminute;
    data_buff[1032]=nsecond;
    status_time_update_timer->start(100);
    status_changeflag=0;


}
void MainWindow::status_time_update_timeout()
{
    static int count=0;
    switch (count)
    {
    case 0:
    ;if(status_changeflag ==0) WriteToStm8(1030,data_buff[1030]);  break;
    case 1:
    ;if(status_changeflag ==0) WriteToStm8(1031,data_buff[1031]);break;
    case 2:
    ;if(status_changeflag ==0) WriteToStm8(1032,data_buff[1032]);break;
    case 3:
    ;if(status_changeflag ==1) WriteToStm8(1070,data_buff[1070]);break;
    case 4:
    ;if(status_changeflag ==2) WriteToStm8(1080,data_buff[1080]);break;
    default:
        count=-1;
        status_time_update_timer->stop();
        break;
    }
    count++;
    {//更新窗口状态
     {//更新时间
     QTime temp_time;
     temp_time.setHMS(data_buff[1030]/16*10+data_buff[1030]%16,data_buff[1031]/16*10+data_buff[1031]%16,data_buff[1032]/16*10+data_buff[1032]%16);//填写时间
     {
     ui->timeEdit->setMaximumTime(QTime(23,59,59));
     ui->timeEdit->setMinimumTime(QTime(0,0,0));
     ui->timeEdit->setUpdatesEnabled(true);
     }
     ui->timeEdit->setTime(temp_time);
       qDebug()<<temp_time.toString("HH:mm:ss");
     }
     {//更新温湿度
      char temp[20];
      sprintf(temp,"%4.1f度，%4.1f%%",data_buff[1052]+0.1*(data_buff[1053] & 0xf),data_buff[1050]+0.1*(data_buff[1051] & 0xf));
      ui->label_6->setText(temp);
     }
     {//更新ADC数据
      char temp[20];
      sprintf(temp,"%d,%s",data_buff[1040]*256+data_buff[1041],(data_buff[1042]?"开":"关"));
      ui->label_8->setText(temp);

     }
     {//更新报警和继电器状态
      ui->label_10->setText(data_buff[1070]?"开":"关");
      ui->label_12->setText(data_buff[1080]?"开":"关");
     }
    }
}

void MainWindow::loaddata_buff()
{
if(QFile::exists("data.bin"))
{
    QFile   temp("data.bin");
    temp.open(QIODevice::ReadOnly);
    temp.read((char *)data_buff,sizeof(data_buff));

}
else
{
    QMessageBox::about(NULL,"提示","data不存在");
}
}
void MainWindow::savedata_buff()
{
    QFile   temp("data.bin");
    temp.open(QIODevice::WriteOnly);
    temp.write((char *)data_buff,sizeof(data_buff));
}

void MainWindow::RuleBtn()
{
ui->frame_2->setEnabled(true);
ui->frame->setEnabled(false);
rule_window_timer->start(200);
ReadFromStm8(4);//读取总开关


}
void MainWindow::Rule_Exit()
{
    ui->frame_2->setEnabled(false);
    ui->frame->setEnabled(true);
    rule_window_timer->stop();
    rule_read_timer->stop();

}
void MainWindow::rule_window_timer_timeout()
{
    int index=atoi(ui->rule_index->text().toStdString().data());
    if(index<1 || index >16) {
                                QMessageBox::about(NULL,"提示","规则引索错误");
                                return;

                              }
    {//总开关状态
        if(data_buff[4]) ui->label_14->setText("开");
        else ui->label_14->setText("关");
    }
    {//当前规则开关状态
       if(data_buff[32*index]) ui->label_17->setText("是");
       else ui ->label_17->setText("否");
    }
    {//当前报警输出
       if(data_buff[32*index+30]) ui->label_19->setText("启用");
       else ui ->label_19->setText("关闭");
    }
    {//当前继电器输出
       if(data_buff[32*index+31]) ui->label_20->setText("启用");
       else ui ->label_20->setText("关闭");
    }
    {//更新读写状态
     if(rule_read_timer->isActive())
     {
        ui->rule_read->setEnabled(false);

     }
     else
     {
         ui->rule_read->setEnabled(true);
     }

    }
}

void MainWindow::rule_on()
{
    WriteToStm8(4,1);
}
void MainWindow::rule_off()
{
    WriteToStm8(4,0);
}

void MainWindow::rule_read_timer_timeout()
{
static int count=6,index;
index=atoi(ui->rule_index->text().toStdString().data());
if(index<1 || index >16) {
                            QMessageBox::about(this,"提示","规则引索错误");
                            return;

                          }
if(IsReceived || !count)
{
    static int add=0;

    if(!count) { //上次读取失败
        if(add!=0) add--;
    }
    ReadFromStm8(32*index+add);
    qDebug()<<"读取地址:"<<32*index+add;
    add++;
    if(add==32) {//读取完成
        add=0;
        rule_read_timer->stop();
        }
    count=6;


}
count--;

}
void MainWindow::rule_read()
{
rule_read_timer->start(300);
}
void MainWindow::rule_current_on()
{
    int index=atoi(ui->rule_index->text().toStdString().data());
    if(index<1 || index >16) {
                                QMessageBox::about(NULL,"提示","规则引索错误");
                                return;

                              }
    WriteToStm8(32*index,1);
}
void MainWindow::rule_current_off()
{
    int index=atoi(ui->rule_index->text().toStdString().data());
    if(index<1 || index >16) {
                                QMessageBox::about(NULL,"提示","规则引索错误");
                                return;

                              }
    WriteToStm8(32*index,0);
}
void MainWindow::rule_current_beep_on()
{
    int index=atoi(ui->rule_index->text().toStdString().data());
    if(index<1 || index >16) {
                                QMessageBox::about(NULL,"提示","规则引索错误");
                                return;

                              }
    WriteToStm8(32*index+30,1);
}
void MainWindow::rule_current_beep_off()
{
    int index=atoi(ui->rule_index->text().toStdString().data());
    if(index<1 || index >16) {
                                QMessageBox::about(NULL,"提示","规则引索错误");
                                return;

                              }
    WriteToStm8(32*index+30,0);
}
void MainWindow::rule_current_relay_on()
{
    int index=atoi(ui->rule_index->text().toStdString().data());
    if(index<1 || index >16) {
                                QMessageBox::about(NULL,"提示","规则引索错误");
                                return;

                              }
    WriteToStm8(32*index+31,1);
}
void MainWindow::rule_current_relay_off()
{
    int index=atoi(ui->rule_index->text().toStdString().data());
    if(index<1 || index >16) {
                                QMessageBox::about(NULL,"提示","规则引索错误");
                                return;

                              }
    WriteToStm8(32*index+31,0);
}
