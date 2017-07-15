#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QtCore/qprocess.h>
#include <QString>
#include <QObject>
#include <QPushButton>
#include <qprocess.h>
#include <QMessageBox>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <termios.h>
#include <strings.h>
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>
#include <QTimer>

#include "serialmodbus.h"

#define WIFI_ON 0
#define uint8_t char
#define uint16_t short int
#define uint32_t int
#define FORREAD 0xA0
#define FORWRITE 0xA1
#define FORVERSION 0xA2
#define FORINPUT 0xB1

#define LED_FUN_DIR  "./led"
#define UPS_FUN_DIR "./ups"
#define NTPDATE_FUN_DIR "./ntp"
#define NUM 1024

#pragma pack(2)
struct CMD
{
    uint8_t HeadPack;    //功能码
    uint8_t Break; 	    //
    uint16_t CrcCheck;  //校验位
};
#pragma pack()
#pragma pack(2)
struct OUTBUF
{
    uint8_t HeadPack;
    uint8_t PackNum;
    uint16_t Monitor24V;
    uint16_t MonitorUPS;
    uint16_t Monitor5V;
    uint16_t TmpTest;
    uint16_t Charge_time;
    uint16_t DisCharge_time;
    uint16_t CrcCheck;
};
#pragma pack()
#pragma pack(2)
struct VERSION
{
    uint8_t HeadPack;
    uint8_t PackNum;
    uint32_t Version;
    uint16_t CrcCheck;
};
#pragma pack()
#pragma pack(2)
struct INPUT
{
    uint8_t HeadPack;
    uint8_t PackNum;
    uint8_t IsLedOn;		//0x00 Off     0x01 On
    uint8_t Out;
    uint16_t CrcCheck;

};
#pragma pack()
#pragma pack(2)
struct ECHO1
{
    uint8_t HeadPack;
    uint8_t Break;
    uint16_t CrcCheck;
};
#pragma pack()
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    int Run_state;
    int Out_state;
    QProcess *led_btn;
    QProcess *ups;
    QStringList pa;
    QTimer *timer;
    SerialModbus  *Larkserial;
    bool serialModbusflag = false;
public slots:
    int write_tofile(FILE *fp,char *buf,char *file_path);
    int read_fromfile(FILE *fp,char *buf,char *file_path);
    int Uart_open(char *Port);
    uint16_t  Get_CrcCheck(struct CMD *cmd);
    uint16_t	recv_CrcCheck(struct OUTBUF *cmd);
    void send_info(struct CMD*,uint8_t,uint8_t);
    int Send_toUart(int,struct CMD *);
    int Recv_toUart(int,struct OUTBUF *);
    int Recv_toUartVer(int,struct VERSION *);
    float itof(int,int);
    int set_opt(int fd,int nSpeed,int nBits,char nEvent,int nStop);
    uint16_t  Version_CrcCheck(struct VERSION *cmd);
    void get_cputemp(char *);
    uint16_t input_CrcCheck(struct INPUT *cmd);
    uint16_t echo_CrcCheck(struct ECHO1 *cmd);
    void usb_test(void);
    void usbeth_test(void);
    void ethcat_test(void);
    void wifi_test(void);
    void C_mcu(void);
    void C_Modbus_mcu(void);
    void On_Led_read();
    void On_Ups_read();
    void Get_led_state(int *run, int *out);
    void set_Btn_state(int run,int out);
    void display_curtime();
private slots:
    void on_MCU_Btn_clicked();
    void on_Run_key_clicked();
    void on_Y1_Key_clicked();
    void on_Y2_key_clicked();
};

#endif // MAINWINDOW_H
