/* Abstract Syntax Notation ver. 1 Distinguished Encoding Rules Encoder
 * 
 * Provides the necessary types, functions, and datastructures to encode DER
 * compliant byte arrays, intended for use formatting Basic Safety Message
 * packets as defined in J2735.
 *
 * Kyle Rush <kyle.rush@leidos.com> 2015 for FHWA TOPR 5
 */

#include "der_encoder.h"

const uint8_T U8_LENGTH = 1;
const uint8_T U16_LENGTH = 2;
const uint8_T U32_LENGTH = 4;
const uint8_T I16_LENGTH = 2;
const uint8_T I32_LENGTH = 4;
const uint8_T F32_LENGTH = 4;

/**
 * Encodes a U8 value at the specified location.
 *
 * @params loc A pointer to the location the value should be encoded to
 * @params class A bitmask representing the class and primitive/constructed status
 * @params tag The DER/ASN.1 tag of the value to the encoded
 * @params value The U8 value to be encoded
 * @return uint8_T* A pointer to the next available location to encode data
 */
uint8_T* encodeU8(uint8_T* loc, uint8_T class, uint32_T tag, uint8_T value) {
	uint8_T* next;
	*loc |= class;
	next = encodeTagNumber(loc, tag);
	next = encodeLength(next, U8_LENGTH);
	memcpy(next, &value, U8_LENGTH);

	return next + U8_LENGTH;
}

/**
 * Encodes a U16 value at the specified location.
 *
 * @params loc A pointer to the location the value should be encoded to
 * @params class A bitmask representing the class and primitive/constructed status
 * @params tag The DER/ASN.1 tag of the value to the encoded
 * @params value The U16 value to be encoded
 * @return uint8_T* A pointer to the next available location to encode data
 */
uint8_T* encodeU16(uint8_T* loc, uint8_T class, uint32_T tag, uint16_T value) {
	uint8_T* next;
 	*loc |= class;
	next = encodeTagNumber(loc, tag);
	next = encodeLength(next, U16_LENGTH);
	memcpy(next, &value, U16_LENGTH);

	return next + U16_LENGTH;
}

/**
 * Encodes a U32 value at the specified location.
 *
 * @params loc A pointer to the location the value should be encoded to
 * @params class A bitmask representing the class and primitive/constructed status
 * @params tag The DER/ASN.1 tag of the value to the encoded
 * @params value The U32 value to be encoded
 * @return uint8_T* A pointer to the next available location to encode data
 */
uint8_T* encodeU32(uint8_T* loc, uint8_T class, uint32_T tag, uint32_T value) {
	uint8_T* next;
 	*loc |= class;
	next = encodeTagNumber(loc, tag);
    next = encodeLength(next, U32_LENGTH);
	memcpy(next, &value, U32_LENGTH);

	return next + U32_LENGTH;
}

/**
 * Encodes a I16 value at the specified location.
 *
 * @params loc A pointer to the location the value should be encoded to
 * @params class A bitmask representing the class and primitive/constructed status
 * @params tag The DER/ASN.1 tag of the value to the encoded
 * @params value The I16 value to be encoded
 * @return uint8_T* A pointer to the next available location to encode data
 */
uint8_T* encodeI16(uint8_T* loc, uint8_T class, uint32_T tag, int16_T value) {
	uint8_T* next;
	*loc |= class;
	next = encodeTagNumber(loc, tag);
    next = encodeLength(next, I16_LENGTH);
	memcpy(next, &value, I16_LENGTH);

	return next + I16_LENGTH;
}

/**
 * Encodes a I32 value at the specified location.
 *
 * @params loc A pointer to the location the value should be encoded to
 * @params class A bitmask representing the class and primitive/constructed status
 * @params tag The DER/ASN.1 tag of the value to the encoded
 * @params value The I32 value to be encoded
 * @return uint8_T* A pointer to the next available location to encode data
 */
uint8_T* encodeI32(uint8_T* loc, uint8_T class, uint32_T tag, int32_T value) {
	uint8_T* next;
	*loc |= class;
	next = encodeTagNumber(loc, tag);
    next = encodeLength(next, I32_LENGTH);
	memcpy(next, &value, I32_LENGTH);

	return next + I32_LENGTH;
}

