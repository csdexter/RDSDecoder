/* Arduino RDS/RBDS (IEC 62016/NRSC-4-B) Decoding Library
 * See the README file for author and licensing information. In case it's
 * missing from your distribution, use the one here as the authoritative
 * version: https://github.com/csdexter/RDSDecoder/blob/master/README
 *
 * This library is for decoding RDS/RBDS data streams (groups).
 * See the example sketches to learn how to use the library in your code.
 *
 * This is the main code file for the library.
 * See the header file for better function documentation.
 */

#include "RDSDecoder.h"
#include "RDSDecoder-private.h"
#include "iso14819-2.h"

#include <string.h>

#if defined(__GNUC__)
# if defined(__i386__) || defined(__x86_64__)
#  define __STDC_FORMAT_MACROS
#  include <inttypes.h>
#  include <stdio.h>
#  define PROGMEM
#  define PGM_P const char *
#  define strncpy_P strncpy
#  define snprintf_P snprintf
#  define lowByte(x) (uint8_t)((x) & 0xFF)
#  define highByte(x) (uint8_t)(((x) >> 8) & 0xFF)
#  define pgm_read_byte(x) (uint8_t)(*x)
#  define pgm_read_word(x) (uint16_t)(*x)
#  define pgm_read_ptr(x) (void *)(*x)
# endif
#else
# warning Non-GNU compiler detected, you are on your own!
#endif

void RDSDecoder::registerCallback(byte type, TRDSCallback callback){
    if (type < sizeof(_callbacks) / sizeof(_callbacks[0]))
        _callbacks[type] = callback;
};

