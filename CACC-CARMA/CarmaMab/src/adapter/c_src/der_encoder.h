/* Abstract Syntax Notation ver. 1 Distinguished Encoding Rules Encoder Header
 *
 * Provides the necessary types, functions, and datastructures to encode DER
 * compliant byte arrays, intended for use formatting Basic Safety Message
 * packets as defined in J2735.
 *
 * Kyle Rush <kyle.rush@leidos.com> 2015 for FHWA TOPR 5
 */

#ifndef __DER_ENCODER_H__
#define __DER_ENCODER_H__

#include "tmwtypes.h"
#include <string.h>

// ASN.1 Identifier bitmasks
#define CLASS_CONSTRUCTED 0x20
#define CLASS_PRIMITIVE 0x0

#define CLASS_UNIVERSAL 0x0
#define CLASS_APPLICATION 0x40
#define CLASS_CONTEXT_SPECIFIC 0x80
#define CLASS_PRIVATE 0xC0

uint8_T* encodeU8(uint8_T*, uint8_T,  uint32_T, uint8_T);
uint8_T* encodeU16(uint8_T*, uint8_T,  uint32_T, uint16_T);
uint8_T* encodeU32(uint8_T*, uint8_T, uint32_T, uint32_T);

uint8_T* encodeI16(uint8_T*, uint8_T, uint32_T, int16_T);
uint8_T* encodeI32(uint8_T*, uint8_T, uint32_T, int32_T);

uint8_T* encodeF32(uint8_T*, uint8_T, uint32_T, float);

uint8_T* encodeU8Arr(uint8_T*, uint8_T, uint32_T, uint8_T*, uint32_T);

uint8_T* encodeSequence(uint8_T*, uint8_T, uint32_T, uint32_T);

uint8_T* encodeLength(uint8_T*, uint32_T);
uint8_T* encodeTagNumber(uint8_T*, uint32_T);

extern const uint8_T U8_LENGTH;
extern const uint8_T U16_LENGTH;
extern const uint8_T U32_LENGTH;
extern const uint8_T I16_LENGTH;
extern const uint8_T I32_LENGTH;
extern const uint8_T F32_LENGTH;

#define LENGTH_LENGTH 1
#define TAG_LENGTH 1

#endif

