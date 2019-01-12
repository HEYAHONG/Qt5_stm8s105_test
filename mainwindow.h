#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <queue>
#include <QMainWindow>
#include <QDebug>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMessageBox>

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


private:
    Ui::MainWindow *ui;
public slots:
    void OpenCom();
    void CloseCom();
    void readyRead(); //串口可以读
};

#endif // MAINWINDOW_H
