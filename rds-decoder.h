#include <stdint.h>
#include <stdbool.h>
#include <string.h>

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

const uint8_t RDS_TMC_D2DynamicPersistence[8] = {
    15, 15, 30, 60, 120, 180, 240, 0xFE};
const uint8_t RDS_TMC_D2LongerlastingPersistence[8] = {
    60, 120, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// NOTE: change to detailed format from EWS spec!
typedef TRDSODAGroup TRDSGroup9A;

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

#define RDS_1A_ECCPI2Country(extendedcountrycode, programidentifier) \
    ((uint16_t)((extendedcountrycode) << 8 | (TRDSPI)(programidentifier).country))