void RDSDecoder::decodeRDSGroup(word block[]){
    byte grouptype;
    word fourchars[2];

    _status.programIdentifier = block[0];
    grouptype = lowByte((block[1] & RDS_TYPE_MASK) >> RDS_TYPE_SHR);
    _status.TP = (bool)(block[1] & RDS_TP);
    _status.PTY = lowByte((block[1] & RDS_PTY_MASK) >> RDS_PTY_SHR);

    switch(grouptype){
        case RDS_GROUP_0A:
        case RDS_GROUP_0B:
        case RDS_GROUP_15B:
            byte DIPSA;
            word twochars;

            _status.TA = (bool)(block[1] & RDS_TA);
            _status.MS = (bool)(block[1] & RDS_MS);
            DIPSA = lowByte(block[1] & RDS_DIPS_ADDRESS);
            if(block[1] & RDS_DI)
                _status.DICC |= (0x1 << (3 - DIPSA));
            else
                _status.DICC &= ~(0x1 << (3 - DIPSA));
            if(grouptype != RDS_GROUP_15B) {
                twochars = swab(block[3]);
                strncpy(&_status.programService[DIPSA * 2],
                        (char *)&twochars, 2);
            };
            if(grouptype == RDS_GROUP_0A) {
                if (_callbacks[RDS_CALLBACK_AF])
                    _callbacks[RDS_CALLBACK_AF](0x00, true, block[2], 0x00);
            }
            break;
        case RDS_GROUP_1A:
            _status.linkageActuator = (bool)(block[2] & RDS_SLABEL_LA);
            switch((block[2] & RDS_SLABEL_MASK) >> RDS_SLABEL_SHR) {
                case RDS_SLABEL_TYPE_PAGINGECC:
                    _status.extendedCountryCode = lowByte(block[2]);
                    _status.pagingOperatorCode = highByte(block[2]) & 0x0F;
                    break;
                case RDS_SLABEL_TYPE_TMCID:
                    _status.tmcIdentification = block[2] & RDS_SLABEL_VALUE_MASK;
                    break;
                case RDS_SLABEL_TYPE_PAGINGID:
                    _status.pagingIdentification = block[2] & RDS_SLABEL_VALUE_MASK;
                    break;
                case RDS_SLABEL_TYPE_LANGUAGE:
                    _status.languageCode = lowByte(block[2]);
                    break;
            };
        case RDS_GROUP_1B:
            _status.programItemNumber = block[3];
            break;
        case RDS_GROUP_2A:
        case RDS_GROUP_2B:
            byte RTA, RTAW;

            if((bool)(block[1] & RDS_TEXTAB) != _rdstextab) {
                if (_callbacks[RDS_CALLBACK_RT])
                    _callbacks[RDS_CALLBACK_RT](0x00,
                                                (grouptype == RDS_GROUP_2A),
                                                0x00, 0x00);
                _rdstextab = !_rdstextab;
                memset(_status.radioText, ' ', sizeof(_status.radioText) - 1);
            }
            RTA = lowByte(block[1] & RDS_TEXT_ADDRESS);
            RTAW = (grouptype == RDS_GROUP_2A) ? 4 : 2;
            fourchars[0] = swab(block[(grouptype == RDS_GROUP_2A) ? 2 : 3]);
            if(grouptype == RDS_GROUP_2A)
                fourchars[1] = swab(block[3]);
            strncpy(&_status.radioText[RTA * RTAW], (char *)fourchars, RTAW);
            break;
        case RDS_GROUP_3A:
            switch(block[3]){
                case RDS_AID_DEFAULT:
                    if ((block[1] & RDS_ODA_GROUP_MASK) == RDS_GROUP_8A) {
                      //Default use of Group 8A is TMC, so act as if we saw an
                      //explicit mapping of TMC's AID to Group 8A.
                      _status.TMC.carriedInGroup = RDS_GROUP_8A;
                      _status.TMC.message = block[2];
                    };
                    break;
                case RDS_AID_ERT:
                    _status.ERT.carriedInGroup = block[1] & RDS_ODA_GROUP_MASK;
                    _status.ERT.message = block[2];
                    break;
                case RDS_AID_RTPLUS:
                    _status.RTP.carriedInGroup = block[1] & RDS_ODA_GROUP_MASK;
                    _status.RTP.message = block[2];
                    break;
                case RDS_AID_IRDS:
                    _status.IRDS.carriedInGroup = block[1] & RDS_ODA_GROUP_MASK;
                    _status.IRDS.message = block[2];
                    break;
                case RDS_AID_TMC:
                    _status.TMC.carriedInGroup = block[1] & RDS_ODA_GROUP_MASK;
                    _status.TMC.message = block[2];
                    break;
            };
            if (_callbacks[RDS_CALLBACK_AID])
                _callbacks[RDS_CALLBACK_AID](block[1] & RDS_ODA_GROUP_MASK,
                                             true, block[2], block[3]);

            break;
        case RDS_GROUP_3B:
        case RDS_GROUP_4B:
        case RDS_GROUP_6A:
        case RDS_GROUP_6B:
        case RDS_GROUP_7B:
        case RDS_GROUP_8B:
        case RDS_GROUP_9B:
        case RDS_GROUP_10B:
        case RDS_GROUP_11A:
        case RDS_GROUP_11B:
        case RDS_GROUP_12A:
        case RDS_GROUP_12B:
        case RDS_GROUP_13B:
            if (_callbacks[RDS_CALLBACK_ODA])
                _callbacks[RDS_CALLBACK_ODA](
                    block[1] & RDS_ODA_GROUP_MASK, ! (grouptype & 0x01),
                    ((grouptype & 0x01) ? 0x00 : block[2]), block[3]);

            break;
        case RDS_GROUP_4A:
            unsigned long MJD, CT, ys;
            word yp;
            byte k, mp;

            CT = ((unsigned long)block[2] << 16) | block[3];
            //The standard mandates that CT must be all zeros if no time
            //information is being provided by the current station.
            if(!CT) break;

            _havect = true;
            MJD = (unsigned long)(block[1] & RDS_TIME_MJD1_MASK) <<
                  RDS_TIME_MJD1_SHL;
            MJD |= (CT & RDS_TIME_MJD2_MASK) >> RDS_TIME_MJD2_SHR;

            _time.tm_hour = (CT & RDS_TIME_HOUR_MASK) >> RDS_TIME_HOUR_SHR;
            _time.tm_tz = CT & RDS_TIME_TZ_MASK;
            if (CT & RDS_TIME_TZ_SIGN)
              _time.tm_tz = - _time.tm_tz;
            _time.tm_min = (CT & RDS_TIME_MINUTE_MASK) >> RDS_TIME_MINUTE_SHR;
            //Use integer arithmetic at all costs, Arduino lacks an FPU
            yp = (MJD * 10 - 150782) * 10 / 36525;
            ys = yp * 36525 / 100;
            mp = (MJD * 10 - 149561 - ys * 10) * 1000 / 306001;
            _time.tm_mday = MJD - 14956 - ys - mp * 306001 / 10000;
            k = (mp == 14 || mp == 15) ? 1 : 0;
            _time.tm_year = 1900 + yp + k;
            _time.tm_mon = mp - 1 - k * 12;
            _time.tm_wday = (MJD + 2) % 7 + 1;
            break;
        case RDS_GROUP_5A:
        case RDS_GROUP_5B:
            if (_callbacks[RDS_CALLBACK_TDC])
                _callbacks[RDS_CALLBACK_TDC](
                    block[1] & RDS_ODA_GROUP_MASK,
                    (grouptype == RDS_GROUP_5A),
                    ((grouptype == RDS_GROUP_5A) ? block[2] : 0x00),
                    block[3]);
            break;
        case RDS_GROUP_7A:
            //TODO: read the standard and do Radio Paging
            break;
        case RDS_GROUP_8A:
            if (_callbacks[RDS_CALLBACK_TMC])
                _callbacks[RDS_CALLBACK_TMC](
                    block[1] & RDS_ODA_GROUP_MASK, true, block[2], block[3]);
            break;
        case RDS_GROUP_9A:
            //NOTE: EWS is defined per-country which is a polite way of saying
            //      there is no standard and it's never going to work. Pity!
            break;
        case RDS_GROUP_10A:
            if((block[1] & RDS_PTYNAB) != _rdsptynab) {
                _rdsptynab = !_rdsptynab;
                memset(_status.programTypeName, ' ', 8);
            }
            fourchars[0] = swab(block[2]);
            fourchars[1] = swab(block[3]);
            strncpy(&_status.programTypeName[(block[1] & RDS_PTYN_ADDRESS) * 4],
                    (char *)&fourchars, 4);
            break;
        case RDS_GROUP_13A:
            //TODO: read the standard and do Enhanced Radio Paging
            break;
        case RDS_GROUP_14A:
            switch(block[1] & RDS_EON_MASK){
                case RDS_EON_TYPE_PS_SA0:
                case RDS_EON_TYPE_PS_SA1:
                case RDS_EON_TYPE_PS_SA2:
                case RDS_EON_TYPE_PS_SA3:
                    twochars = swab(block[2]);
                    strncpy(
                        &_status.EON.programService[(block[1] & RDS_EON_MASK) * 2],
                        (char *)&twochars, 2);
                    break;
                case RDS_EON_TYPE_AF:
                    if (_callbacks[RDS_CALLBACK_EON])
                        _callbacks[RDS_CALLBACK_EON](1, true, block[2], 0x00);
                    break;
                case RDS_EON_TYPE_MF_FM0:
                case RDS_EON_TYPE_MF_FM1:
                case RDS_EON_TYPE_MF_FM2:
                case RDS_EON_TYPE_MF_FM3:
                    if (_callbacks[RDS_CALLBACK_EON])
                        _callbacks[RDS_CALLBACK_EON](2, true, block[2], 0x00);
                    break;
                case RDS_EON_TYPE_MF_AM:
                    if (_callbacks[RDS_CALLBACK_EON])
                        _callbacks[RDS_CALLBACK_EON](3, true, block[2], 0x00);
                    break;
                case RDS_EON_TYPE_LINKAGE:
                    memcpy(&_status.EON.linkageInformation, &block[2],
                           sizeof(_status.EON.linkageInformation));
                    break;
                case RDS_EON_TYPE_PTYTA:
                    _status.EON.PTY = (block[2] & RDS_EON_PTY_MASK) >> RDS_EON_PTY_SHR;
                    _status.EON.TA = block[2] & RDS_EON_TA_A;
                    break;
                case RDS_EON_TYPE_PIN:
                    _status.EON.programItemNumber = block[2];
                    break;
            };
        case RDS_GROUP_14B:
            _status.EON.TP = block[1] & RDS_EON_TP;
            _status.EON.programIdentifier = block[3];
            if (grouptype == RDS_GROUP_14B)
                _status.EON.TA = block[1] & RDS_EON_TA_B;
               //TODO: implement PTY(ON): News/Weather/Alarm
            break;
        case RDS_GROUP_15A:
            //Withdrawn and currently unallocated, ignore
            break;
    }
}

void RDSDecoder::getRDSData(TRDSData* rdsdata){
    makePrintable(_status.programService);
    makePrintable(_status.programTypeName);
    makePrintable(_status.radioText);
    makePrintable(_status.EON.programService);

    *rdsdata = _status;
}

bool RDSDecoder::getRDSTime(TRDSTime* rdstime){
    if(_havect && rdstime) *rdstime = _time;

    return _havect;
}

void RDSDecoder::resetRDS(void){
    memset(&_status, 0x00, sizeof(_status));
    memset(_status.programService, ' ', sizeof(_status.programService) - 1);
    memset(_status.programTypeName, ' ', sizeof(_status.programTypeName) - 1);
    memset(_status.radioText, ' ', sizeof(_status.radioText) - 1);
    _rdstextab = false;
    _rdsptynab = false;
    _havect = false;
}

