#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDateTime>
#include <QTimer>
#include <QtCore/qmath.h>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Lark 功能测试工具");
    system("insmod ./kerneldriver.ko");
    if(WIFI_ON)
        ui->wifi_test_info->setEnabled(1);
    else
        ui->wifi_test_info->setDisabled(1);
    Run_state=0;
    Out_state=3;
    ui->Run_key->setText("关");
    ui->Y1_Key->setText("关");
    ui->Y2_key->setText("关");
    led_btn = new QProcess;
    ups = new QProcess;
    timer = new QTimer;
    timer->start(100);

    Larkserial = new SerialModbus;

    if (Larkserial->Modbus_Init("/dev/ttyS3",460800,8,'N',1) == 0)
    {
        if(Larkserial->LModbus_ReadAppVer() != MOD_BUS_ERROR_CODE)
        {
            qDebug()<<"modbus mcu ver...";
            serialModbusflag = true;
            Larkserial->setled(0);
            Larkserial->setout(0);
        }
        else
        {
            qDebug()<<"old mcu ver...";
            serialModbusflag = false;
        }
    }

    if(!serialModbusflag)
    {
        pa<<"0"<<"3";
        led_btn->start("./led",pa);

        connect(led_btn,&QProcess::readyReadStandardOutput,this,&MainWindow::On_Led_read);
        system("ntpdate time.nist.gov");
    }

    ups->start(UPS_FUN_DIR);
    connect(ups,&QProcess::readyReadStandardOutput,this,&MainWindow::On_Ups_read);
    connect(timer,SIGNAL(timeout()),this,SLOT(display_curtime()));
    ui->time_label->setStyleSheet("color:black");


}

void MainWindow::display_curtime()
{
    ui->time_label->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd     HH:mm:ss     ddd"));
}
MainWindow::~MainWindow()
{
    delete ui;
    system("rmmod kerneldriver.ko");
}

