#ifndef EBUSD_H
#define EBUSD_H

//includes


#ifdef __cplusplus
extern "C"
{
#endif

#define EBUSD_DATA_FIELD_LENGTH 17


// test code crc ebus
//            uint8_t test_data[8]= {0x10, 0x76, 0xB5, 0x11, 0x01, 0x01, 0x16};
//            uint8_t var=0x81;
//            uint8_t crc_uit=0xff;
//            while (crc_uit!=0) {
//                ++var;
//                uint8_t crc=0;
//                uint8_t i=0;
//                for (;i<8;++i) {
//                    crc=test_ebusd_crc(crc,test_data[i],var);
//                }
//                crc_uit=crc;
//            }
// => 0x00 of 0x80

#define EBUSD_Polynomial        0x80

#define EBUSD_SYN       0xaa

//0xAP
//--->A master adres
//--->P prioriteidklasse
#define EBUSD_MASTER_01 0x00 //CAN id 0x00
#define EBUSD_MASTER_02 0x10 //CAN id 0x01 ZZ PB SB
#define EBUSD_MASTER_03 0x30 //CAN id 0x02
#define EBUSD_MASTER_04 0x70 //CAN id 0x03
#define EBUSD_MASTER_05 0xf0 //CAN id 0x04
#define EBUSD_MASTER_06 0x01 //CAN id 0x05
#define EBUSD_MASTER_07 0x11 //CAN id 0x06
#define EBUSD_MASTER_08 0x31 //CAN id 0x07
#define EBUSD_MASTER_09 0x71 //CAN id 0x08 ZZ PB SB
#define EBUSD_MASTER_10 0xf1 //CAN id 0x09
#define EBUSD_MASTER_11 0x03 //CAN id 0x0A ZZ PB SB
#define EBUSD_MASTER_12 0x13 //CAN id 0x0b
#define EBUSD_MASTER_13 0x33 //CAN id 0x0c
#define EBUSD_MASTER_14 0x73 //CAN id 0x0d
#define EBUSD_MASTER_15 0xf3 //CAN id 0x0e
#define EBUSD_MASTER_16 0x07 //CAN id 0x0f
#define EBUSD_MASTER_17 0x17 //CAN id 0x10
#define EBUSD_MASTER_18 0x37 //CAN id 0x11
#define EBUSD_MASTER_19 0x77 //CAN id 0x12
#define EBUSD_MASTER_20 0xf7 //CAN id 0x13
#define EBUSD_MASTER_21 0x0f //CAN id 0x14
#define EBUSD_MASTER_22 0x1f //CAN id 0x15
#define EBUSD_MASTER_23 0x3f //CAN id 0x16
#define EBUSD_MASTER_24 0x7f //CAN id 0x17
#define EBUSD_MASTER_25 0xff //CAN id 0x18

#define EBUSD_ACK_ok    0x00
#define EBUSD_ACK_N_ok  0xff
//Command request symbols (from ebusd to interface)
#define EBUSD_INIT      0x00
#define EBUSD_SEND      0x01
#define EBUSD_START     0x02
#define EBUSD_INFO      0x03

struct EBUSD_telegram
{
    unsigned char QQ;//master adres max 25 verschillend adressen
    unsigned char ZZ;//bestemmings adres max 254 verschillend adressen
    unsigned char PB;//primaire commando max 254 verschillend niet 0xaa en 0xa9
    unsigned char SB;//sekudeer commando max 254 verschillend niet 0xaa en 0xa9
    unsigned char NN;//aantal data byts 0 tot 16
    unsigned char data_byte[EBUSD_DATA_FIELD_LENGTH];
    unsigned char CRC;//de CRC
    unsigned char Slave_ACK;//0x00 = ok, 0xff = niet ok
    unsigned char Slave_NN;//aantal data byts 0 tot 16
    unsigned char Slave_data_byte[EBUSD_DATA_FIELD_LENGTH];
    unsigned char Slave_CRC;//de CRC
    unsigned char ACK;//0x00 = ok, 0xff = niet ok

};

extern struct EBUSD_telegram EBUSD_telegram_master;

unsigned long ebusd_To_CAN_id();
//unsigned char test_ebusd_crc(unsigned char crc, unsigned char data, unsigned char Polynomial);
unsigned char ebusd_crc(unsigned char crc, unsigned char data);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // EBUSD_H