const char PROGMEM RDS2LCD_S[] = "\xE1\xE0\xE9\xE8\xED\xEE\xF3\xF2\xFA\xF9\xD1"
                                 "\xC7S\xDF\xA1J\xE2\xE4\xEA\xEB\xEE\xEF\xF4"
                                 "\xF6\xFB\xFC\xF1\xE7sgij\xAA\x90\xA9%Gen\xF6"
                                 "\x93""E\xA3$\x1B\x18\x1A\x19\xBA\xB9\xB2\xB3"
                                 "\xB1In\xFC\xB5\xBF\xF7\xB0\xBC\xBD\xBE\xA7"
                                 "\xC1\xC0\xC9\xC8\xCD\xCE\xD3\xD2\xDA\xD9RCSZ"
                                 "\xD0L\xC2\xC4\xCA\xCB\xCE\xCF\xD4\xD6\xDB\xDC"
                                 "rcsz\xF0l\xC3\xC5\xC6Oy\xDD\xD5""0\xDEGRCSZT"
                                 "\xF0\xE3\xE5\xE6ow\xF5""0\xFEgrcszt";

void RDSDecoder::makePrintable(char* str){
    for(byte i = 0; i < strlen(str); i++) {
        if(str[i] == 0x0D) {
            //CR ends the string, according to RDS §6.1.5.3
            str[i] = '\0';
            break;
        }
        // 0x24 is currency sign (U+00A4), not dollar sign
        // 0x5E is horizontal bar (quotation dash, U+2015), not caret
        // 0x60 is double vertical line (math norm symbol, U+2016), not backtick
        // 0x7E is overline (U+203E), not tilde
        // 0x80: a-acute, a-grave, e-acute, e-grave, i-acute, i-grave, o-acute,
        //       o-grave, u-acute, u-grave, N-tilde, C-cedilla, S-cedilla,
        //       scharfes-es, spanish-exclamation, dutch-IJ, a-circ, a-umlaut,
        //       e-circ, e-umlaut, i-circ, i-umlaut, o-circ, o-umlaut, u-circ,
        //       u-umlaut, n-tilde, c-cedilla, s-cedilla, g-breve,
        //       turkish-i-nodot, dutch-ij, a-superscript, alpha, (c), permille,
        //       G-breve, e-caron, n-caron, o-dprime, pi, EUR, GBP, USD, 
        //       arrow-left, arrow-up, arrow-right, arrow-down, o-superscript,
        //       1-superscript, 2-superscript, 3-superscript, +/-,
        //       turkish-I-dot, n-acute, u-dprime, miu, spanish-question,
        //       division, degree, 1/4, 1/2, 3/4, paragraph,
        //       A,E,I,O,U{acute,grave}, R,C,S,Z{caron}, D-line, L-dot,
        //       A,E,I,O,U{circ,umlaut}, r,c,s,z{caron}, d-line, l-dot,
        //       A-tilde, A-circle, AE, OE, y-circ, Y-acute, O-tilde, O-slash,
        //       Thorn, NG, R,C,S,Z{acute}, T-bar, th, a-tilde, a-circle, ae,
        //       oe, w-circ, o-tilde, o-slash, thorn, ng, r,c,s,z{acute}, t-bar
        if(str[i] == 0x0A || str[i] == 0x0B || str[i] == 0x1F)
            //LF, VT and US are allowed as a control characters. The first with
            //the same meaning as on UNIX, second as end-of-headline indicator
            //and third as soft-hyphen, according to RDS §6.1.5.3
            continue;
        //Any other control character is an undetected error on the receiving
        //side (because the manufacturers of the RDS decoder chip were too cheap
        //to properly implement the ECC in the standard).
        if(str[i] < 32) str[i] = '?';
        else if(str[i] == 0x24) str[i] = '\xA4';
        else if(str[i] == 0x5E) str[i] = '-';
        else if(str[i] == 0x60) str[i] = '\xA0';
        else if(str[i] == 0x7E) str[i] = '_';
        else if(str[i] >= 0x80)
            str[i] = (char)pgm_read_byte(&RDS2LCD_S[str[i] - 0x80]);
    }
}

const char PTY2Text_S_None[] PROGMEM = "None/Undefined";
const char PTY2Text_S_News[] PROGMEM = "News";
const char PTY2Text_S_Current[] PROGMEM = "Current affairs";
const char PTY2Text_S_Information[] PROGMEM = "Information";
const char PTY2Text_S_Sports[] PROGMEM = "Sports";
const char PTY2Text_S_Education[] PROGMEM = "Education";
const char PTY2Text_S_Drama[] PROGMEM = "Drama";
const char PTY2Text_S_Culture[] PROGMEM = "Culture";
const char PTY2Text_S_Science[] PROGMEM = "Science";
const char PTY2Text_S_Varied[] PROGMEM = "Varied";
const char PTY2Text_S_Pop[] PROGMEM = "Pop";
const char PTY2Text_S_Rock[] PROGMEM = "Rock";
const char PTY2Text_S_EasySoft[] PROGMEM = "Easy & soft";
const char PTY2Text_S_Classical[] PROGMEM = "Classical";
const char PTY2Text_S_Other[] PROGMEM = "Other music";
const char PTY2Text_S_Weather[] PROGMEM = "Weather";
const char PTY2Text_S_Finance[] PROGMEM = "Finance";
const char PTY2Text_S_Children[] PROGMEM = "Children's";
const char PTY2Text_S_Social[] PROGMEM = "Social affairs";
const char PTY2Text_S_Religion[] PROGMEM = "Religion";
const char PTY2Text_S_TalkPhone[] PROGMEM = "Talk & phone-in";
const char PTY2Text_S_Travel[] PROGMEM = "Travel";
const char PTY2Text_S_Leisure[] PROGMEM = "Leisure";
const char PTY2Text_S_Jazz[] PROGMEM = "Jazz";
const char PTY2Text_S_Country[] PROGMEM = "Country";
const char PTY2Text_S_National[] PROGMEM = "National";
const char PTY2Text_S_Oldies[] PROGMEM = "Oldies";
const char PTY2Text_S_Folk[] PROGMEM = "Folk";
const char PTY2Text_S_Documentary[] PROGMEM = "Documentary";
const char PTY2Text_S_EmergencyTest[] PROGMEM = "Emergency test";
const char PTY2Text_S_Emergency[] PROGMEM = "Emergency";
const char PTY2Text_S_Adult[] PROGMEM = "Adult hits";
const char PTY2Text_S_Top40[] PROGMEM = "Top 40";
const char PTY2Text_S_Nostalgia[] PROGMEM = "Nostalgia";
const char PTY2Text_S_RnB[] PROGMEM = "Rhythm and blues";
const char PTY2Text_S_Language[] PROGMEM = "Language";
const char PTY2Text_S_Personality[] PROGMEM = "Personality";
const char PTY2Text_S_Public[] PROGMEM = "Public";
const char PTY2Text_S_College[] PROGMEM = "College";
const char PTY2Text_S_Spanish[] PROGMEM = "Espanol";
const char PTY2Text_S_HipHop[] PROGMEM = "Hip hop";