void MainWindow::On_Led_read()
{


}
void MainWindow::On_Ups_read()
{
    QString ups_info = QLatin1String(ups->readAllStandardOutput());
    qDebug()<<ups_info;
    if(ups_info == QString("UPS ok\n"))
    {
        ui->ups_lab->setStyleSheet("color:green");
        ui->ups_lab->setText("UPS 功能正常，系统即将关机");
    }
    else if(ups_info == QString("read ups error\n"))
    {
        ui->ups_lab->setStyleSheet("color:red");
        ui->ups_lab->setText("读取UPS关机标志失败");
    }
}
void MainWindow::usbeth_test(void)
{
    int ret=0;
    ret=system("ping -f -c 100 192.168.1.1");
    if(ret !=0)
    {
        printf("error\n");
        ui->usb_net_LineEdit->setStyleSheet("color:red");
        ui->usb_net_LineEdit->setText("测试失败");
    }
    else
    {
        ui->usb_net_LineEdit->setStyleSheet("color:black");
        ui->usb_net_LineEdit->setText("测试通过");
    }
}
void MainWindow::C_mcu(void)
{
    //  printf("in func %s\n",__FUNCTION__);
    int fd1;
    int flags;
    int test_count=0;
    //   printf("[malloc before]\n");
    struct CMD *Send_cmd =(struct CMD *)malloc(sizeof(struct CMD));
    struct OUTBUF *Recv_buf = (struct OUTBUF *)malloc(sizeof(struct OUTBUF));
    struct VERSION *ver = (struct VERSION *)malloc(sizeof(struct VERSION));
    //  printf("[malloc Send_cmd is %p,Recv_buf is %p,ver is %p]\n",Send_cmd,Recv_buf,ver);
    int ret=0;
    int ver_temp=0;
    char cpu_temp[4]={0};
    //打开设备文件
    fd1 = Uart_open("/dev/ttyS3");
    printf("fd1 is %d\n",fd1);
    //uart 初始化程序
    flags=set_opt(fd1,460800,8,'N',1);
    if(flags== -1)
    {
        exit(123);
    }
    //		printf("in %s,the in speed is %d,out speed is %d\n",__func__,cfgetispeed(&term),cfgetospeed(&term));
    /**********************A2************************************************/
    /*A2 发送格式化*/
    /*
struct CMD
{
        uint8_t HeadPack;    //功能码
        uint8_t Break; 	    //
        uint16_t CrcCheck;  //校验位
};
    */
    memset(Send_cmd,0,sizeof(CMD));
    send_info(Send_cmd,FORVERSION,0);
    printf("Cmd headpack is 0x%x,Break is 0x%x,CrcCheck is 0x%x\n",\
           Send_cmd->HeadPack,Send_cmd->Break,Send_cmd->CrcCheck);
    flags  = Send_toUart(fd1,Send_cmd);
    if(flags < 0)
    {
        exit(10);
    }
    /*
struct VERSION
{
    uint8_t HeadPack;
    uint8_t PackNum;
    uint32_t Version;
    uint16_t CrcCheck;
    };

*/
    flags = Recv_toUartVer(fd1,ver);
    printf("HeadPack is 0x%x,Packnum  is 0x%x,Version is 0x%x,CrcCheck is 0x%x\n",\
           ver->HeadPack,ver->PackNum,ver->Version,ver->CrcCheck);
    if( ver->CrcCheck == Version_CrcCheck(ver) )
    {
        ver_temp = ver->Version;
    }


    //printf("recv version is %x\n",ver.Version);
    //	usleep(1000);
    /**********************************A0   ************************************/
    //发送的格式化
    memset(Send_cmd,0,sizeof(CMD));
    send_info(Send_cmd,FORREAD,0);
    printf("Cmd headpack is 0x%x,Break is 0x%x,CrcCheck is 0x%x\n",\
           Send_cmd->HeadPack,Send_cmd->Break,Send_cmd->CrcCheck);

    get_cputemp(cpu_temp);
    flags=Send_toUart(fd1,Send_cmd);
    if(flags < 0)
    {
        exit(124);
    }
    ret=Recv_toUart(fd1,Recv_buf);
    if(ret<0){
        exit(125);
    }
    printf("recv outpack ok\n");
    /*
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



*/
    printf("OutBUf HeadPack is 0x%x ,packNum is 0x%x,24V is 0x%x ,Ups is 0x%x ,5V is 0x%x,Tmptest is0x%x,charge_time is 0x%x,Discharge_time is 0x%x,CrcCheck is 0x%x \n",\
           Recv_buf->HeadPack,Recv_buf->PackNum,Recv_buf->Monitor24V,Recv_buf->MonitorUPS,\
           Recv_buf->Monitor5V,Recv_buf->TmpTest,Recv_buf->Charge_time,Recv_buf->DisCharge_time,\
           Recv_buf->CrcCheck);
    if(ret==0)
    {

        /*
             *uint8_t HeadPack;
            *uint8_t PackNum;
            *uint16_t Monitor24V;
            *uint16_t MonitorUPS;
            *uint16_t Monitor5V;
            *uint16_t TmpTest;
            *uint16_t Charge_time;
            *uint16_t DisCharge_time;
            *uint16_t CrcCheck;
             */
        //		printf("the HeadPack is %X,the PackNum is %X\n",Recv_buf.HeadPack,Recv_buf.PackNum);
        //	printf("the 24V is %x the UPS is %x the 5v is %x the testtem is %x ,the ChargeTime is %x,the discharge is %x,the CrcCheck is %X\n",Recv_buf.Monitor24V,\
        //  Recv_buf.MonitorUPS,Recv_buf.Monitor5V,Recv_buf.TmpTest,Recv_buf.Charge_time,Recv_buf.DisCharge_time,Recv_buf.CrcCheck);
#if 1
        if(recv_CrcCheck(Recv_buf)==Recv_buf->CrcCheck)
        {
            //   printf("%s,%.1f ,%x,%.1f ,%.1f ,%.1f ,%d, %d \n",cpu_temp,itof(Recv_buf->Monitor24V),\
            //       ver_temp,itof(Recv_buf->MonitorUPS),itof(Recv_buf->Monitor5V),itof(Recv_buf->TmpTest),Recv_buf->Charge_time,Recv_buf->DisCharge_time);
            ui->v_ups->setText(QString("%1").arg(itof(Recv_buf->MonitorUPS,1)));
            ui->v_5->setText(QString("%1").arg(itof(Recv_buf->Monitor5V,1)));
            ui->v_24->setText(QString("%1").arg(itof(Recv_buf->Monitor24V,1)));
            ui->discharge_time->setText(QString("%1").arg(Recv_buf->DisCharge_time));
            ui->charge_time->setText(QString("%1").arg(Recv_buf->Charge_time));;
            ui->Ver->setText(QString::number(ver_temp,16).toUpper());
            ui->cpu->setText(QString(QLatin1String(cpu_temp)));
            ui->mcu->setText(QString("%1").arg(itof(Recv_buf->TmpTest,1)));;
        }
        else
        {
            //  qDebug()<<"recv_CrcCheck(&Recv_buf)!=Recv_buf.CrcCheck";
            printf("recv_CrcCheck(&Recv_buf)!=Recv_buf.CrcCheck\n");
            test_count++;
        }
#endif
    }
    //   printf("[free before Send_cmd is %p,Recv_buf is %p,ver is %p]\n",Send_cmd,Recv_buf,ver);
    free(Send_cmd );
    //  printf("free cmd over\n");
    free(Recv_buf);
    //  printf("free Recv_buf over\n");
    free(ver);
    //  printf("free ver over\n");
    printf("test_count is %d\n",test_count);
    ::close(fd1);
    //  printf("close fd1 over\n");
}

