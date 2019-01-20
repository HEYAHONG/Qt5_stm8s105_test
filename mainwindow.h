#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#ifdef WIN32
 #include <windows.h>
 #include <conio.h> // for console I/O
 #define sleep(x) Sleep(1000 * (x))
 #define msleep(x) Sleep(x)
 #define CLEAR_TERM system("CLS");
#else
 #include <unistd.h>
 #define msleep(x) usleep(1000 * (x))
 #define CLEAR_TERM system("clear");
#endif

#include <queue>
#include <QMainWindow>
#include <QDebug>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMessageBox>
#include <QScrollBar>
#include <QTimer>
#include <QTime>
#include <QFile>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void UpdateComInfo();
    QSerialPort *SerialPort;
    unsigned int CRC16(unsigned char * arr_buff,unsigned char len);
    //读写STM8的EEPROM
    unsigned char WriteToStm8(unsigned int address,unsigned char data);
    unsigned char ReadFromStm8(unsigned int address);
    //当前读取的地址
    unsigned int curr_address;
    bool  IsReceived;


private:
    Ui::MainWindow *ui;
    //STM8从站地址
    unsigned char Stm8_addr;
    //从STM8读取的数据将存在此数组
    unsigned char data_buff[1200];
    //状态编辑菜单定时器
    QTimer *status_timer;
    QTimer *status_time_update_timer;
    //规则编辑定时器
    QTimer *rule_window_timer;
    QTimer *rule_read_timer;
public slots:
    void OpenCom();
    void CloseCom();
    void readyRead(); //串口可以读
    void ReadBtn();
    void WriteBtn();

    void StatusBtn();
    //状态编辑菜单超时
    void status_timer_timeout();
    void status_exitbtn();
    void status_stop_timerbtn();
    void status_start_timerbtn();
    void status_beep_on();
    void status_beep_off();
    void status_relay_on();
    void status_relay_off();
    void status_time_update();
    void status_time_update_timeout();

    void loaddata_buff();
    void savedata_buff();

    void RuleBtn();
    void Rule_Exit();
    void rule_window_timer_timeout();
    void rule_on();
    void rule_off();
    void rule_read_timer_timeout();
    void rule_read();
    void rule_current_on();
    void rule_current_off();
    void rule_current_beep_on();
    void rule_current_beep_off();
    void rule_current_relay_on();
    void rule_current_relay_off();
    void rule_hour_1_write();
    void rule_hour_2_write();
    void rule_hour_flag_write();
    void rule_minute_1_write();
    void rule_minute_2_write();
    void rule_minute_flag_write();

};

#endif // MAINWINDOW_H
