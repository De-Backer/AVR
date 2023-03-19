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
#define EBUSD_MASTER_01 0x00 //CAN id 0x00 slave 05
#define EBUSD_MASTER_02 0x10 //CAN id 0x01 slave 15 VRC 700/5  (thermostaat)
#define EBUSD_MASTER_03 0x30 //CAN id 0x02 slave 35
#define EBUSD_MASTER_04 0x70 //CAN id 0x03 slave 75
#define EBUSD_MASTER_05 0xf0 //CAN id 0x04 slave f5
#define EBUSD_MASTER_06 0x01 //CAN id 0x05 slave 06
#define EBUSD_MASTER_07 0x11 //CAN id 0x06 slave 16
#define EBUSD_MASTER_08 0x31 //CAN id 0x07 slave 36
#define EBUSD_MASTER_09 0x71 //CAN id 0x08 slave 76 VWZ AI VML X/2 A (externe modulle warmtepomp)
#define EBUSD_MASTER_10 0xf1 //CAN id 0x09 slave f6
#define EBUSD_MASTER_11 0x03 //CAN id 0x0A slave 08 VML 85/3 A 230 V (warmtepomp)
#define EBUSD_MASTER_12 0x13 //CAN id 0x0b slave 18
#define EBUSD_MASTER_13 0x33 //CAN id 0x0c slave 38
#define EBUSD_MASTER_14 0x73 //CAN id 0x0d slave 78
#define EBUSD_MASTER_15 0xf3 //CAN id 0x0e slave f8
#define EBUSD_MASTER_16 0x07 //CAN id 0x0f slave 0c
#define EBUSD_MASTER_17 0x17 //CAN id 0x10 slave 1c
#define EBUSD_MASTER_18 0x37 //CAN id 0x11 slave 3c
#define EBUSD_MASTER_19 0x77 //CAN id 0x12 slave 7c
#define EBUSD_MASTER_20 0xf7 //CAN id 0x13 slave fc
#define EBUSD_MASTER_21 0x0f //CAN id 0x14 slave 14
#define EBUSD_MASTER_22 0x1f //CAN id 0x15 slave 24
#define EBUSD_MASTER_23 0x3f //CAN id 0x16 slave 44
#define EBUSD_MASTER_24 0x7f //CAN id 0x17 slave 84
#define EBUSD_MASTER_25 0xff //CAN id 0x18 slave 04

#define EBUSD_MASTER_ZZ_01 0x05 //CAN id 0x00 slave 05
#define EBUSD_MASTER_ZZ_02 0x15 //CAN id 0x01 slave 15 VRC 700/5  (thermostaat)
#define EBUSD_MASTER_ZZ_03 0x35 //CAN id 0x02 slave 35
#define EBUSD_MASTER_ZZ_04 0x75 //CAN id 0x03 slave 75
#define EBUSD_MASTER_ZZ_05 0xf5 //CAN id 0x04 slave f5
#define EBUSD_MASTER_ZZ_06 0x06 //CAN id 0x05 slave 06
#define EBUSD_MASTER_ZZ_07 0x16 //CAN id 0x06 slave 16
#define EBUSD_MASTER_ZZ_08 0x31 //CAN id 0x07 slave 36
#define EBUSD_MASTER_ZZ_09 0x76 //CAN id 0x08 slave 76 VWZ AI VML X/2 A (externe modulle warmtepomp)
#define EBUSD_MASTER_ZZ_10 0xf6 //CAN id 0x09 slave f6
#define EBUSD_MASTER_ZZ_11 0x08 //CAN id 0x0A slave 08 VML 85/3 A 230 V (warmtepomp)
#define EBUSD_MASTER_ZZ_12 0x18 //CAN id 0x0b slave 18
#define EBUSD_MASTER_ZZ_13 0x38 //CAN id 0x0c slave 38
#define EBUSD_MASTER_ZZ_14 0x78 //CAN id 0x0d slave 78
#define EBUSD_MASTER_ZZ_15 0xf8 //CAN id 0x0e slave f8
#define EBUSD_MASTER_ZZ_16 0x0c //CAN id 0x0f slave 0c
#define EBUSD_MASTER_ZZ_17 0x1c //CAN id 0x10 slave 1c
#define EBUSD_MASTER_ZZ_18 0x3c //CAN id 0x11 slave 3c
#define EBUSD_MASTER_ZZ_19 0x7c //CAN id 0x12 slave 7c
#define EBUSD_MASTER_ZZ_20 0xfc //CAN id 0x13 slave fc
#define EBUSD_MASTER_ZZ_21 0x14 //CAN id 0x14 slave 14
#define EBUSD_MASTER_ZZ_22 0x24 //CAN id 0x15 slave 24
#define EBUSD_MASTER_ZZ_23 0x44 //CAN id 0x16 slave 44
#define EBUSD_MASTER_ZZ_24 0x84 //CAN id 0x17 slave 84
#define EBUSD_MASTER_ZZ_25 0x04 //CAN id 0x18 slave 04


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
    //10 = VRC 700/5 (thermostaat)
    //71 = VWZ AI VML X/2 A (externe modulle warmtepomp)
    //03 = VML 85/3 A 230 V (warmtepomp)
    unsigned char ZZ;
    //bestemmings adres max 228 verschillend adressen
    // master adresen and A9, AA mogen niet!
    //08 = VML 85/3 A 230 V (warmtepomp)
    //15 ~ VRC 700/5 (thermostaat) <= https://ebus-wiki.org/lib/exe/fetch.php/ebus/spec_prot_7_v1_6_1_anhang_ausgabe_1.pdf
    //52 = V71
    //76 = VWZ AI VML X/2 A (externe modulle warmtepomp)
    //E8 = Slave 190 VML 85/3 A 230 V (warmtepomp)
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

/* VML 85/3 A 230 V (warmtepomp) <= is de EBUS voeding
 * master adres 03
 * slave  adres 08
 * in de warmtepomp zit een extra Slave 190
 * slave  adres E8
 *
 * als enig op de bus: (er zijn dus meerder Ebus-deelnemers in de warmtepomp)
 * CAN id 0x0AE8b512#f0 0b0000000000
 * CAN id 0x0AE8b512#f3 00000000001ffff
 * CAN id 0x0AE8b512#f4 ffffff00
 *
 * master 0b0000000000
 * slave  00000000001fff ffffff00
 *
 * bij opstart 3 maal CAN id 0x0AE8b512
 * master 0c01000000002b 006401
 * slave  01
 * dan
 * master 0b0000000000
 * slave  00004f0101ffff ffffff00
 * dan
 * master 0b0000000000
 * slave  4e01000001ffff ffffff00
 * dan blijft dit om de 10 sec herhalen
 * master 0b0000000000
 * slave  0000000001ffff ffffff00
*/

/* VRC 700/5 (thermostaat)
 * master adres 10
 * slave  adres 15
 * met VML 85/3 A 230 V en WW op de bus:
 * CAN id 0x0108b504
 * CAN id 0x0108b511
 * CAN id 0x0108b510
 * CAN id 0x01feb516
 * CAN id 0x01feb505
 * CAN id 0x010ab505
 * CAN id 0x0108b507
 * CAN id 0x010ab504
 * CAN id 0x0108b512
 * CAN id 0x0108b513
*/
/* VWZ AI VML X/2 A (externe modulle warmtepomp)
 * master adres 71
 * slave  adres 76
 * CAN id 0x0808b507
 * CAN id 0x0808b511
 * CAN id 0x0808b51a
*/

/* V71
 * slave  adres 52
*/