void MainWindow::C_Modbus_mcu()
{
    char cpu_temp[4]={0};
    get_cputemp(cpu_temp);
    ui->v_ups->setText(QString("%1").arg(itof(Larkserial->LModbus_Readinfo_16b(V_UPS),2)));
    ui->v_5->setText(QString("%1").arg(itof(Larkserial->LModbus_Readinfo_16b(V_DCDC5V),2)));
    ui->v_24->setText(QString("%1").arg(itof(Larkserial->LModbus_Readinfo_16b(V_24V),2)));
    ui->discharge_time->setText(QString("%1").arg(Larkserial->LModbus_Readinfo_16b(DISCHARGE_TIMER)));
    ui->charge_time->setText(QString("%1").arg(Larkserial->LModbus_Readinfo_16b(CHARGE_TIMER)));;
    ui->Ver->setText(QString::number(Larkserial->LModbus_Readinfo_32b(APP_VERSION),16).toUpper());
    ui->cpu->setText(QString(QLatin1String(cpu_temp)));
    ui->mcu->setText(QString("%1").arg(itof(Larkserial->LModbus_Readinfo_16b(TEMPERATURE),1)));
}
void MainWindow::usb_test(void)
{
    //   process_usb_flash->start("/root/workspace/testtool/command/usbtest");
    DIR *dir=NULL;
    struct dirent *read_dir;
    int num=0;
    int ret=0;
    FILE *fp=NULL;
    char str[128]={0};
    char buf[NUM] = {0};
    char buf1[NUM] = {0};
    struct timeval begin;
    struct timeval end;
    char *dir_name[8]={0};
    char *u_name[2]={0};
    int i=0;
    int temp=0;
    char *fflag = NULL;

    ui->usbtest_name1->clear();
    ui->usbtest_name2->clear();
    ui->lineEdit_usbtest->clear();
    ui->lineEdit_usbtest_2->clear();

    gettimeofday(&begin,0);
    do
    {
        dir = opendir("/media/Lark");
        gettimeofday(&end,0);
        if((end.tv_sec-begin.tv_sec)>5)
            break;
    }while(dir == NULL);

    memset(buf,rand(),sizeof(buf));

    if(dir == NULL)
    {
        if(ENOENT == errno)
        {
            // printf("test error\n");
            ui->lineEdit_usbtest->setText("测试失败");
            ui->lineEdit_usbtest_2->setText("测试失败");
        }
        // return -1;
    }
    else
    {
        while((read_dir=readdir(dir)) !=NULL )
        {
            dir_name[i]=read_dir->d_name;
            printf("dir_name[%d] is %s\n",i,dir_name[i]);

            if(dir_name[i] !=NULL && strcmp(dir_name[i],".")!=0 && strcmp(dir_name[i],"..")!=0 && strcmp(dir_name[i],".linuxroot")!=0)
            {
                u_name[num]=dir_name[i];
                //   printf("u_name[%d] is %s\n",num,u_name[num]);
                ui->usbtest_name1->setStyleSheet("color:black");
                ui->usbtest_name2->setStyleSheet("color:black");
                ui->usbtest_name1->setText(u_name[0]);
                ui->usbtest_name2->setText(u_name[1]);
                num++;
            }
            i++;
            temp++;
        }
        if(temp < 4)
        {
            ui->lineEdit_usbtest->setStyleSheet("color:red");
            ui->lineEdit_usbtest_2->setStyleSheet("color:red");
            ui->lineEdit_usbtest->setText("无磁盘");
            ui->lineEdit_usbtest_2->setText("无磁盘");

        }

        for(i=0;i<2;i++)
        {
            if(u_name[i] != 0)
            {

                strcpy(str,"/media/Lark/");
                strcat(str,u_name[i]);
                strcat(str,"/Larkusbfuntest");
                ret = this->write_tofile(fp,buf,str);
                if(ret<0)
                {
                    //  printf("\ntest error\n");
                    if(i == 0)
                        ui->lineEdit_usbtest->setText("写入失败");
                    else if(i ==1)
                        ui->lineEdit_usbtest_2->setText("写入失败");
                }

                ret=this->read_fromfile(fp,buf1,str);
                if(ret < 0)
                {
                    // printf("test error\n");
                    if(i == 0)
                        ui->lineEdit_usbtest->setText("读取失败");
                    else if(i == 1)
                        ui->lineEdit_usbtest_2->setText("读取失败");
                }
                if(strncmp(buf,buf1,sizeof(buf1)) == 0)
                {
                    //   printf("test ok\n")


                    if(i ==0 )
                    {
                        ui->lineEdit_usbtest->setStyleSheet("color:black");
                        ui->lineEdit_usbtest->setText("测试通过");
                    }
                    else if(i == 1)
                    {
                        ui->lineEdit_usbtest_2->setStyleSheet("color:black");
                        ui->lineEdit_usbtest_2->setText("测试通过");
                    }

                }
                else
                {
                    // printf("test error\n");
                    if(i == 0)
                        ui->lineEdit_usbtest->setText("测试失败");
                    else if(i == 1)
                        ui->lineEdit_usbtest_2->setText("测试失败");
                }
            }


        }
    }
    //keyboard test
    dir =opendir("/dev/input/by-path");
    if(dir == NULL)
    {
        ui->kbd_test_infos->setStyleSheet("color:red");
        ui->kbd_test_infos->setText("测试失败");
    }
    else
    {

        while((read_dir=readdir(dir)) !=NULL )
        {
            fflag=strstr(read_dir->d_name,"kbd");
            if(fflag != NULL)
            {
                ui->kbd_test_infos->setStyleSheet("color:black");
                ui->kbd_test_infos->setText("测试通过");
                i=10;
            }
        }

    }
    if(i != 10)
    {
        ui->kbd_test_infos->setStyleSheet("color:red");
        ui->kbd_test_infos->setText("测试失败");

    }
}
int MainWindow:: write_tofile(FILE *fp,char *buf,char *file_path)
{
    int ret=0;
    fp=fopen(file_path,"w+");
    if(fp == NULL)
    {
        //				perror("test error\n");
        return -1;
    }
    ret=fwrite(buf,1,NUM,fp);
    fclose(fp);
    if(ret < 0)
    {
        //				printf("\ntest error\n");
        return -1;
    }
    if(ret == NUM)
    {
        return 0;
    }

}
int MainWindow:: read_fromfile(FILE *fp,char *buf,char *file_path)
{

    int ret=0;
    fp=fopen(file_path,"r");
    ret=fread(buf,1,NUM,fp);
    buf[NUM]='\0';
    fclose(fp);
    if(ret == NUM)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

void MainWindow::ethcat_test(void)
{


    unsigned char	APRDSendBuf[56] = {
        0xff,0xff,0xff,0xff,0xff,0xff,
        0x00,0x88,0xa4,0x00,0x00,0x11,
        0x88,0xa4,0x0d,0x10,0x01,0x00,
        0x00,0x00,0x00,0x09,0x04,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00};
    unsigned char APRDRevBuf[1024]={0};
    int  sd, bytes;
    int count = 0;
    int val=0;
    struct sockaddr_ll device;
    struct ifreq ifr;
    struct timeval begin_time,end_time;

    if ((sd = socket (PF_PACKET, SOCK_RAW, htons (0x88a4))) < 0) {
        perror ("socket() failed to get socket descriptor for using ioctl() ");
        exit (EXIT_FAILURE);
    }
    strncpy(ifr.ifr_name,"eth-gfd",IFNAMSIZ);            //指定网卡名称
    if(-1 == ioctl(sd, SIOCGIFINDEX, &ifr))    //获取网络接口
    {
        perror("ioctl");
        ::close(sd);
        exit(-1);
    }
    memset (&device, 0, sizeof (device));
    device.sll_family = PF_PACKET;
    device.sll_protocol = htons(0x88a4);
    device.sll_ifindex = ifr.ifr_ifindex;
    if(bind(sd,(const struct sockaddr *)(&device),sizeof(device))<0)
    {
        perror("bind is error");
        exit(1);
    }

    if(val== fcntl(sd,F_GETFL,0))
    {
        printf("get fctl error\n");
        exit(1);
    }
    if(-1 == fcntl(sd,F_SETFL,val | O_NONBLOCK))
    {
        printf("set nonblock error\n");
        exit(1);
    }
    int len=sizeof(device);
    int flags =0;
    while(count <=100)
    {

        if ((bytes = sendto (sd, (void *)APRDSendBuf, sizeof(APRDSendBuf), 0, (struct sockaddr *) &device, len)) <= 0)
        {
            perror ("sendto() failed");
            exit (EXIT_FAILURE);
        }
        gettimeofday(&begin_time,0);
        while(1)
        {
            if((bytes=recvfrom(sd, (void *)APRDRevBuf, sizeof(APRDRevBuf), 0, NULL, NULL))>0)
            {
                //		printf("bytes is %d,buf is %s\n",bytes,APRDRevBuf);
                break;
            }
            gettimeofday(&end_time,0);
            if(abs(end_time.tv_usec-begin_time.tv_usec) > 100*1000 )
            {

                flags = 1;
                break;
            }

        }
        count++;
        // usleep(2*1000);
        if(flags == 1)
        {
            break;
        }
    }
    if(flags == 0)
    {
        printf("test ok\n");
        ui->ethercat_line_edit->setStyleSheet("color:black");
        ui->ethercat_line_edit->setText("测试通过");
    }
    else if(flags == 1)
    {
        ui->ethercat_line_edit->setStyleSheet("color:red");
        ui->ethercat_line_edit->setText("测试失败");
    }

    ::close (sd);


}


void MainWindow::wifi_test()
{
    int ret=0;
    ret=system("ping -I wlan0 -f -c 100 192.168.1.1");
    if(ret == 0)
    {
        ui->wifi_test_info->setStyleSheet("color:black");
        ui->wifi_test_info->setText("测试通过");
    }
    else
    {
        ui->wifi_test_info->setStyleSheet("color:red");
        ui->wifi_test_info->setText("测试失败");
    }
}

uint16_t MainWindow:: input_CrcCheck(struct INPUT *cmd)
{
    int i=0;
    cmd->CrcCheck=0x3f;
    uint8_t *pDate=NULL;
    pDate=(uint8_t *)cmd;

    for(i=0;i<sizeof(*cmd)-2;i++)
    {
        cmd->CrcCheck+=* ( pDate + i );
    }
    return cmd->CrcCheck;
}

uint16_t  MainWindow:: echo_CrcCheck(struct	ECHO1	*cmd)
{
    int i=0;
    cmd->CrcCheck=0x3f;
    uint8_t *pDate=NULL;
    pDate=(uint8_t *)cmd;

    for(i=0;i<sizeof(*cmd)-2;i++)
    {
        cmd->CrcCheck+=* ( pDate + i );
    }
    return cmd->CrcCheck;
}

void  MainWindow:: send_info(struct CMD *Send_cmd,uint8_t HeadPack,uint8_t Break)
{
    Send_cmd->HeadPack = HeadPack;
    Send_cmd->Break = Break;
    Send_cmd->CrcCheck =Get_CrcCheck(Send_cmd);
    // return Send_cmd;
}
int MainWindow::Recv_toUartVer(int fd, VERSION *cmd)
{
    int ret=0;
    int len=0;
    char buf[8]={0};
    struct VERSION *p=cmd;
    while(ret != sizeof(*cmd))
    {
        //len=read(fd,p,sizeof(*cmd)-ret);
        len=read(fd,&buf[ret],sizeof(*cmd)-ret);
        ret+=len;
        printf("in %s,len is %d,ret is %d\n",__func__,len,ret);
        if(ret == 8)
        {
            break;
        }
        //p+=len;

    }
    memcpy(p,buf,sizeof(buf));
    //  ret=read(fd,p,sizeof(*cmd));
    if(ret < 0)
    {
        return -1;
    }
    else
    {
        printf("size of VERSION is %d,recv size is %d\n",sizeof(*cmd),ret);
        return 0;
    }
}
int MainWindow:: Recv_toUart(int fd,struct OUTBUF *cmd)
{
    int ret=0;
    int len=0;
    char buf[16]={0};
    struct OUTBUF *p=cmd;
    while(ret != sizeof(*cmd))
    {
        //len=read(fd,p,sizeof(*cmd)-ret);
        len=read(fd,&buf[ret],sizeof(*cmd)-ret);
        ret+=len;
        printf("in %s,len is %d,ret is %d\n",__func__,len,ret);
        if(ret == 16)
        {
            break;
        }
        //p+=len;
    }

    // ret = read(fd,p,sizeof(*cmd));
    memcpy(p,buf,sizeof(buf));
    if(ret < 0)
    {
        return -1;
    }
    else
    {
        printf("size of OUTBUF is %d,recv size is %d\n",sizeof(*cmd),ret);
        return 0;
    }
}
int MainWindow:: Send_toUart(int fd,struct CMD *cmd)
{

    int ret=write(fd,cmd,sizeof(*cmd));
    if(ret < 0)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
//uint16_t  Get_CrcCheck(struct CMD *cmd)
uint16_t MainWindow:: Get_CrcCheck(struct CMD *cmd)
{
    int i=0;
    cmd->CrcCheck=0x3f;
    uint8_t *pDate=NULL;
    pDate=(uint8_t *)cmd;

    for(i=0;i<sizeof(*cmd)-2;i++)
    {
        cmd->CrcCheck+=* ( pDate + i );
    }
    return cmd->CrcCheck;
}
uint16_t  MainWindow:: Version_CrcCheck(struct VERSION *cmd)
{
    int i=0;
    uint16_t myCrcCheck=0x3f;
    uint8_t *pDate=NULL;
    pDate=(uint8_t *)cmd;

    for(i=0;i<sizeof(*cmd)-2;i++)
    {
        myCrcCheck+=* ( pDate + i );
    }
    return myCrcCheck;
}
uint16_t  MainWindow:: recv_CrcCheck(struct OUTBUF *cmd)
{
    int i=0;
    uint16_t myCrcCheck=0x3f;
    uint8_t *pDate=NULL;
    pDate=(uint8_t *)cmd;

    for(i=0;i<sizeof(*cmd)-2;i++)
    {
        myCrcCheck+=* ( pDate + i );
    }
    return myCrcCheck;
}
//串口打开函数
int  MainWindow::Uart_open(char *Port)
{
    int fd;
    fd = open( Port,O_RDWR | O_NDELAY);
    if(fd < 0){
        exit(113);
    }

    //判断是否为阻塞

    if(fcntl(fd,F_SETFL,0)<0)
    {
        exit(12);
    }
    return fd;

}

float  MainWindow::itof(int num, int nBit)
{
    float x=(float) num/qPow(10,nBit);
    return x;

}

int  MainWindow::set_opt(int fd,int nSpeed,int nBits,char nEvent,int nStop)
{
    /*保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息*/
    struct termios oldtio;
    struct termios newtio;
    int flags = 1;
    if(tcgetattr(fd,&oldtio)!=0) {
        perror("SetupSerial 1");
        return -1;
    }
    //printf("oldispeed is %d,oldospeed is %d\n",cfgetispeed(&oldtio),cfgetospeed(&oldtio));
    bzero(&newtio,sizeof(newtio));
    cfmakeraw(&newtio);
    /*步骤一，设置字符大小*/
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;
    /*设置停止位*/
    switch(nBits)
    { case 7:
        newtio.c_cflag |=CS7;
        break;
    case 8:
        newtio.c_cflag |=CS8;
        break; }
    /*设置奇偶校验位*/
    switch(nEvent) {
    case 'O'://奇数    newtio.c_cflag |= PARENB;
        newtio.c_cflag |=PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'E'://偶数
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
    case 'N'://无奇偶校验位
        newtio.c_cflag &= ~PARENB;
        break; }
    /*设置波特率*/
    switch(nSpeed)
    { case 2400:
        cfsetispeed(&newtio,B2400);
        cfsetospeed(&newtio,B2400);
        break;
    case 4800:
        cfsetispeed(&newtio,B4800);
        cfsetospeed(&newtio,B4800);
        break;
    case 9600:
        cfsetispeed(&newtio,B9600);
        cfsetospeed(&newtio,B9600);
        break;
    case 115200:
        //printf("b is %d\n",B115200);
        cfsetispeed(&newtio,B115200);
        cfsetospeed(&newtio,B115200);
        break;
    case 460800:
        //printf("b is %d\n",B460800);
        cfsetispeed(&newtio,B460800);
        cfsetospeed(&newtio,B460800);
        break;
    default:
        cfsetispeed(&newtio,B9600);
        cfsetospeed(&newtio,B9600);
        break; }
    /*设置停止位*/
    if(nStop==1)
        newtio.c_cflag &= ~CSTOPB;
    else if(nStop==2)
        newtio.c_cflag |= CSTOPB;
    /*设置等待时间和最小接收字符*/
    //   newtio.c_cc[VTIME] =0;
    //    newtio.c_cc[VMIN]=8;
    /*处理未接受字符*/
    tcflush(fd, TCIFLUSH);
    /*激活新配置*/
    if((flags = tcsetattr(fd,TCSANOW,&newtio))!=0)
    {
        perror("com set error");
        return -1;
    }
    // printf("set done!\n");
    // printf("flags is %d\n",flags);
    //printf("newispeed is %d,newospeed is %d\n",cfgetispeed(&newtio),cfgetospeed(&newtio));
    return 0;
}
void  MainWindow::get_cputemp(char *buf)
{
    int ret=0;
    FILE *fp=NULL;
    fp = fopen("/sys/class/hwmon/hwmon0/device/temp0_input","r");
    if(fp == NULL)
    {
        exit(1120);
    }
    ret=fread(buf,sizeof(buf),1,fp);
    if(ret < 0)
    {
        fclose(fp);
        exit(1130);
    }
    fclose(fp);
}



void MainWindow::on_MCU_Btn_clicked()
{
    //c_mcu function
    if(!serialModbusflag)
    {
        C_mcu();
    }
    else
    {
         C_Modbus_mcu();
    }
    usleep(1000);
    //usb_eth test
    usbeth_test();
    usleep(1000);
    if(WIFI_ON)
    {
        //wifi test
        wifi_test();
        usleep(1000);
    }
    //usb test
    usb_test();
    usleep(1000);
    //ethcat test
    ethcat_test();
    usleep(1000);



}
void MainWindow::Get_led_state(int *run,int *out)
{
    if(ui->Run_key->text()== "关")
    {
        *run=0;
    }
    else if(ui->Run_key->text()=="开")
    {
        *run=1;
    }
    if(!serialModbusflag)
    {
        if(ui->Y1_Key->text()== "关" && ui->Y2_key->text()=="关")
        {
            //01 11
            *out=0b11;
        }
        else if(ui->Y1_Key->text()=="关" && ui->Y2_key->text()=="开")
        {
            //00 10
            *out=0b01;
        }

        else if(ui->Y1_Key->text()== "开" && ui->Y2_key->text()=="关")
        {
            *out=0b10;
        }
        else if(ui->Y1_Key->text()=="开" &&ui->Y2_key->text() == "开")
        {
            *out =0b00;
        }
    }
    /*modbusver*/
    else
    {
        if(ui->Y1_Key->text()== "关" && ui->Y2_key->text()=="关")
        {
            //01 11
            *out=0;
        }
        else if(ui->Y1_Key->text()=="关" && ui->Y2_key->text()=="开")
        {
            //00 10
            *out=2;
        }

        else if(ui->Y1_Key->text()== "开" && ui->Y2_key->text()=="关")
        {
            *out=1;
        }
        else if(ui->Y1_Key->text()=="开" &&ui->Y2_key->text() == "开")
        {
            *out =3;
        }

    }

}
void MainWindow::set_Btn_state(int run,int out)
{
    if(run ==1)
    {
        ui->Run_key->setStyleSheet("background-color:green");
        ui->Run_key->setText("开");
    }
    else if(run ==0)
    {
        ui->Run_key->setStyleSheet("background-color:white");
        ui->Run_key->setText("关");
    }
    if(!serialModbusflag)
    {
        if(out ==0b00)
        {
            ui->Y1_Key->setStyleSheet("background-color:green");
            ui->Y1_Key->setText("开");
            ui->Y2_key->setStyleSheet("background-color:green");
            ui->Y2_key->setText("开");
        }
        else if(out ==0b01)
        {
            ui->Y1_Key->setStyleSheet("background-color:white");
            ui->Y1_Key->setText("关");
            ui->Y2_key->setStyleSheet("background-color:green");
            ui->Y2_key->setText("开");
        }
        else if(out == 0b10)
        {
            ui->Y1_Key->setStyleSheet("background-color:green");
            ui->Y1_Key->setText("开");
            ui->Y2_key->setStyleSheet("background-color:white");
            ui->Y2_key->setText("关");

        }
        else if(out==0b11)
        {
            ui->Y1_Key->setStyleSheet("background-color:white");
            ui->Y1_Key->setText("关");
            ui->Y2_key->setStyleSheet("background-color:white");
            ui->Y2_key->setText("关");
        }
    }
    else
    {
        if(out == 3)
        {
            ui->Y1_Key->setStyleSheet("background-color:green");
            ui->Y1_Key->setText("开");
            ui->Y2_key->setStyleSheet("background-color:green");
            ui->Y2_key->setText("开");
        }
        else if(out == 2)
        {
            ui->Y1_Key->setStyleSheet("background-color:white");
            ui->Y1_Key->setText("关");
            ui->Y2_key->setStyleSheet("background-color:green");
            ui->Y2_key->setText("开");
        }
        else if(out == 1)
        {
            ui->Y1_Key->setStyleSheet("background-color:green");
            ui->Y1_Key->setText("开");
            ui->Y2_key->setStyleSheet("background-color:white");
            ui->Y2_key->setText("关");

        }
        else if(out == 0)
        {
            ui->Y1_Key->setStyleSheet("background-color:white");
            ui->Y1_Key->setText("关");
            ui->Y2_key->setStyleSheet("background-color:white");
            ui->Y2_key->setText("关");
        }
    }
}
void MainWindow::on_Run_key_clicked()
{
    //get state
//    qDebug()<<"xxxxx-----xxxxx";
    Get_led_state(&Run_state , &Out_state);
    set_Btn_state(Run_state,Out_state);
    if(ui->Run_key->text()== "关")
    {
        Run_state |= 1;
    }
    else if(ui->Run_key->text()=="开")
    {
        Run_state &= 0;
    }
    //set btn state
    set_Btn_state(Run_state,Out_state);
    if(!serialModbusflag)
    {
        QString R = QString::number(Run_state,10);
        QString O = QString::number(Out_state,10);

        QStringList pa;
        pa<<R<<O;
        led_btn->start(LED_FUN_DIR,pa);
    }
    else
    {
//        qDebug()<<"Run_state" << Run_state;
//        qDebug()<<"Out_state" << Out_state;
        Larkserial->setled(Run_state);
        Larkserial->setout(Out_state);
    }
}

void MainWindow::on_Y1_Key_clicked()
{
    Get_led_state(&Run_state , &Out_state);
    set_Btn_state(Run_state,Out_state);
    if(ui->Y1_Key->text()== "开")
    {
        if(!serialModbusflag)
        {
            if((Out_state &0b10) !=0) //10 11
                Out_state|=0b11;
            else
                Out_state|=0b01;
        }
        else
        {
          Out_state &= 2;
        }
    }
    else if(ui->Y1_Key->text()=="关")
    {
        if(!serialModbusflag)
        {
            if((Out_state &0b10) !=0)
                Out_state&=0b10;
            else
                Out_state&=0b00;
        }
        else
        {

            Out_state |= 1;
        }
    }
//    printf("out is %d\n",Out_state);
    //set btn state
    set_Btn_state(Run_state,Out_state);
    if(!serialModbusflag)
    {
        QString R = QString::number(Run_state,10);
        QString O = QString::number(Out_state,10);

        QStringList pa;
        pa<<R<<O;
        led_btn->start(LED_FUN_DIR,pa);
    }
    else
    {
        Larkserial->setout(Out_state);
    }
}

void MainWindow::on_Y2_key_clicked()
{
    //  on_off(0x00,0x02);
    Get_led_state(&Run_state , &Out_state);
    set_Btn_state(Run_state,Out_state);
    if(ui->Y2_key->text()== "开")
    {
        if(!serialModbusflag)
        {
            if((Out_state &0b01) !=0)
                Out_state|=0b11;
            else
                Out_state|=0b10;
        }
        else
        {
            Out_state &= 1;
        }
    }
    else if(ui->Y2_key->text()=="关")
    {
        if(!serialModbusflag)
        {
            if((Out_state &0b01) !=0)
                Out_state&=0b01;
            else
                Out_state&=0b00;

        }
        else
        {

            Out_state |= 2;
        }

    }
    //set btn state
    set_Btn_state(Run_state,Out_state);
    if(!serialModbusflag)
    {
        QString R = QString::number(Run_state,10);
        QString O = QString::number(Out_state,10);

        QStringList pa;
        pa<<R<<O;
        led_btn->start(LED_FUN_DIR,pa);
    }
    else
    {
        Larkserial->setout(Out_state);
    }
}