/**
 * Encodes a F32 value at the specified location.
 *
 * @params loc A pointer to the location the value should be encoded to
 * @params class A bitmask representing the class and primitive/constructed status
 * @params tag The DER/ASN.1 tag of the value to the encoded
 * @params value The F32 value to be encoded
 * @return uint8_T* A pointer to the next available location to encode data
 */
uint8_T* encodeF32(uint8_T* loc, uint8_T class, uint32_T tag, float value) {
	uint8_T* next;
	*loc |= class;
	next = encodeTagNumber(loc, tag);
    next = encodeLength(next, F32_LENGTH);
	memcpy(next, &value, F32_LENGTH);

	return next + F32_LENGTH;
}

/**
 * Encodes an OCTET STRING value at the specified location.
 *
 * @params loc A pointer to the location the value should be encoded to
 * @params class A bitmask representing the class and primitive/constructed status
 * @params tag The DER/ASN.1 tag of the value to the encoded
 * @params value A pointer to the OCTET STRING to be encoded
 * @params length The length of the OCTET STRING element and input array
 * @return uint8_T* A pointer to the next available location to encode data
 */ 
uint8_T* encodeU8Arr(uint8_T* loc, uint8_T class, uint32_T tag, uint8_T* value, uint32_T length) {
	uint8_T* next;
	*loc |= class;
	next = encodeTagNumber(loc, tag);
    next = encodeLength(next, length);
	memcpy(next, value, length);

	return next + length;
}

/**
 * Encodes a sequence element at the specified location.
 *
 * @params loc A pointer to the location the sequence should be encoded to
 * @params class A bitmask representing the class and primitive/constructed status
 * @params tag The DER/ASN.1 tag of the sequence to the encoded
 * @params length The length of the sequence's contents
 * @return uint8_T* A pointer to the next available location to encode data,
 *                  unlike other functions in this file, this pointer points
 *                  to the first available location _INSIDE_ the sequence.
 */
uint8_T* encodeSequence(uint8_T* loc, uint8_T class, uint32_T tag, uint32_T length) {
	uint8_T* next;
	*loc |= class;
	//*loc |= ((uint8_T) (16 & 0x1F));
	next = encodeTagNumber(loc, tag);
    next = encodeLength(next, length);

	return next;
}

/**
 * Encodes a variable length tag number, starting at bit 5 of the specified location
 *
 * @params loc A pointer to the byte the tag number should start at
 * @params tag The number of the tag to encode
 * @return uint8_T* A pointer to the next available location to encode data,
 *                  right after the tag.
 */
uint8_T* encodeTagNumber(uint8_T* loc, uint32_T tagNumber) {
	int idx = 3;
	uint8_T temp[4] = {0};
	uint8_T lowestSeven = 0;
	
	if (tagNumber <= 30) {
		*loc |=  tagNumber;
		return loc + 1;
	} else {
		// Write extended form tag
		*loc |= 0x1F; // Set signifier bits to denote extended form
		
		while (tagNumber > 0) {
			// Mask out the lower order bits
			lowestSeven = tagNumber & 0x7F;
			
			if (idx == 3) {
				// There's no more data after the lowest order bits
				lowestSeven &= 0x7F;
			} else {
				lowestSeven |= 0x80;
			} 

			temp[idx] = lowestSeven;
            lowestSeven = 0;
            
		   	tagNumber >>= 7;
			idx--;
		}

		// Store the computed value back in loc
		memcpy(loc + 1, &temp[idx + 1], 3 - idx);
		
		return loc + 1 + 3 - idx;
	}
}

/**
 * Encodes a variable length field length starting at bit 7 of the specified
 * location.
 *
 * @params loc A pointer to the byte the length encoding should start at
 * @params length The length value to encode
 * @return uint8_T* A pointer to the next available location to encode data,
 *                  right after the length value.
 */
uint8_T* encodeLength(uint8_T* loc, uint32_T length) {
	uint8_T temp[4] = {0};
	int idx = 3;
		
	if (length <= 127) {
		*loc = (uint8_T) length;
		return loc + 1;
	} else {
		// Encode extended-form length
	
		while (length > 0) {
			temp[idx] = (uint8_T) length & 0xFF;

			idx--;
			length >>= 8;
		}

		*loc |= 0x80; // Set the highest order bit
		*loc |= (uint8_T) (3 - idx); // Set the number of length bytes
		
		memcpy(loc + 1, &temp[idx + 1], 3 - idx); // Copy the actual length bytes

		return loc + 1 + 3 - idx;
	}
}

