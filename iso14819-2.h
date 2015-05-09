/* Arduino RDS/RBDS (IEC 62016/NRSC-4-B) Decoding Library
 * See the README file for author and licensing information. In case it's
 * missing from your distribution, use the one here as the authoritative
 * version: https://github.com/csdexter/RDSDecoder/blob/master/README
 *
 * This library is for decoding RDS/RBDS data streams (groups).
 * See the example sketches to learn how to use the library in your code.
 *
 * This file contains definitions that are only used by ISO 14819-2 -specific
 * code i.e. TMC event rendering.
 * 
 * NOTE: the term "EEPROM" as used in the conditional #defines below refers to
 *       an externally connected EEPROM that's large enough to store the entire
 *       data structure configured for EEPROM storage (e.g. serial EEPROM via
 *       SPI or I2C). It does *NOT* in any way or form refer to the EEPROM
 *       memory in the AVR chip itself!
 */

#ifndef _ISO14819_2_H_INCLUDED
#define _ISO14819_2_H_INCLUDED

#if defined(__GNUC__)
# if defined(__AVR__)
#  if defined(ARDUINO) && ARDUINO >= 100
#   include <Arduino.h>
#  else
#   include <WProgram.h>
#  endif
# elif defined(__i386__) || defined(__x86_64__)
#  include <stdint.h>
#  include <stdbool.h>
#  define byte uint8_t
#  define PROGMEM
# endif
#else
# warning Non-GNU compiler detected, you are on your own!
#endif

/* Ask for ALL IN by default so that it fails on most AVRs and makes people pay
 * attention and make an informed and deliberate choice about their
 * configuration.
 */
#if !(defined(WITH_RDS_TMC_ALLIN_FLASH) || defined(WITH_RDS_TMC_ALLIN_EEPROM))
# warning No ISO 14819-2 string tables storage configuration specified, using\
 defaults which may not be what you want!
# define WITH_RDS_TMC_ALLIN_FLASH
#endif

#if defined(WITH_RDS_TMC_ALLIN_FLASH)
# define WITH_RDS_TMC_EVENTS
# define WITH_RDS_TMC_SUPPLEMENTARY
# define WITH_RDS_TMC_EVENT_STRING_POINTERS
# define WITH_RDS_TMC_EVENT_STRINGS_FLASH
# define WITH_RDS_TMC_SUPPLEMENTARY_STRINGS_FLASH
#endif

#if defined(WITH_RDS_TMC_ALLIN_EEPROM)
# define WITH_RDS_TMC_EVENTS
# define WITH_RDS_TMC_SUPPLEMENTARY
# define WITH_RDS_TMC_EVENT_STRING_POINTERS
# define WITH_RDS_TMC_EVENT_STRINGS_EEPROM
# define WITH_RDS_TMC_SUPPLEMENTARY_STRINGS_EEPROM
#endif

/* Sanity check string table configuration */
#if defined(WITH_RDS_TMC_EVENT_STRING_POINTERS) && \
 !defined(WITH_RDS_TMC_EVENTS)
# warning ISO 14819-2 string pointers in event list requested but event list\
 not enabled: enabling event list in FLASH!
# define WITH_RDS_TMC_EVENTS
#endif

#if defined(WITH_RDS_TMC_EVENT_STRING_POINTERS) && \
 !(defined(WITH_RDS_TMC_EVENT_STRINGS_FLASH) ||\
   defined(WITH_RDS_TMC_EVENT_STRINGS_EEPROM))
# warning ISO 14819-2 string pointers in event list requested but no storage\
 configuration specified for the pointed-to strings: choosing FLASH which may\
 not be what you want!
# define WITH_RDS_TMC_EVENT_STRINGS_FLASH
#endif

#if defined(WITH_RDS_TMC_SUPPLEMENTARY) && \
 !(defined(WITH_RDS_TMC_SUPPLEMENTARY_STRINGS_FLASH) ||\
   defined(WITH_RDS_TMC_SUPPLEMENTARY_STRINGS_EEPROM))
# warning ISO 14819-2 supplementary information string table requested but no\
 storage configuration specified for said strings: choosing FLASH which may not\
 be what you want!
# define WITH_RDS_TMC_SUPPLEMENTARY_STRINGS_FLASH
#endif

#define RDS_TMC_QUANTIFIER_SMALL_NUMBER 0x0
#define RDS_TMC_QUANTIFIER_NUMBER 0x1
#define RDS_TMC_QUANTIFIER_LESSTHAN_METERS 0x2
#define RDS_TMC_QUANTIFIER_PERCENT 0x3
#define RDS_TMC_QUANTIFIER_UPTO_KMH 0x4
#define RDS_TMC_QUANTIFIER_UPTO_MINUTES 0x5
#define RDS_TMC_QUANTIFIER_DEGREES_CELSIUS 0x6
#define RDS_TMC_QUANTIFIER_TIME 0x7
#define RDS_TMC_QUANTIFIER_TONNES 0x8
#define RDS_TMC_QUANTIFIER_METERS 0x9
#define RDS_TMC_QUANTIFIER_UPTO_MILLIMETERS 0xA
#define RDS_TMC_QUANTIFIER_MHZ 0xB
#define RDS_TMC_QUANTIFIER_KHZ 0xC
#define RDS_TMC_QUANTIFIER_LAST RDS_TMC_QUANTIFIER_KHZ

#define RDS_TMC_NATURE_INFORMATION 0x0
#define RDS_TMC_NATURE_FORECAST 0x1
#define RDS_TMC_NATURE_SILENT 0x2

#define RDS_TMC_URGENCY_NORMAL 0x0
#define RDS_TMC_URGENCY_URGENT 0x1
#define RDS_TMC_URGENCY_XURGENT 0x2

#if defined(WITH_RDS_TMC_EVENTS)
typedef struct __attribute__ ((__packed__)) {
  uint16_t code:12;
  uint8_t quantifier:4;
  uint8_t nature:2;
  uint8_t urgency:2;
  uint8_t longerLasting:1;
  uint8_t silentDuration:1;
  uint8_t bidirectional:1;
  byte updateClass;
# if defined(WITH_RDS_TMC_EVENT_STRING_POINTERS)
  const char * const description;
# endif
} TRDSTMCEventListEntry;
#endif

#if defined(WITH_RDS_TMC_SUPPLEMENTARY)
typedef struct {
  byte code;
  const char * const description;
} TRDSTMCSupplementaryEntry;
#endif

#include "iso14819-2-events.h"
#include "iso14819-2-supplementary.h"

#endif
