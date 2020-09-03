#ifndef _IoT_CHECKSUM_H_  //Added by ABM
#define _IoT_CHECKSUM_H_  //Added by ABM

#define POLY 0x8408

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Added by Austin McElroy
 *
 * @author Isak Jonsson (https://people.cs.umu.se/isak/)
 * @fn unsigned short crc16(char*, unsigned short)
 * @brief
 *
 *                                   16   12   5
this is the CCITT CRC 16 polynomial X  + X  + X  + 1.
This works out to be 0x1021, but the way the algorithm works
lets us use 0x8408 (the reverse of the bit pattern).  The high
bit is always assumed to be set, thus we only use 16 bits to
represent the 17 bit value.
 *
 * @pre
 * @post
 * @param data_p
 * @param length
 */


unsigned short IoT_Checksum_CRC16(char *data_p, unsigned short length);

#ifdef __cplusplus
}
#endif

#endif 
