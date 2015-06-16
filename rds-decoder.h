#include <stdint.h>
#include <stdbool.h>
#include <string.h>

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

typedef struct __attribute__ ((__packed__)) {
  TRDSBlock1 block1;
  TRDSBlock2 block2;
  uint8_t informationField1:2;
  uint8_t segmentType:3;
  uint32_t informationField2;
} TRDSGroup13A;

typedef TRDSODAGroup TRDSGroup13B;

#define RDS_13A_SEGMENT_ADDRESS_SHORT 0x00
#define RDS_13A_SEGMENT_ADDRESS_LONGHIGH 0x01
#define RDS_13A_SEGMENT_ADDRESS_LONGLOW 0x02
#define RDS_13A_SEGMENT_VALUE_ADDED 0x03