PGM_P const PTY2Text_EU[32] PROGMEM = {
    PTY2Text_S_None,
    PTY2Text_S_News,
    PTY2Text_S_Current,
    PTY2Text_S_Information,
    PTY2Text_S_Sports,
    PTY2Text_S_Education,
    PTY2Text_S_Drama,
    PTY2Text_S_Culture,
    PTY2Text_S_Science,
    PTY2Text_S_Varied,
    PTY2Text_S_Pop,
    PTY2Text_S_Rock,
    PTY2Text_S_EasySoft,
    PTY2Text_S_Classical,
    PTY2Text_S_Classical,
    PTY2Text_S_Other,
    PTY2Text_S_Weather,
    PTY2Text_S_Finance,
    PTY2Text_S_Children,
    PTY2Text_S_Social,
    PTY2Text_S_Religion,
    PTY2Text_S_TalkPhone,
    PTY2Text_S_Travel,
    PTY2Text_S_Leisure,
    PTY2Text_S_Jazz,
    PTY2Text_S_Country,
    PTY2Text_S_National,
    PTY2Text_S_Oldies,
    PTY2Text_S_Folk,
    PTY2Text_S_Documentary,
    PTY2Text_S_EmergencyTest,
    PTY2Text_S_Emergency};

PGM_P const PTY2Text_US[32] PROGMEM = {
    PTY2Text_S_None,
    PTY2Text_S_News,
    PTY2Text_S_Information,
    PTY2Text_S_Sports,
    PTY2Text_S_TalkPhone,
    PTY2Text_S_Rock,
    PTY2Text_S_Rock,
    PTY2Text_S_Adult,
    PTY2Text_S_Rock,
    PTY2Text_S_Top40,
    PTY2Text_S_Country,
    PTY2Text_S_Oldies,
    PTY2Text_S_EasySoft,
    PTY2Text_S_Nostalgia,
    PTY2Text_S_Jazz,
    PTY2Text_S_Classical,
    PTY2Text_S_RnB,
    PTY2Text_S_RnB,
    PTY2Text_S_Language,
    PTY2Text_S_Religion,
    PTY2Text_S_Religion,
    PTY2Text_S_Personality,
    PTY2Text_S_Public,
    PTY2Text_S_College,
    PTY2Text_S_Spanish,
    PTY2Text_S_Spanish,
    PTY2Text_S_HipHop,
    PTY2Text_S_None,
    PTY2Text_S_None,
    PTY2Text_S_Weather,
    PTY2Text_S_EmergencyTest,
    PTY2Text_S_Emergency};

const byte PTY_EU2US[32] PROGMEM = {0, 1, 0, 2, 3, 23, 0, 0, 0, 0, 7, 5, 12, 15,
                                    15, 0, 29, 0, 0, 0, 20, 4, 0, 0, 14, 10, 0,
                                    11, 0, 0, 30, 31};
const byte PTY_US2EU[32] PROGMEM = {0, 1, 3, 4, 21, 11, 11, 10, 11, 10, 25, 27,
                                    12, 27, 24, 14, 15, 15, 0, 20, 20, 0, 0, 5,
                                    0, 0, 0, 0, 0, 16, 30, 31};

void RDSTranslator::getTextForPTY(byte PTY, byte locale, char* text,
                                    byte textsize){
    switch(locale){
        case RDS_LOCALE_US:
            strncpy_P(text, (PGM_P)(pgm_read_ptr(&PTY2Text_US[PTY])),
                      textsize);
            break;
        case RDS_LOCALE_EU:
            strncpy_P(text, (PGM_P)(pgm_read_ptr(&PTY2Text_EU[PTY])),
                    textsize);
            break;
    }
}

byte RDSTranslator::translatePTY(byte PTY, byte fromlocale, byte tolocale){
    if(fromlocale == tolocale) return PTY;
    else switch(fromlocale){
        case RDS_LOCALE_US:
            return pgm_read_byte(&PTY_US2EU[PTY]);
            break;
        case RDS_LOCALE_EU:
            return pgm_read_byte(&PTY_EU2US[PTY]);
            break;
    }

    //Never reached
    return 0;
}

const char PI2CallSign_S[] PROGMEM = "KEXKFHKFIKGAKGOKGUKGWKGYKIDKITKJRKLOKLZ"
                                     "KMAKMJKNXKOA---------KQVKSLKUJKVIKWG---"
                                     "---KYW---WBZWDZWEW---WGLWGNWGR---WHAWHB"
                                     "WHKWHO---WIPWJRWKYWLSWLW------WOC---WOL"
                                     "WOR---------WWJWWL------------------KDB"
                                     "KGBKOYKPQKSDKUTKXLKXO---WBTWGHWGYWHPWIL"
                                     "WMCWMTWOIWOWWRRWSBWSMKBWKCYKDF------KHQ"
                                     "KOB---------------------WISWJWWJZ------"
                                     "---WRC";

bool RDSTranslator::decodeCallSign(word programIdentifier, char* callSign){
    if (!callSign) return false;

    if (programIdentifier < 0x1000 ||
        (programIdentifier > 0x9EFF && programIdentifier < 0xA100) ||
        programIdentifier > 0xAFAF)
        return false;
    else if (programIdentifier >= 0x9950 && programIdentifier <= 0x9EFF) {
        if (programIdentifier <= 0x99B9) {
            programIdentifier -= 0x9950;
            char exists = (char)pgm_read_byte(&PI2CallSign_S[programIdentifier * 3]);
            if (exists != '-') {
                strncpy_P(callSign, &PI2CallSign_S[programIdentifier * 3], 3);
                callSign[3] = '\0';
                return true;
            } else
                return false;
        } else
            return false;
    };


    if ((programIdentifier & 0xFFF0) == 0xAFA0)
        programIdentifier <<= 12;
    else if ((programIdentifier & 0xFF00) == 0xAF00)
        programIdentifier <<= 8;
    else if ((programIdentifier & 0xF000) == 0xA000)
        programIdentifier = (programIdentifier & 0x0F00 << 4) |
                            (programIdentifier & 0x00FF);

    if (programIdentifier >= 0x54A8) {
        callSign[0] = 'W';
        programIdentifier -= 0x54A8;
    } else if (programIdentifier >= 0x1000) {
        callSign[0] = 'K';
        programIdentifier -= 0x1000;
    } else
        return false;

    callSign[1] = char(programIdentifier / 676);
    programIdentifier -= byte(callSign[1]) * 676;
    callSign[1] += 'A';
    callSign[2] = char(programIdentifier / 26);
    programIdentifier -= byte(callSign[2] * 26);
    callSign[2] += 'A';
    callSign[3] = programIdentifier + 'A';
    callSign[4] = '\0';

    return true;
}

