/* Arduino RDS/RBDS (IEC 62016/NRSC-4-B) Decoding Library
 * See the README file for author and licensing information. In case it's
 * missing from your distribution, use the one here as the authoritative
 * version: https://github.com/csdexter/RDSDecoder/blob/master/README
 *
 * This library is for decoding RDS/RBDS data streams (groups).
 * See the example sketches to learn how to use the library in your code.
 *
 * This is the main include file for the library.
 */

#ifndef _RDSDECODER_H_INCLUDED
#define _RDSDECODER_H_INCLUDED

#if defined(ARDUINO) && ARDUINO >= 100
# include <Arduino.h>
#else
# include <WProgram.h>
#endif

//Define the Locale options
#define RDS_LOCALE_US 0
#define RDS_LOCALE_EU 1

//Define RDS-related public flags (that may come handy to the user)
#define RDS_DI_STEREO 0x01
#define RDS_DI_ARTIFICIAL_HEAD 0x02
#define RDS_DI_COMPRESSED 0x04
#define RDS_DI_DYNAMIC_PTY 0x08
#define RDS_PI_AREA_LOCAL 0x00
#define RDS_PI_AREA_INTERNATIONAL 0x01
#define RDS_PI_AREA_NATIONAL 0x02
#define RDS_PI_AREA_SUPRAREGIONAL 0x03
#define RDS_PI_AREA_REGIONAL_FIRST 0x04
#define RDS_AF_FILLER 0xCD
#define RDS_AF_NODATA 0xE0
#define RDS_AF_FOLLOWS_FM_FIRST 0xE1
#define RDS_AF_FOLLOWS_AM 0xFA

//RDS Decoder callback types
#define RDS_CALLBACK_AF 0x00
#define RDS_CALLBACK_TDC 0x01
#define RDS_CALLBACK_AID 0x02
#define RDS_CALLBACK_ODA 0x03
#define RDS_CALLBACK_EON 0x04
#define RDS_CALLBACK_LAST 0x05

//This holds time of day as received via RDS. Mimicking struct tm from
//<time.h> for familiarity.
//NOTE: RDS does not provide seconds, only guarantees that the minute update
//      will occur close enough to the seconds going from 59 to 00 to be
//      meaningful -- so we don't provide tm_sec
//NOTE: RDS does not provide DST information so we don't provide tm_isdst
//NOTE: we will provide tm_wday (day of week) but not tm_yday (day of year)
typedef struct {
    byte tm_min;
    byte tm_hour;
    byte tm_mday;
    byte tm_mon;
    word tm_year;
    byte tm_wday;
    int8_t tm_tz;
} TRDSTime;

typedef struct __attribute__ ((__packed__)) {
  uint8_t country:4;
  uint8_t area:4;
  byte program;
} TRDSPI;

typedef struct __attribute__ ((__packed__)) {
  uint8_t day:5;
  uint8_t hour:5;
  uint8_t minute:6;
} TRDSPIN;

typedef struct __attribute__ ((__packed__)) {
  uint8_t romClassNumber:5;
  uint16_t serialNumber:10;
  uint8_t scopeFlag:1;
} TRDSIRDSMessage;

typedef struct __attribute__ ((__packed__)) {
  uint8_t variantCode:2;
  union {
    struct {
      uint8_t unused:2;
      uint8_t locationTableNumber:6;
      uint8_t alternateFrequencyIndicator:1;
      uint8_t mode:1;
      uint8_t international:1;
      uint8_t national:1;
      uint8_t regional:1;
      uint8_t urban:1;
    };
    struct {
      uint8_t gapParameter:2;
      uint8_t serviceIdentifier:6;
      uint8_t activityTime:2;
      uint8_t windowTime:2;
      uint8_t delayTime:2;
    };
  };
} TRDSTMCMessage;

typedef struct {
  word applicationIdentification;
  word message;
  byte carriedInGroup;
} TRDSAppID;

typedef struct __attribute__ ((__packed__)) {
  uint8_t linkageActuator:1;
  uint8_t extendedGeneric:1;
  uint8_t internationalLinkageSet:1;
  uint16_t linkageSet:12;
} TRDSLinkageInformation;

typedef struct {
    union {
        word programIdentifier;
        TRDSPI programIdentifierEBU;
    };
    bool TP, TA;
    byte PTY;
    char programService[9];
    union {
      word programItemNumber;
      TRDSPIN PIN;
    };
    TRDSLinkageInformation linkageInformation;
} TRDSEON;

typedef struct {
    union {
        word programIdentifier;
        TRDSPI programIdentifierEBU;
    };
    bool TP, TA, MS;
    byte PTY, DICC;
    char programService[9];
    char programTypeName[9];
    char radioText[65];
    union {
        word programItemNumber;
        TRDSPIN PIN;
    };
    bool linkageActuator;
    byte pagingOperatorCode;
    byte extendedCountryCode;
    byte languageCode;
    word tmcIdentification;
    word pagingIdentification;
    TRDSAppID IRDS;
    TRDSAppID TMC;
    TRDSEON EON;
} TRDSData;

