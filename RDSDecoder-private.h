/* Arduino RDS/RBDS (IEC 62016/NRSC-4-B) Decoding Library
 * See the README file for author and licensing information. In case it's
 * missing from your distribution, use the one here as the authoritative
 * version: https://github.com/csdexter/RDSDecoder/blob/master/README
 *
 * This library is for decoding RDS/RBDS data streams (groups).
 * See the example sketches to learn how to use the library in your code.
 *
 * This file contains definitions that are only used internally and shouldn't be
 * needed by most users.
 */

#ifndef _RDSDECODER_PRIVATE_H_INCLUDED
#define _RDSDECODER_PRIVATE_H_INCLUDED

//Define RDS block B decoding masks
#define RDS_TYPE_MASK 0xF800
#define RDS_TYPE_SHR 11
#define RDS_TP 0x0400
#define RDS_PTY_MASK 0x03E0
#define RDS_PTY_SHR 5
#define RDS_TA word(0x0010)
#define RDS_MS word(0x0008)
#define RDS_DI word(0x0004)
#define RDS_DIPS_ADDRESS word(0x0003)
#define RDS_TEXTAB word(0x0010)
#define RDS_TEXT_ADDRESS word(0x000F)
#define RDS_PTYNAB word(0x0010)
#define RDS_PTYN_ADDRESS word(0x0001)

//Define RDS SL (group 1A) decoding values
#define RDS_SLABEL_TYPE_PAGINGECC 0x00
#define RDS_SLABEL_TYPE_TMCID 0x01
#define RDS_SLABEL_TYPE_PAGINGID 0x02
#define RDS_SLABEL_TYPE_LANGUAGE 0x03
#define RDS_SLABEL_TYPE_INHOUSE 0x06
#define RDS_SLABEL_TYPE_EWSID 0x07
#define RDS_SLABEL_LA 0x8000
#define RDS_SLABEL_MASK 0x7000
#define RDS_SLABEL_SHR 12
#define RDS_SLABEL_VALUE_MASK 0x0FFF

//Define RDS ODA (group 3A) values and decoding masks
#define RDS_AID_DEFAULT 0x0000
#define RDS_AID_ERT 0x6552
#define RDS_AID_RTPLUS 0x4BD7
#define RDS_AID_IRDS 0xC563
#define RDS_AID_TMC 0xCD46 // 0xCD45 and 0xCD47 also seen in the wild
#define RDS_ODA_GROUP_MASK 0x1F

//Define RDS CT (group 4A) decoding masks
#define RDS_TIME_TZ_MASK 0x0000001FUL
#define RDS_TIME_TZ_SIGN 0x00000020UL
#define RDS_TIME_MINUTE_MASK 0x00000FC0UL
#define RDS_TIME_MINUTE_SHR 6
#define RDS_TIME_HOUR_MASK 0x0001F000UL
#define RDS_TIME_HOUR_SHR 12
#define RDS_TIME_MJD2_MASK 0xFFFE0000UL
#define RDS_TIME_MJD2_SHR 17
#define RDS_TIME_MJD1_MASK word(0x0003)
#define RDS_TIME_MJD1_SHL 15

//Define RDS EON (group 14A) decoding masks
#define RDS_EON_TP word(0x0010)
#define RDS_EON_MASK word(0x000F)
#define RDS_EON_PTY_MASK 0xF800
#define RDS_EON_PTY_SHR 11
#define RDS_EON_TA_A word(0x0001)
#define RDS_EON_TA_B word(0x0008)
#define RDS_EON_TYPE_PS_SA0 0x00
#define RDS_EON_TYPE_PS_SA1 0x01
#define RDS_EON_TYPE_PS_SA2 0x02
#define RDS_EON_TYPE_PS_SA3 0x03
#define RDS_EON_TYPE_AF 0x04
#define RDS_EON_TYPE_MF_FM0 0x05
#define RDS_EON_TYPE_MF_FM1 0x06
#define RDS_EON_TYPE_MF_FM2 0x07
#define RDS_EON_TYPE_MF_FM3 0x08
#define RDS_EON_TYPE_MF_AM 0x09
#define RDS_EON_TYPE_LINKAGE 0x0C
#define RDS_EON_TYPE_PTYTA 0x0D
#define RDS_EON_TYPE_PIN 0x0E
#define RDS_EON_TYPE_INHOUSE 0x0F

//Define RDS group types
#define RDS_GROUP_0A 0x00
#define RDS_GROUP_0B 0x01
#define RDS_GROUP_1A 0x02
#define RDS_GROUP_1B 0x03
#define RDS_GROUP_2A 0x04
#define RDS_GROUP_2B 0x05
#define RDS_GROUP_3A 0x06
#define RDS_GROUP_3B 0x07
#define RDS_GROUP_4A 0x08
#define RDS_GROUP_4B 0x09
#define RDS_GROUP_5A 0x0A
#define RDS_GROUP_5B 0x0B
#define RDS_GROUP_6A 0x0C
#define RDS_GROUP_6B 0x0D
#define RDS_GROUP_7A 0x0E
#define RDS_GROUP_7B 0x0F
#define RDS_GROUP_8A 0x10
#define RDS_GROUP_8B 0x11
#define RDS_GROUP_9A 0x12
#define RDS_GROUP_9B 0x13
#define RDS_GROUP_10A 0x14
#define RDS_GROUP_10B 0x15
#define RDS_GROUP_11A 0x16
#define RDS_GROUP_11B 0x17
#define RDS_GROUP_12A 0x18
#define RDS_GROUP_12B 0x19
#define RDS_GROUP_13A 0x1A
#define RDS_GROUP_13B 0x1B
#define RDS_GROUP_14A 0x1C
#define RDS_GROUP_14B 0x1D
#define RDS_GROUP_15A 0x1E
#define RDS_GROUP_15B 0x1F

#endif