void RDSTranslator::unpackEBUPI(word programIdentifier, TRDSPI *unpacked) {
    if(!unpacked) return;

    unpacked->country = (programIdentifier & RDS_PI_COUNTRY_MASK) >>
                        RDS_PI_COUNTRY_SHR;
    unpacked->area = (programIdentifier & RDS_PI_AREA_MASK) >> RDS_PI_AREA_SHR;
    unpacked->program = lowByte(programIdentifier);
};

void RDSTranslator::unpackPIN(word programItemNumber, TRDSTime *unpacked) {
    if(!unpacked) return;

    unpacked->tm_mday = (programItemNumber & RDS_PIN_DAY_MASK) >>
        RDS_PIN_DAY_SHR;
    unpacked->tm_hour = (programItemNumber & RDS_PIN_HOUR_MASK) >>
        RDS_PIN_HOUR_SHR;
    unpacked->tm_min = programItemNumber & RDS_PIN_MINUTE_MASK;
};

byte RDSTranslator::decodeTMCDistance(byte length) {
    if (length == 0) return 0xFF;
    else if (length > 0 && length <= 10) return length;
    else if (length > 10 && length <= 15) return 10 + (length - 10) * 2;
    else
    	return 20 + (length - 15) * 5;
}

void RDSTranslator::decodeTMCDuration(byte length, TRDSTime* tmctime) {
    if (!tmctime) return;
    else memset(tmctime, 0x00, sizeof(TRDSTime));

    if (length <= 95) {
        tmctime->tm_min = (length % 4) * 45;
        tmctime->tm_hour = length / 4;
    } else if (length > 95 && length <= 200) {
        tmctime->tm_hour = (length - 95) % 24;
        tmctime->tm_mday = (length - 95) / 24;
    } else if (length > 200 && length < 231) {
        tmctime->tm_mday = length - 200;
    } else if (length > 231) {
        // NOTE: according to RDS-TMC standard, this is expressed as half-month
        // intervals. Therefore, this function will output things like Feb 30th
        // with the understanding that the UI will render it appropriately.
        tmctime->tm_mday = ((length - 231) * 15) % 31;
        tmctime->tm_mon = ((length - 232) / 2) + 1;
    };
};

word RDSTranslator::decodeAFFrequency(byte AF, bool FM, byte locale) {
    if (FM) return (AF + 875) * 10;
    else if (AF < 16) return (AF - 1) * 9 + 153;
    else if (locale == RDS_LOCALE_US)
        return (AF - 16) * 10 + 530;
    else return (AF - 16) * 9 + 531;
};

int16_t RDSTranslator::decodeTZValue(int8_t tz) {
    return (tz / 2) * 60 + (tz % 2) * 30;
};

void RDSTranslator::unpackTMCMessage3(word tmcMessage,
                                      TRDSTMCMessage3 *unpacked) {
    if(!unpacked) return;
    else memset(unpacked, 0x00, sizeof(TRDSTMCMessage3));

    unpacked->variantCode = (tmcMessage & RDS_TMC_MESSAGE_VARIANT_MASK) >>
                            RDS_TMC_MESSAGE_VARIANT_SHR;
    switch(unpacked->variantCode) {
        case RDS_TMC_MESSAGE_VARIANT_LTN:
            unpacked->locationTableNumber = (
                tmcMessage & RDS_TMC_MESSAGE_LTN_MASK) >>
                RDS_TMC_MESSAGE_LTN_SHR;
            unpacked->alternateFrequencyIndicator = (bool)(
                tmcMessage & RDS_TMC_MESSAGE_AFI);
            unpacked->mode = (bool)(tmcMessage & RDS_TMC_MESSAGE_MODE);
            unpacked->international = (bool)(
                tmcMessage & RDS_TMC_MESSAGE_SCOPE_INTERNATIONAL);
            unpacked->national = (bool)(
                tmcMessage & RDS_TMC_MESSAGE_SCOPE_NATIONAL);
            unpacked->regional = (bool)(
                tmcMessage & RDS_TMC_MESSAGE_SCOPE_REGIONAL);
            unpacked->urban = (bool)(tmcMessage & RDS_TMC_MESSAGE_SCOPE_URBAN);
            break;
        case RDS_TMC_MESSAGE_VARIANT_SID:
            unpacked->gapParameter = (
                tmcMessage & RDS_TMC_MESSAGE_GAP_MASK) >>
                RDS_TMC_MESSAGE_GAP_SHR;
            unpacked->serviceIdentifier = (
                tmcMessage & RDS_TMC_MESSAGE_SID_MASK) >>
                RDS_TMC_MESSAGE_SID_SHR;
            unpacked->activityTime = (
                tmcMessage & RDS_TMC_MESSAGE_TA_MASK) >> RDS_TMC_MESSAGE_TA_SHR;
            unpacked->windowTime = (
                tmcMessage & RDS_TMC_MESSAGE_TW_MASK) >> RDS_TMC_MESSAGE_TW_SHR;
            unpacked->delayTime = tmcMessage & RDS_TMC_MESSAGE_TD_MASK;
            break;
    };
};

word RDSTranslator::decryptLocation(word location, word key) {
    return decryptLocation(location, highByte(key), (lowByte(key) & 0xF0) >> 4,
                           lowByte(key) & 0x0F);
};

word RDSTranslator::decryptLocation(word location, byte xorValue, byte start,
                                    byte rol) {
    uint16_t result = (word(xorValue) << start) ^ location;
    return (result >> (16 - rol)) | (result << rol);
};

word RDSTranslator::decryptLocation(word location, byte encId,
                                    const word table[], bool in_flash) {
    if(in_flash)
        return decryptLocation(location, pgm_read_word(&table[encId]));
    return decryptLocation(location, table[encId]);
};

word RDSTranslator::decryptLocation(word location, byte serviceKey, byte encId,
                                    const word table[][32], bool in_flash) {
    if(in_flash)
    	return decryptLocation(location,
                               pgm_read_word(&table[serviceKey][encId]));
    return decryptLocation(location, table[serviceKey][encId]);
};