//RDS Decoder callback prototype.
//In general, the first argument is the semantic equivalent of the segment
//address, the second is true if this was an A group and the third parameter
//contains meaningful data; while the last two parameters are data from the
//particular RDS group blocks C and D.
//RDS_CALLBACK_AF:
//    First parameter always 0x00, second always true, third contains one pair
//    of AF codes, as presented in a 0A group.
//RDS_CALLBACK_TDC:
//    First parameter contains the segment address, second true if this was a
//    5A group, third and fourth contain data in blocks C and D of source group.
//RDS_CALLBACK_AID:
//    First parameter contains the carried in group, second always true, third
//    and fourth contain the application message and AID.
//RDS_CALLBACK_ODA:
//    First parameter contains the first 5 bits of ODA data, second is true if
//    data comes from an A group, third and fourth contain the remaining 32 bits
//    of ODA data.
//RDS_CALLBACK_EON:
//    First parameter is 1 if this is an AF pair (variant 4), 2 if this is a
//    mapped FM frequency pair (variants 5-8) or 3 if this is a mapped AM
//    frequency pair (variant 9); second is always true, third contains the
//    frequency pair and the fourth is always zero.
typedef void (*TRDSCallback)(byte, bool, word, word);

class RDSDecoder
{
    public:
        /*
        * Description:
        *   Default constructor.
        */
        RDSDecoder() { resetRDS(); }

        /*
        * Description:
        *   Registers a new callback of the given type, which is one of the
        *   RDS_CALLBACK_* constants. The second parameter is the
        *   callback. Using NULL for the second parameter causes the currently
        *   registered callback of this type (if any) to be removed.
        */
        void registerCallback(byte type, TRDSCallback callback = NULL);

        /*
        * Description:
        *   Decodes one RDS group and updates internal data structures.
        */
        void decodeRDSGroup(word block[]);

        /*
        * Description:
        *   Returns currently decoded RDS data, filling a struct
        *   RDS_Data.
        */
        void getRDSData(TRDSData* rdsdata);

        /*
        * Description:
        *   Returns currently decoded RDS CT information filling a struct
        *   TRDSTime, if any is available, and returns true; otherwise
        *   returns false and does not touch rdstime.
        * Parameters:
        *   rdstime - pointer to a struct RDS_Time to be filled with
        *             CT information, ignore if only interested in CT
        *             availability and not actual value.
        */
        bool getRDSTime(TRDSTime* rdstime = NULL);

        /*
        * Description:
        *   Resets internal data structures, use when switching to a new
        *   station.
        */
        void resetRDS(void);

    private:
        TRDSData _status;
        TRDSTime _time;
        bool _rdstextab, _rdsptynab, _havect;
        TRDSCallback _callbacks[RDS_CALLBACK_LAST];

        /*
        * Description:
        *   Filters the string str in place to only contain printable
        *   characters and also replaces 0x0D (CR) with 0x00 effectively
        *   ending the string at that point as per RDBS §3.1.5.3.
        *   Any unprintable character is converted to a question mark ("?"),
        *   as is customary. This helps with filtering out noisy strings.
        */
        void makePrintable(char* str);

        /*
        * Description:
        *   When treating word values as two characters, RDS and AVR have a
        *   problem because RDS uses big endian semantics while the AVR is
        *   little endian. Hence this function to translate between the two
        *   worlds.
        * Parameters:
        *   value - the word to be switched
        */
        inline word swab(word value) { return (value >> 8) | (value << 8); }
};

class RDSTranslator
{
    public:
        /*
        * Description:
        *   Translates the given PTY into human-readable text for the given
        *   locale. At most textsize-1 characters will be copied to the buffer
        *   at text.
        */
        void getTextForPTY(byte PTY, byte locale, char* text, byte textsize);

        /*
        * Description:
        *   Translates the given PTY between the given locales.
        */
        byte translatePTY(byte PTY, byte fromlocale, byte tolocale);

        /*
        * Description:
        *   Decodes the station callsign out of the PI using the method
        *   defined in the RDBS standard for North America. If not under RDBS,
        *   you don't need to call this as you can directly access the decoded
        *   version via the fields of RDS_Data.programIdentifierEBU.
        * 
        * Parameters:
        *   programIdentifier - a word containing the Program Identifier value
        *                       from RDBS
        *   callSign - pointer to a char[] at least 5 characters long that
        *              receives the decoded station call sign
        */
        void decodeCallSign(word programIdentifier, char* callSign);

        /*
        * Description:
        *   Translates an RDS-TMC event distance code into a human readable
        *   measurement in km. Returns 0xFF to signal "more than 100km".
        */
        byte decodeTMCDistance(byte length);

        /*
        * Description:
        *   Translates an RDS-TMC event duration code into a human readable
        *   representation.
        */
        void decodeTMCDuration(byte length, TRDSTime* tmctime);

        /*
        * Description:
        *   Translates an AF frequency code into a human readable measurement
        *   in kHz (or tens of kHz if FM is true), valid for the given locale.
        */
        word decodeAFFrequency(byte AF, bool FM = true,
                               byte locale = RDS_LOCALE_EU);

        /*
        * Description:
        *   Translates a timezone value to a human readable measurement in
        *   minutes.
        */
        int16_t decodeTZValue(int8_t tz);
};

#endif