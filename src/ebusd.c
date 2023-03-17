#include "../include/ebusd.h"


unsigned long ebusd_To_CAN_id()
{

    unsigned char Master_address[26]={
        EBUSD_MASTER_01,EBUSD_MASTER_02,EBUSD_MASTER_03,EBUSD_MASTER_04,
        EBUSD_MASTER_05,EBUSD_MASTER_06,EBUSD_MASTER_07,EBUSD_MASTER_08,
        EBUSD_MASTER_09,EBUSD_MASTER_10,EBUSD_MASTER_11,EBUSD_MASTER_12,
        EBUSD_MASTER_13,EBUSD_MASTER_14,EBUSD_MASTER_15,EBUSD_MASTER_16,
        EBUSD_MASTER_17,EBUSD_MASTER_18,EBUSD_MASTER_19,EBUSD_MASTER_20,
        EBUSD_MASTER_21,EBUSD_MASTER_22,EBUSD_MASTER_23,EBUSD_MASTER_24,EBUSD_MASTER_25};
    unsigned char var=0;
    while (EBUSD_telegram_master.QQ != Master_address[var]) {
        var++;
        if(var>25){
            return 0xFFFFFF;
        }
    }
    unsigned long varL=0x0000;
    varL=(unsigned long)var;
    varL <<= 8;
    varL|=(unsigned long)EBUSD_telegram_master.ZZ;
    varL <<= 8;
    varL|=(unsigned long)EBUSD_telegram_master.PB;
    varL <<= 8;
    varL|=(unsigned long)EBUSD_telegram_master.SB;
    return varL;
}

unsigned char ebusd_crc(unsigned char crc, unsigned char data)
{
    unsigned char polynom;
    int i;

    for (i = 0; i < 8; i++)
    {
       if (crc & 0x80)
       {
          polynom = (unsigned char) EBUSD_Polynomial;
       }
       else
       {
          polynom = (unsigned char) 0;
       }
       crc = (unsigned char)((crc & ~0x80) << 1);
       if (data & 0x80)
       {
          crc = (unsigned char)(crc | 1) ;
       }
       crc = (unsigned char)(crc ^ polynom);
       data = (unsigned char)(data << 1);
    }
    return crc;
}
//{
//    unsigned char i;
//    crc = crc ^ data;
//    for (i = 0; i < 8; i++)
//    {
//        if (crc & 0x80){
//            crc <<= 1;
//            crc ^= EBUSD_Polynomial;
//        } else {
//            crc <<= 1;
//        }
//    }
//    return crc;
//}
