#include "serialmodbus.h"
#include "string.h"
#include "stdlib.h"

SerialModbus::SerialModbus()
{

}
SerialModbus::~SerialModbus()
{
    Modbus_close();
}

int SerialModbus::Modbus_Init(char *serialName, int nSpeed, int nBits, char nEvent, int nStop)
{
    int ret = -1;
    mb = modbus_new_rtu(serialName,nSpeed,nEvent,nBits,nStop);
    if(mb == NULL)
    {
        return MOD_BUS_ERROR_CODE;
    }
    if((ret = modbus_set_slave(mb,1)) < 0)//set slave address
    {
        //qDebug()<<"modbus_set_slave error";
        return MOD_BUS_ERROR_CODE;
    }

    if((ret = modbus_connect(mb)) < 0)
    {
        //qDebug()<<"modbus_connect error";
        return MOD_BUS_ERROR_CODE;
    }

    return 0;
}

void SerialModbus::Modbus_close()
{
    modbus_close(mb);
}


//readappver
uint32_t SerialModbus::LModbus_ReadAppVer(void)
{
    struct timeval t;
    int regs = -1;

    t.tv_sec=0;
    t.tv_usec=1000000;//set modbus time 1000ms

    modbus_set_response_timeout(mb,&t);
    if((regs=modbus_read_registers(mb, 0, sizeof(RtuData)/sizeof(uint16_t), (uint16_t*)&RtuData)) < 0)
    {
        //qDebug()<<"modbus_read_registers error";
        return MOD_BUS_ERROR_CODE;
    }

    return RtuData.AppVersion;
}

//readbootver
uint32_t SerialModbus::LModbus_ReadBootVer(void)
{
    struct timeval t;
    int regs = -1;
    memset(&RtuData,0x0,sizeof(RtuData));
    t.tv_sec=0;
    t.tv_usec=1000000;//set modbus time 1000ms

    modbus_set_response_timeout(mb,&t);
    if((regs=modbus_read_registers(mb, 0, sizeof(RtuData)/sizeof(uint16_t), (uint16_t*)&RtuData)) < 0)
    {
        //qDebug()<<"modbus_read_registers error";
        return MOD_BUS_ERROR_CODE;
    }

    return RtuData.BootVersion;
}

//readproductcode
uint32_t SerialModbus::LModbus_ReadProductCode(void)
{
    struct timeval t;
    int regs = -1;
    memset(&RtuData,0x0,sizeof(RtuData));
    t.tv_sec=0;
    t.tv_usec=1000000;//set modbus time 1000ms

    modbus_set_response_timeout(mb,&t);
    if((regs=modbus_read_registers(mb, 0, sizeof(RtuData)/sizeof(uint16_t), (uint16_t*)&RtuData)) < 0)
    {
        //qDebug()<<"modbus_read_registers error";
        return MOD_BUS_ERROR_CODE;
    }

    return RtuData.ProcductCode;
}



uint32_t SerialModbus::LModbus_Readinfo_32b(int n)
{
    struct timeval t;
    int regs = -1;
    memset(&RtuData,0x0,sizeof(RtuData));
    t.tv_sec=0;
    t.tv_usec=1000000;//set modbus time 1000ms

    modbus_set_response_timeout(mb,&t);
    if((regs=modbus_read_registers(mb, 0, sizeof(RtuData)/sizeof(uint16_t), (uint16_t*)&RtuData)) < 0)
    {
        //qDebug()<<"modbus_read_registers error";
        return MOD_BUS_ERROR_CODE;
    }
    switch(n)
    {
        case PRODUCT_CODE:
            return RtuData.ProcductCode;
        case APP_VERSION:
            return RtuData.AppVersion;
        case BOOT_VERSION:
            return RtuData.BootVersion;
        default:
            return 0;
        }
}


uint16_t SerialModbus::LModbus_Readinfo_16b(int n)
{
    struct timeval t;
    int regs = -1;
    memset(&RtuData,0x0,sizeof(RtuData));
    t.tv_sec=0;
    t.tv_usec=1000000;//set modbus time 1000ms

    modbus_set_response_timeout(mb,&t);
    if((regs=modbus_read_registers(mb, 0, sizeof(RtuData)/sizeof(uint16_t), (uint16_t*)&RtuData)) < 0)
    {
        //qDebug()<<"modbus_read_registers error";
        return MOD_BUS_ERROR_CODE;
    }
    switch(n)
    {
        case V_24V:
            return RtuData.Voltage_24V;
        case V_UPS:
            return RtuData.Voltage_UPS;
        case V_DCDC5V:
            return RtuData.Voltage_DCDC5V;
        case TEMPERATURE:
            return RtuData.Temperature;
        case CHARGE_TIMER:
            return RtuData.ChargeTimer;
        case DISCHARGE_TIMER:
            return RtuData.ProcductCode; 
        default:
            return 0;
        }
}

uint16_t SerialModbus::setled(int n)
{
    struct timeval t;
    int regs = -1;
    memset(&RtuData,0x0,sizeof(RtuData));
    t.tv_sec=0;
    t.tv_usec=1000000;//set modbus time 1000ms
    
    if (n !=0  && n != 1)
    {
        return  -1;
    }
    if((regs=modbus_read_registers(mb, 0, sizeof(RtuData)/sizeof(uint16_t), (uint16_t*)&RtuData)) < 0)
    {
        //qDebug()<<"modbus_read_registers error";
        return MOD_BUS_ERROR_CODE;
    }
    RtuData.LedControl = n;
    modbus_set_response_timeout(mb,&t);
    if((regs=modbus_write_registers(mb, 0, sizeof(RtuData)/sizeof(uint16_t), (uint16_t*)&RtuData)) < 0)
    {
        //qDebug()<<"modbus_read_registers error";
        return MOD_BUS_ERROR_CODE;
    }

}


uint16_t SerialModbus::setout(int n)
{
    struct timeval t;
    int regs = -1;
    memset(&RtuData,0x0,sizeof(RtuData));
    t.tv_sec=0;
    t.tv_usec=1000000;//set modbus time 1000ms
    
    if (n <0 || n >3)
    {
        return  -1;
    }
    
    if((regs=modbus_read_registers(mb, 0, sizeof(RtuData)/sizeof(uint16_t), (uint16_t*)&RtuData)) < 0)
    {
        //qDebug()<<"modbus_read_registers error";
        return MOD_BUS_ERROR_CODE;
    }

    RtuData.OutControl = n;
    modbus_set_response_timeout(mb,&t);
    if((regs=modbus_write_registers(mb, 0, sizeof(RtuData)/sizeof(uint16_t), (uint16_t*)&RtuData)) < 0)
    {
        //qDebug()<<"modbus_read_registers error";
        return MOD_BUS_ERROR_CODE;
    }

}
