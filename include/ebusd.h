#ifndef EBUSD_H
#define EBUSD_H

//includes


#ifdef __cplusplus
extern "C"
{
#endif

#define  EBUSD_DATA_FIELD_LENGTH 17

#define EBUSD_SYN       0xaa

//0xAP
//--->A master adres
//--->P prioriteidklasse
#define EBUSD_MASTER_01 0x00
#define EBUSD_MASTER_02 0x10 //--
#define EBUSD_MASTER_03 0x30
#define EBUSD_MASTER_04 0x70
#define EBUSD_MASTER_05 0xf0
#define EBUSD_MASTER_06 0x01
#define EBUSD_MASTER_07 0x11
#define EBUSD_MASTER_08 0x31
#define EBUSD_MASTER_09 0x71 //--
#define EBUSD_MASTER_10 0xf1
#define EBUSD_MASTER_11 0x03 //--
#define EBUSD_MASTER_12 0x13
#define EBUSD_MASTER_13 0x33
#define EBUSD_MASTER_14 0x73
#define EBUSD_MASTER_15 0xf3
#define EBUSD_MASTER_16 0x07
#define EBUSD_MASTER_17 0x17
#define EBUSD_MASTER_18 0x37
#define EBUSD_MASTER_19 0x77
#define EBUSD_MASTER_20 0xf7
#define EBUSD_MASTER_21 0x0f
#define EBUSD_MASTER_22 0x1f
#define EBUSD_MASTER_23 0x3f
#define EBUSD_MASTER_24 0x7f
#define EBUSD_MASTER_25 0xff

#define EBUSD_ACK_ok    0x00
#define EBUSD_ACK_N_ok  0xff
//Command request symbols (from ebusd to interface)
#define EBUSD_INIT      0x00
#define EBUSD_SEND      0x01
#define EBUSD_START     0x02
#define EBUSD_INFO      0x03


typedef struct EBUSD_telegram
{
    unsigned char QQ;//master adres max 25 verscillend adressen
    unsigned char ZZ;//bestemmings adres max 254 verscillend adressen
    unsigned char PB;//primaire commando max 254 verscillend niet 0xaa en 0xa9
    unsigned char SB;//sekudeer commando max 254 verscillend niet 0xaa en 0xa9
    unsigned char NN;//aantal data byts 0 tot 16
    unsigned char data_byte[EBUSD_DATA_FIELD_LENGTH];
    unsigned char CRC;//de CRC
    unsigned char Slave_ACK;//0x00 = ok, 0xff = niet ok
    unsigned char Slave_NN;//aantal data byts 0 tot 16
    unsigned char Slave_data_byte[EBUSD_DATA_FIELD_LENGTH];
    unsigned char Slave_CRC;//de CRC
    unsigned char ACK;//0x00 = ok, 0xff = niet ok

};

struct EBUSD_telegram EBUSD_telegram_master;

#ifdef __cplusplus
} // extern "C"
#endif
#endif // EBUSD_H