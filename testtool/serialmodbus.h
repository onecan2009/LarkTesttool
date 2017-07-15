#ifndef SERIALMODBUS_H
#define SERIALMODBUS_H
#include<stdio.h>
#include<stdlib.h>
#include <modbus/modbus.h>


#define ADDR_IAP					0x8000
#define REGISTER_MAX_NO				100//256-4
#define REGISTER_SIZEOF				2
#define MOD_BUS_ERROR_CODE          0xBEEF

#define PRODUCT_CODE                1
#define APP_VERSION                 2
#define BOOT_VERSION                3
#define V_24V                       4
#define V_UPS                       5
#define V_DCDC5V                    6
#define TEMPERATURE                 7
#define CHARGE_TIMER                8
#define DISCHARGE_TIMER             9


#define LEDCONTROL                  10
#define OUTCONTROL                  11



/*larkC板最新采用modbus协议*/
/*Rtu 数据结构*/
typedef struct
{
    uint32_t 		ProcductCode;
    uint32_t 		AppVersion;
    uint32_t 		BootVersion;
    uint16_t 		Voltage_24V;
    uint16_t 		Voltage_UPS;
    uint16_t 		Voltage_DCDC5V;
    uint16_t 		Temperature;
    uint16_t 		ChargeTimer;
    uint16_t 		DishargeTimer;
    uint16_t 		Reserved;
    uint16_t 		LedControl;		//led 1 灯亮 0 灯灭
    uint16_t 		OutControl;
    uint16_t 		IAPControl;
    uint16_t 		DelayTimer;
}RtuData_t;


typedef struct iap_t
{
    uint16_t u16FileType;
    uint16_t u16FileName;
    uint16_t u16Index;
    uint16_t u16RegisterNo;
    uint16_t u16Reg[REGISTER_MAX_NO];
}IapPacket_t;

class SerialModbus
{
public:
    SerialModbus();
    ~SerialModbus();
    int Modbus_Init(char *serialName,int nSpeed,int nBits,char nEvent,int nStop);
    void Modbus_close(void);
//    int32_t  Modbus_Read(uint32_t u32No , uint32_t u32Addr , uint16_t* pRegs);
//    int32_t Modbus_Write(uint32_t u32No , uint32_t u32Addr , uint16_t* pRegs);
    uint32_t LModbus_ReadAppVer(void);          //readappver
    uint32_t LModbus_ReadProductCode(void);      //readproductcode
    uint32_t LModbus_ReadBootVer(void);//readbootver
    uint32_t LModbus_Readinfo_32b(int n);
    uint16_t LModbus_Readinfo_16b(int n);
    uint16_t setled(int n);
    uint16_t setout(int n);

private:
    modbus_t *mb;
    RtuData_t RtuData;


};

#endif // SERIALMODBUS_H