void RDSTranslator::unpackTMCFLT(word flt, TRDSTMCFLT *unpacked) {
    if((flt & RDS_TMC_MESSAGE_LOCATION_FOREIGN_MASK) !=
       RDS_TMC_MESSAGE_LOCATION_FOREIGN_MASK)
        //Not a FLT code.
        return;
    if(!unpacked)
        return;
    unpacked->magic = (flt & RDS_TMC_MESSAGE_LOCATION_FOREIGN_MASK) >>
                      RDS_TMC_MESSAGE_LOCATION_FOREIGN_SHR;
    unpacked->country = (flt & RDS_TMC_MESSAGE_LOCATION_FOREIGN_COUNTRY_MASK) >>
                        RDS_TMC_MESSAGE_LOCATION_FOREIGN_COUNTRY_SHR;
    unpacked->locationTableNumber = flt &
                                    RDS_TMC_MESSAGE_LOCATION_FOREIGN_LTN_MASK;
};

void RDSTranslator::unpackTMCMessage8(byte tmcXbits, word tmcYbits,
                                      word tmcZbits,
                                      TRDSTMCMessage8 *unpacked) {
    if(!unpacked)
        return;

    unpacked->systemMessage = (bool)(tmcXbits & RDS_TMC_MESSAGE_SYSTEM);
    if(unpacked->systemMessage) {
        word twochars;

        unpacked->variantCode = tmcXbits & ~RDS_TMC_MESSAGE_SYSTEM;
        switch(unpacked->variantCode) {
          case RDS_TMC_MESSAGE_VARIANT_SPN_A:
          case RDS_TMC_MESSAGE_VARIANT_SPN_B:
              twochars = swab(tmcYbits);
              strncpy(unpacked->serviceProviderName, (char *)&twochars, 2);
              twochars = swab(tmcZbits);
              strncpy(&unpacked->serviceProviderName[2], (char *)&twochars, 2);
              break;
          case RDS_TMC_MESSAGE_VARIANT_EON_AF:
              unpacked->alternativeFrequency[0] = highByte(tmcYbits);
              unpacked->alternativeFrequency[1] = lowByte(tmcYbits);
              unpacked->programIdentifier2 = tmcZbits;
              break;
          case RDS_TMC_MESSAGE_VARIANT_EON_TM:
              unpacked->tuningFrequency = highByte(tmcYbits);
              unpacked->mappedFrequency = lowByte(tmcYbits);
              unpacked->programIdentifier2 = tmcZbits;
              break;
          case RDS_TMC_MESSAGE_VARIANT_EON_PI:
              unpacked->programIdentifier1 = tmcYbits;
              unpacked->programIdentifier2 = tmcZbits;
              break;
          case RDS_TMC_MESSAGE_VARIANT_EON_EX:
              unpacked->locationTableNumber = (
                tmcYbits & RDS_TMC_MESSAGE_LTN_ON_MASK) >>
                RDS_TMC_MESSAGE_LTN_ON_SHR;
              unpacked->international = (bool)(
                  tmcYbits & RDS_TMC_MESSAGE_SCOPE_ON_INTERNATIONAL);
              unpacked->national = (bool)(
                  tmcYbits & RDS_TMC_MESSAGE_SCOPE_ON_NATIONAL);
              unpacked->regional = (bool)(
                  tmcYbits & RDS_TMC_MESSAGE_SCOPE_ON_REGIONAL);
              unpacked->urban = (bool)(
                  tmcYbits & RDS_TMC_MESSAGE_SCOPE_ON_URBAN);
              unpacked->serviceIdentifier = tmcYbits &
                                            RDS_TMC_MESSAGE_SID_ON_MASK;
              unpacked->programIdentifier2 = tmcZbits;
              break;
        };
    } else {
        unpacked->single = (bool)(tmcXbits & RDS_TMC_MESSAGE_SINGLE);
        if(unpacked->single) {
            unpacked->duration = tmcXbits & RDS_TMC_MESSAGE_DURATION_MASK;
            unpacked->diversion = (bool)(tmcYbits & RDS_TMC_MESSAGE_DIVERSION);
            unpacked->direction = (bool)(tmcYbits & RDS_TMC_MESSAGE_DIRECTION);
            unpacked->extent = (tmcYbits & RDS_TMC_MESSAGE_EXTENT_MASK) >>
                               RDS_TMC_MESSAGE_EXTENT_SHR;
            unpacked->event = tmcYbits & RDS_TMC_MESSAGE_EVENT_MASK;
            unpacked->location = tmcZbits;
        } else {
            //Same position as duration in single-group messages
            unpacked->continuationIndicator = tmcXbits &
                                              RDS_TMC_MESSAGE_DURATION_MASK;
            if(unpacked->continuationIndicator) {
                //Same position as diversion in single-group messages
                unpacked->first = (bool)(tmcYbits & RDS_TMC_MESSAGE_DIVERSION);
                if(unpacked->first) {
                    //Same format as single-group messages
                    unpacked->direction = (bool)(
                        tmcYbits & RDS_TMC_MESSAGE_DIRECTION);
                    unpacked->extent = (
                        tmcYbits & RDS_TMC_MESSAGE_EXTENT_MASK) >>
                        RDS_TMC_MESSAGE_EXTENT_SHR;
                    unpacked->event = tmcYbits & RDS_TMC_MESSAGE_EVENT_MASK;
                    unpacked->location = tmcZbits;
                } else {
                    //Same position as direction in single-group messages.
                    unpacked->second = (bool)(
                        tmcYbits & RDS_TMC_MESSAGE_DIRECTION);
                    unpacked->sequence = (
                        tmcYbits & RDS_TMC_MESSAGE_GSI_MASK) >>
                        RDS_TMC_MESSAGE_GSI_SHR;
                    unpacked->data = (
                      (uint32_t)(tmcYbits & RDS_TMC_MESSAGE_CONTAINER_MASK) <<
                      RDS_TMC_MESSAGE_CONTAINER_SHL) | (uint32_t)tmcZbits;
                };
            } else {
                unpacked->encVariantCode = (
                    tmcYbits & RDS_TMC_MESSAGE_ENC_VARIANT_MASK) >>
                    RDS_TMC_MESSAGE_ENC_VARIANT_SHR;
                if(!unpacked->encVariantCode) {
                    unpacked->test = (
                      tmcYbits & RDS_TMC_MESSAGE_EAG_TEST_MASK) >>
                      RDS_TMC_MESSAGE_EAG_TEST_SHR;
                    unpacked->encServiceIdentifier = (
                      tmcYbits & RDS_TMC_MESSAGE_EAG_SID_MASK) >>
                      RDS_TMC_MESSAGE_EAG_SID_SHR;
                    unpacked->encId = tmcYbits & RDS_TMC_MESSAGE_EAG_ENCID_MASK;
                    unpacked->encLocationTableNumber = (
                      tmcZbits & RDS_TMC_MESSAGE_EAG_LTNBE_MASK) >>
                      RDS_TMC_MESSAGE_EAG_LTNBE_SHR;
                };
            };
        };
    };
};

