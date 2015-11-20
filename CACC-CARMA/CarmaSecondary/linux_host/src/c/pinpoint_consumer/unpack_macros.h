/** CONFIDENTIAL - do not disclose, distribute, or alter without express permission by TORC Robotics.
All use subject to express agreement only. No implied use or license.

Software copyrights by TORC Robotics, LLC as of initial publish date.

Unless required by applicable law or agreed in writing, use of software is on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef PINPOINT_UNPACK_MACROS_H
#define PINPOINT_UNPACK_MACROS_H

#include <stdint.h>

/**
 * These macros convert numbers from byte array representations of little endian endian numbers to standard integers and unsigned integers of the proper size and endianness
 */

// Converts a byte array (pass in a uint8_t*) to an unsigned 8 bit integer type
static __inline uint8_t UNPACK_UINT8(const uint8_t* const bytes) {
    return bytes[0];
}

// Converts a byte array (pass in a uint8_t*) to a signed 8 bit integer type
static __inline int8_t UNPACK_INT8(const uint8_t* const bytes) {
    return (int8_t)bytes[0];
}

// Converts a byte array (pass in a uint8_t*) to an unsigned 16 bit integer type
static __inline uint16_t UNPACK_UINT16(const uint8_t* const bytes) {
    uint16_t ret = ((uint16_t)bytes[1] << 8);
    ret |= ((uint16_t)bytes[0]);
    return ret;
}

// Converts a byte array (pass in a uint8_t*) to a signed 16 bit integer type
static __inline int16_t UNPACK_INT16(const uint8_t* const bytes) {
    int16_t ret = ((int16_t)bytes[1] << 8);
    ret |= ((uint16_t)bytes[0]);
    return ret;
}

// Converts a byte array (pass in a uint8_t*) to an unsigned "24" bit integer type. In C, there is no standard 24 bit integer, so we just use a 32 bit integer to store it
static __inline uint32_t UNPACK_UINT24(const uint8_t* const bytes) {
    uint32_t ret = ((uint32_t)bytes[2] << 16);
    ret |= ((uint32_t)bytes[1] << 8);
    ret |= ((uint32_t)bytes[0]);
    return ret;
}

// Converts a byte array (pass in a uint8_t*) to a signed "24" bit integer type. In C, there is no standard 24 bit integer, so we just use a 32 bit integer to store it
// We align it with the most significant byte so the sign is correct, then shift everything down a byte to fix the scaling
static __inline int32_t UNPACK_INT24(const uint8_t* const bytes) {
    int32_t ret = (int32_t)bytes[2] << 24;
    ret |= ((uint32_t)bytes[1] << 16);
    ret |= ((uint32_t)bytes[0] << 8 );
    ret = ret >> 8;
    return ret;
}

// Converts a byte array (pass in a uint8_t*) to an unsigned 32 bit integer type
static __inline uint32_t UNPACK_UINT32(const uint8_t* const bytes) {
    uint32_t ret = ((uint32_t)bytes[3] << 24);
    ret |= ((uint32_t)bytes[2] << 16);
    ret |= ((uint32_t)bytes[1] << 8);
    ret |= ((uint32_t)bytes[0]);
    return ret;
}

// Converts a byte array (pass in a uint8_t*) to a signed 32 bit integer type
static __inline int32_t UNPACK_INT32(const uint8_t* const bytes) {
    int32_t ret = (int32_t)bytes[3] << 24;
    ret |= ((uint32_t)bytes[2] << 16);
    ret |= ((uint32_t)bytes[1] << 8);
    ret |= ((uint32_t)bytes[0]);
    return ret;
}

// Converts a byte array (pass in a uint8_t*) to an unsigned 64 bit integer type
static __inline uint64_t UNPACK_UINT64(const uint8_t* const bytes) {
    uint64_t ret = ((uint64_t)bytes[7] << 56);
    ret |= ((uint64_t)bytes[6] << 48);
    ret |= ((uint64_t)bytes[5] << 40);
    ret |= ((uint64_t)bytes[4] << 32);
    ret |= ((uint64_t)bytes[3] << 24);
    ret |= ((uint64_t)bytes[2] << 16);
    ret |= ((uint64_t)bytes[1] << 8 );
    ret |= ((uint64_t)bytes[0]);
    return ret;
}

// Converts a byte array (pass in a uint8_t*) to a signed 64 bit integer type
static __inline int64_t UNPACK_INT64(const uint8_t* const bytes) {
    int64_t ret = (int64_t)bytes[7] << 56;
    ret |= ((uint64_t)bytes[6] << 48);
    ret |= ((uint64_t)bytes[5] << 40);
    ret |= ((uint64_t)bytes[4] << 32);
    ret |= ((uint64_t)bytes[3] << 24);
    ret |= ((uint64_t)bytes[2] << 16);
    ret |= ((uint64_t)bytes[1] << 8 );
    ret |= ((uint64_t)bytes[0]);
    return ret;
}

#endif
