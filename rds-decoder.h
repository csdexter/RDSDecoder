#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct __attribute__ ((__packed__)) {
  uint8_t country:4;
  uint8_t area:4;
  uint8_t program;
} TRDSPI;

// Block 1 of ANY group ALWAYS has this format
typedef struct __attribute__ ((__packed__)) {
  union {
    TRDSPI programIdentifierEU;
    uint16_t programIdentifierUS;
  };
} TRDSBlock1;

// Block 3 of ANY version B group ALWAYS has this format
typedef TRDSBlock1 TRDSBlock3;

// Block 2 of ANY group ALWAYS begins with this format
typedef struct __attribute__ ((__packed__)) {
  uint8_t groupType:4;
  uint8_t version:1;
  uint8_t trafficProgram:1;
  uint8_t programType:5;
} TRDSBlock2;

typedef struct __attribute__ ((__packed__)) {
  TRDSBlock1 block1;
  TRDSBlock2 block2;
  uint8_t segmentAddress:5;
  union {
    uint32_t dataA;
    struct {
      TRDSBlock3 block3;
      uint16_t dataB;
    };
  };
} TRDSODAGroup;

typedef struct __attribute__ ((__packed__)) {
  TRDSBlock1 block1;
  TRDSBlock2 block2;
  uint8_t trafficAnnouncement:1;
  uint8_t musicSpeechSwitch:1;
  uint8_t decoderIdentification:1;
  uint8_t segmentAddress:2;
  union {
    uint8_t alternativeFrequency[2];
    TRDSBlock3 block3;
  };
  char programServiceName[2];
} TRDSGroup0;

typedef struct __attribute__ ((__packed__)) {
  TRDSBlock1 block1;
  TRDSBlock2 block2;
  union {
    struct {
      uint8_t radioPaging:5;
      uint8_t linkageActuator:1;
      uint8_t variantCode:3;
      union {
        struct {
          uint8_t pagingOperator:4;
          uint8_t extendedCountryCode;
        };
        uint16_t data:12;
      };
    };
    struct {
      uint8_t unused:5;
      TRDSBlock3 block3;
    };
  };
  uint8_t day:5;
  uint8_t hour:5;
  uint8_t minute:6;
} TRDSGroup1;

typedef struct __attribute__ ((__packed__)) {
  TRDSBlock1 block1;
  TRDSBlock2 block2;
  uint8_t textABFlag:1;
  uint8_t segmentAddress:4;
  union {
    char radioTextA[4];
    struct {
      TRDSBlock3 block3;
      char radioTextB[2];
    };
  };
} TRDSGroup2;

typedef struct __attribute__ ((__packed__)) {
  TRDSBlock1 block1;
  TRDSBlock2 block2;
  uint8_t applicationGroupType:5;
  uint16_t message;
  uint16_t applicationIdentifier;
} TRDSGroup3A;

typedef TRDSODAGroup TRDSGroup3B;

typedef struct __attribute__ ((__packed__)) {
  TRDSBlock1 block1;
  TRDSBlock2 block2;
  uint8_t unused:3;
  uint32_t modifiedJulianDay:17;
  uint8_t hour:5;
  uint8_t minute:6;
  uint8_t offsetSign:1;
  uint8_t timeZoneOffset:5;
} TRDSGroup4A;

typedef TRDSODAGroup TRDSGroup4B;

typedef struct __attribute__ ((__packed__)) {
  TRDSBlock1 block1;
  TRDSBlock2 block2;
  uint8_t address:5;
  union {
    uint32_t dataA;
    struct {
      TRDSBlock3 block3;
      uint16_t dataB;
    };
  };
} TRDSGroup5;

typedef TRDSODAGroup TRDSGroup6;

typedef struct __attribute__ ((__packed__)) {
  TRDSBlock1 block1;
  TRDSBlock2 block2;
  uint8_t pagingABFlag:1;
  uint8_t segmentAddress:5;
  uint32_t data;
} TRDSGroup7A;

#define RDS_7A_SEGMENT_NOMESSAGE 0x00
#define RDS_7A_SEGMENT_FUNCTION 0x01
#define RDS_7A_SEGMENT_10DIGIT 0x02
#define RDS_7A_SEGMENT_18DIGIT 0x04
#define RDS_7A_SEGMENT_ALPHA 0x08

typedef struct __attribute__ ((__packed__)) {
  uint8_t nibble_:4;
} TRDSBCD;

typedef struct __attribute__ ((__packed__)) {
  union {
    struct {
      TRDSBCD group[2];
      TRDSBCD individual[4];
      union {
        uint8_t unused;
        // NOTE: BCD packed but HEX coding!
        TRDSBCD digits1[2];
      };
    };
    TRDSBCD digits2[8];
    char message[4];
  };
} TRDSPaging;

typedef TRDSODAGroup TRDSGroup7B;

typedef struct __attribute__ ((__packed__)) {
  TRDSBlock1 block1;
  TRDSBlock2 block2;
  uint8_t systemMessage:1;
  union {
    uint8_t variantCode:4;
    struct {
      uint8_t single:1;
      union {
        uint8_t duration:3;
        uint8_t continuationIndicator:3;
      };
    };
  };
  union {
    struct {
      union {
        uint8_t first:1;
        uint8_t diversion:1;
      };
      union {
        struct {
          uint8_t direction:1;
          uint8_t extent:3;
          uint16_t event:11;
          uint16_t location;
        };
        struct {
          uint8_t second:1;
          uint8_t sequence:2;
          uint32_t data:28;
        };
      };
    };
    char serviceProviderName[4];
    struct {
      union {
        uint8_t alternativeFrequency[2];
        struct {
          uint8_t tuningFrequency;
          uint8_t mappedFrequency;
        };
        TRDSBlock1 block3;
        struct {
          uint8_t locationTableNumber:6;
          uint8_t international:1;
          uint8_t national:1;
          uint8_t regional:1;
          uint8_t urban:1;
          uint8_t serviceIdentifier:6;
        };
      };
      TRDSBlock1 block4;
    };
  };
} TRDSGroup8A;