void RDSTranslator::glueTMCContainerSlices(uint32_t slices[4]) {
    slices[0] <<= 4;
    slices[0] |= (slices[1] & 0x0F000000) >> 24;
    slices[1] <<= 8;
    slices[1] |= (slices[2] & 0x0FF00000) >> 20;
    slices[2] <<= 12;
    slices[2] |= (slices[3] & 0x0FFF0000) >> 16;
    slices[3] <<= 16;
};

word RDSTranslator::readFromTMCContainer(const uint32_t slices[4],
                                         TRDSTMCContainerIndex *fp, byte size) {
    if(!fp || size > 16)
        return 0x0000;
    //Enforce order of evaluation
    if(fp->sliceIndex == 6)
        return 0x0000;
    if(!(fp->bitIndex || fp->sliceIndex)) {
        //First call, set pointer to start of container
        fp->bitIndex = 31;
        fp->sliceIndex = 1;
    };

    uint16_t result = 0x0000;

    if(size > fp->bitIndex + 1) {
        //Split fetch
        result = slices[fp->sliceIndex - 1] & ((1U << fp->bitIndex) - 1);
        result <<= size - (fp->bitIndex + 1);
        if(fp->sliceIndex < 4) {
            fp->sliceIndex++;
            result |= (((1U << (size - fp->bitIndex)) - 1) <<
                      (31 - (size - fp->bitIndex) + 1)) &
                      slices[fp->sliceIndex - 1];
            fp->bitIndex = 31 - (size - fp->bitIndex);
        } else
            //Simulate EOF.
            fp->sliceIndex = 6;
    } else {
        //Single fetch
        result = slices[fp->sliceIndex - 1] >> (fp->bitIndex - size + 1);
        result &= (1U << size) - 1;
        if(size == fp->bitIndex + 1) {
            if(fp->sliceIndex < 4) {
                //We read the last bit from the slice, moving on to the next.
                fp->bitIndex = 31;
                fp->sliceIndex++;
            } else {
                //Simulate EOF.
                fp->sliceIndex = 6;
            };
        } else
            fp->bitIndex -= size;
    };

    return result;
};

bool RDSTranslator::readNextTMCLabel(const uint32_t slices[4],
                                     TRDSTMCContainerIndex *fp,
                                     TRDSTMCLabel *label) {
    if(!(fp || label))
        return false;
    //Enforce order of evaluation
    if(fp->sliceIndex == 6)
        //Container at EOF.
        return false;

    label->type = readFromTMCContainer(slices, fp, 4);
    switch(label->type) {
      case RDS_TMC_LABEL_RESERVED2:
        //Label 15 has an undefined size so nothing can exist beyond it: fake
        //EOF status.
        fp->sliceIndex = 6;
        break;
      case RDS_TMC_LABEL_SEPARATOR:
        //Label 14 has a size of zero.
        break;
      case RDS_TMC_LABEL_DURATION:
      case RDS_TMC_LABEL_CONTROL:
        label->value = readFromTMCContainer(slices, fp, 3);
        if(label->type == RDS_TMC_LABEL_DURATION && !label->value) {
            //Label 0 cannot have an argument of 0, if we see that (i.e. 7
            //consecutive zero bits), the end of the container was where the
            //last read ended.
            fp->sliceIndex = 6;
            return false;
        };
        break;
      case RDS_TMC_LABEL_LENGTH:
      case RDS_TMC_LABEL_SPEED:
      case RDS_TMC_LABEL_QUANTIFIER_5:
        label->value = readFromTMCContainer(slices, fp, 5);
        break;
      case RDS_TMC_LABEL_QUANTIFIER_8:
      case RDS_TMC_LABEL_SUPPLEMENTARY:
      case RDS_TMC_LABEL_START:
      case RDS_TMC_LABEL_STOP:
        label->value = readFromTMCContainer(slices, fp, 8);
        break;
      case RDS_TMC_LABEL_ADDITIONAL:
        label->value = readFromTMCContainer(slices, fp, 11);
        break;
      case RDS_TMC_LABEL_DIVERSION:
      case RDS_TMC_LABEL_DESTINATION:
      case RDS_TMC_LABEL_RESERVED1:
      case RDS_TMC_LABEL_XREF:
        label->value = readFromTMCContainer(slices, fp, 16);
        break;
    };

    return true;
};

const char QuantifierText_S_0[] PROGMEM = "%" PRIu8;
const char QuantifierText_S_1[] PROGMEM = "%" PRIu16;
const char QuantifierText_S_2[] PROGMEM = "less than %" PRIu16 " meters";
const char QuantifierText_S_3[] PROGMEM = "%" PRIu8 "%%";
const char QuantifierText_S_4[] PROGMEM = "of up to %" PRIu8 " km/h";
const char QuantifierText_S_5[] PROGMEM = "of up to %" PRIu8 " %s";
const char QuantifierText_S_6[] PROGMEM = "%" PRIi8 " degrees Celsius";
const char QuantifierText_S_7[] PROGMEM = "%02" PRIu8 ":%02" PRIu8;
const char QuantifierText_S_8[] PROGMEM = "%2.1f tonnes";
const char QuantifierText_S_9[] PROGMEM = "%2.1f meters";
const char QuantifierText_S_A[] PROGMEM = "of up to %" PRIu8 " millimeters";
const char QuantifierText_S_B[] PROGMEM = "%3.1f MHz";
const char QuantifierText_S_C[] PROGMEM = "%" PRIu16 " kHz";

PGM_P const QuantifierText_S[13] PROGMEM = {
    QuantifierText_S_0,
    QuantifierText_S_1,
    QuantifierText_S_2,
    QuantifierText_S_3,
    QuantifierText_S_4,
    QuantifierText_S_5,
    QuantifierText_S_6,
    QuantifierText_S_7,
    QuantifierText_S_8,
    QuantifierText_S_9,
    QuantifierText_S_A,
    QuantifierText_S_B,
    QuantifierText_S_C
};