const uint8_t RDS_TMC_Label2Sizes[16] = {
    3, 3, 5, 5, 5, 8, 8, 8, 8, 11, 16, 16, 16, 16, 0, 0};
const uint8_t RDS_TMC_D2DynamicPersistence[8] = {
    15, 15, 30, 60, 120, 180, 240, 0xFE};
const uint8_t RDS_TMC_D2LongerlastingPersistence[8] = {
    60, 120, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#define RDS_TMC_L1_URGENCY_INC 0x00
#define RDS_TMC_L1_URGENCY_DEC 0x01
#define RDS_TMC_L1_DIRECTION_INV 0x02
#define RDS_TMC_L1_DURATION_INV 0x03
#define RDS_TMC_L1_SPOKEN_INV 0x04
#define RDS_TMC_L1_DIVERSION 0x05
#define RDS_TMC_L1_EXTENT_ADD8 0x06
#define RDS_TMC_L1_EXTENT_ADD16 0x07

#define RDS_8A_VARIANT_SPN_A 0x04
#define RDS_8A_VARIANT_SPN_B 0x05
#define RDS_8A_VARIANT_EON_AF 0x06
#define RDS_8A_VARIANT_EON_TM 0x07
#define RDS_8A_VARIANT_EON_PI 0x08
#define RDS_8A_VARIANT_EON_EX 0x09

typedef TRDSODAGroup TRDSGroup8B;

// NOTE: change to detailed format from EWS spec!
typedef TRDSODAGroup TRDSGroup9A;

typedef TRDSODAGroup TRDSGroup9B;

typedef struct __attribute__ ((__packed__)) {
  TRDSBlock1 block1;
  TRDSBlock2 block2;
  uint8_t programTypeNameABFlag:1;
  uint8_t unused:3;
  uint8_t segmentAddress:1;
  char programTypeName[4];
} TRDSGroup10A;

typedef TRDSODAGroup TRDSGroup10B;

typedef TRDSODAGroup TRDSGroup11;

typedef TRDSODAGroup TRDSGroup12;

typedef struct __attribute__ ((__packed__)) {
  TRDSBlock1 block1;
  TRDSBlock2 block2;
  uint8_t informationField1:2;
  uint8_t segmentType:3;
  uint32_t informationField2;
} TRDSGroup13A;

typedef TRDSODAGroup TRDSGroup13B;

typedef struct __attribute__ ((__packed__)) {
  TRDSBlock1 block1;
  TRDSBlock2 block2;
  uint8_t trafficProgram:1;
  union {
    struct {
      uint8_t variantCode:4;
      union {
        char programServiceName[2];
        uint8_t alternativeFrequency[2];
        struct {
          uint8_t tuningFrequency;
          uint8_t mappedFrequency;
        };
	uint16_t linkageInformation;
        struct {
          uint8_t programType:5;
          uint16_t unused1:10;
          uint8_t trafficAnnouncement1:1;
        };
        struct {
	  uint8_t day:5;
	  uint8_t hour:5;
	  uint8_t minute:6;
	};
        uint16_t inHouse;
      };
    };
    struct {
      uint8_t trafficAnnouncement2:1;
      uint8_t unused2:3;
      TRDSBlock3 block3;
    };
  };
  TRDSBlock1 block4;
} TRDSGroup14;

// NOTE: supposedly deprecated and currently unassigned, supposedly ...
typedef struct __attribute__ ((__packed__)) {
  TRDSBlock1 block1;
  TRDSBlock2 block2;
  uint8_t trafficAnnouncement:1;
  uint8_t unused:3;
  uint8_t segmentAddress:1;
  char programServiceName[4];
} TRDSGroup15A;

typedef struct __attribute__ ((__packed__)) {
  TRDSBlock1 block1;
  TRDSBlock2 block2;
  uint8_t trafficAnnouncement1:1;
  uint8_t musicSpeechSwitch1:1;
  uint8_t decoderIdentification1:1;
  uint8_t segmentAddress1:2;
  TRDSBlock3 block3;
  TRDSBlock2 block4;
  uint8_t trafficAnnouncement2:1;
  uint8_t musicSpeechSwitch2:1;
  uint8_t decoderIdentification2:1;
  uint8_t segmentAddress2:2;  
} TRDSGroup15B;

#define RDS_13A_SEGMENT_ADDRESS_SHORT 0x00
#define RDS_13A_SEGMENT_ADDRESS_LONGHIGH 0x01
#define RDS_13A_SEGMENT_ADDRESS_LONGLOW 0x02
#define RDS_13A_SEGMENT_VALUE_ADDED 0x03

#define RDS_TMC_ANY_SID 0x00
const uint8_t RDS_TMC_G2Values[4] = {3, 5, 8, 11};
#define RDS_TMC_Tw2Value(x) (1 << (x))
#define RDS_TMC_Ta2Value(x) (1 << (x))

#define RDS_1A_ECCPI2Country(extendedcountrycode, programidentifier) \
    ((uint16_t)((extendedcountrycode) << 8 | (TRDSPI)(programidentifier).country))