void RDSTranslator::decodeQuantifier(byte qType, TRDSTMCLabel *label, char *buf,
                                     size_t size) {
  if(!(label && buf))
      return;
  if(qType > RDS_TMC_QUANTIFIER_LAST)
      return;
  if(!(label->type == RDS_TMC_LABEL_QUANTIFIER_5 ||
       label->type == RDS_TMC_LABEL_QUANTIFIER_8))
      return;
  if(label->type == RDS_TMC_LABEL_QUANTIFIER_5 &&
     !(qType == RDS_TMC_QUANTIFIER_SMALL_NUMBER ||
       qType == RDS_TMC_QUANTIFIER_NUMBER ||
       qType == RDS_TMC_QUANTIFIER_LESSTHAN_METERS ||
       qType == RDS_TMC_QUANTIFIER_PERCENT ||
       qType == RDS_TMC_QUANTIFIER_UPTO_KMH ||
       qType == RDS_TMC_QUANTIFIER_UPTO_MINUTES))
      return;
  if(label->type == RDS_TMC_LABEL_QUANTIFIER_8 &&
    !(qType == RDS_TMC_QUANTIFIER_DEGREES_CELSIUS ||
      qType == RDS_TMC_QUANTIFIER_TIME ||
      qType == RDS_TMC_QUANTIFIER_TONNES ||
      qType == RDS_TMC_QUANTIFIER_METERS ||
      qType == RDS_TMC_QUANTIFIER_UPTO_MILLIMETERS ||
      qType == RDS_TMC_QUANTIFIER_MHZ ||
      qType == RDS_TMC_QUANTIFIER_KHZ))
      return;

  byte aByte1, aByte2;
  word aWord;
  int8_t aSChar;
  float aFloat;
  switch(qType) {
      case RDS_TMC_QUANTIFIER_SMALL_NUMBER:
          if(label->value <= 28)
              aByte1 = (label->value ? label->value : 36);
          else
              aByte1 = 28 + (label->value - 28) * 2;
          snprintf_P(buf, size, (PGM_P)(pgm_read_ptr(&QuantifierText_S[qType])),
                     aByte1);
          break;
      case RDS_TMC_QUANTIFIER_NUMBER:
          if(label->value <= 4)
              aWord = (label->value ? label->value : 1000);
          else if(label->value > 4 && label->value <= 14)
              aWord = (label->value - 4) * 10;
          else if(label->value > 14)
              aWord = 100 + (label->value - 14) * 50;
          snprintf_P(buf, size, (PGM_P)(pgm_read_ptr(&QuantifierText_S[qType])),
                     aWord);
          break;
      case RDS_TMC_QUANTIFIER_LESSTHAN_METERS:
          aWord = label->value * 10;
          snprintf_P(buf, size, (PGM_P)(pgm_read_ptr(&QuantifierText_S[qType])),
                     aWord);
          break;
      case RDS_TMC_QUANTIFIER_PERCENT:
          aByte1 = (label->value - 1) * 5;
          snprintf_P(buf, size, (PGM_P)(pgm_read_ptr(&QuantifierText_S[qType])),
                     aByte1);
          break;
      case RDS_TMC_QUANTIFIER_UPTO_KMH:
          aByte1 = (label->value ? label->value * 5 : 160);
          snprintf_P(buf, size, (PGM_P)(pgm_read_ptr(&QuantifierText_S[qType])),
                     aByte1);
          break;
      case RDS_TMC_QUANTIFIER_UPTO_MINUTES:
          if(label->value > 0 && label->value <= 10) {
              aByte1 = label->value * 5;
              snprintf_P(buf, size,
                         (PGM_P)(pgm_read_ptr(&QuantifierText_S[qType])),
                         aByte1, "minutes");
          } else {
              if(label->value <= 22)
                  aByte1 = (label->value ? label->value - 10 : 72);
              else
                  aByte1 = 12 + (label->value - 22) * 6;
              snprintf_P(buf, size,
                         (PGM_P)(pgm_read_ptr(&QuantifierText_S[qType])),
                         aByte1, "hours");
          };
          break;
      case RDS_TMC_QUANTIFIER_DEGREES_CELSIUS:
          aSChar = (int8_t)label->value - 51;
          snprintf_P(buf, size, (PGM_P)(pgm_read_ptr(&QuantifierText_S[qType])),
                     aSChar);
          break;
      case RDS_TMC_QUANTIFIER_TIME:
          aByte1 = label->value / 6;
          aByte2 = (label->value % 6 - 1) * 10;
          snprintf_P(buf, size, (PGM_P)(pgm_read_ptr(&QuantifierText_S[qType])),
                     aByte1, aByte2);
          break;
      case RDS_TMC_QUANTIFIER_TONNES:
      case RDS_TMC_QUANTIFIER_METERS:
          if(label->value <= 100)
              aFloat = label->value * 0.1;
          else
              aFloat = 10.0 + (label->value - 100) * 0.5;
          snprintf_P(buf, size, (PGM_P)(pgm_read_ptr(&QuantifierText_S[qType])),
                     aFloat);
          break;
      case RDS_TMC_QUANTIFIER_UPTO_MILLIMETERS:
          aByte1 = label->value;
          snprintf_P(buf, size, (PGM_P)(pgm_read_ptr(&QuantifierText_S[qType])),
                     aByte1);
          break;
      case RDS_TMC_QUANTIFIER_MHZ:
      case RDS_TMC_QUANTIFIER_KHZ:
        //TODO: make this locale-aware (don't hardcode).
        aWord = decodeAFFrequency(
          label->value,
          qType == RDS_TMC_QUANTIFIER_MHZ, RDS_LOCALE_EU);
        snprintf_P(buf, size, (PGM_P)(pgm_read_ptr(&QuantifierText_S[qType])),
                   (qType == RDS_TMC_QUANTIFIER_KHZ ? aWord : aWord / 100.0));
        break;
  };
};

void RDSTranslator::adjustTMCContainerForFLT(uint32_t slices[4], word *maybeFLT,
                                             TRDSTMCFLT *unpacked) {
    if(!maybeFLT)
        return;
    if((*maybeFLT & RDS_TMC_MESSAGE_LOCATION_FOREIGN_MASK) !=
       RDS_TMC_MESSAGE_LOCATION_FOREIGN_MASK)
        //Not a FLT code.
        return;
    if(!unpacked)
        return;

    unpackTMCFLT(*maybeFLT, unpacked);
    *maybeFLT = (slices[0] & 0xFFFF0000) >> 16;
    for(byte i = 0; i < 4; i++) {
      slices[i] <<= 16;
      slices[i] |= (slices[i + 1] & 0xFFFF0000) >> 16;
    }
};

bool RDSTranslator::locateMessageRecord(const void *table, size_t recSize,
                                        size_t tableSize, size_t idOffset,
                                        bool wordId, word key, void *record,
                                        TBlockFetcher blockFetcher) {
    if(!(table && record && blockFetcher && key))
        return false;
    if((!wordId && idOffset + sizeof(byte) > recSize) ||
       (wordId && idOffset + sizeof(word) > recSize))
        return false;
    if(!wordId && (tableSize > 256 || key > 256))
        return false;

    word recNo = key - 1, curId = 0;
    if(recNo > tableSize - 1)
        recNo = tableSize - 1;
    do {
        blockFetcher((byte *)table + recNo * recSize + idOffset, &curId,
                     wordId ? sizeof(word) : sizeof(byte));
        if(curId == key) {
            blockFetcher((byte *)table + recNo * recSize, record, recSize);
            return true;
        };
    } while(recNo-- > 0);

    return false;
};
