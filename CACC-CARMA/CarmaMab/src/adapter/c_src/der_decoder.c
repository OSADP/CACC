/* Abstract Syntax Notation ver. 1 Distinguished Encoding Rules Decoder Source
 *
 * Provides the necessary types, functions, and datastructures to process DER
 * formatted data, such as messages defined for Connected Vehicle Applications
 * in J2735.
 *
 * Kyle Rush <kyle.rush@leidos.com> 2015 for FHWA TOPR 5
 */

#include "tmwtypes.h"
#include "der_decoder.h"

/**
 * Decode an ASN.1 DER/BER encoded value from a pointer in a byte array.
 *
 * The AsnObject return value contains copies of the tag and length fields as
 * well as a pointer into the original buffer where the value lies. Thus, the
 * the lifespan of this AsnObject is inherently tied to the lifespan of the
 * buffer containing it and should not be used after that buffer is freed.
 *
 * @params buf An unsigned byte array containing the ASN.1 object to be decoded
 * @return AsnObject A struct containing the parsed tag, length, and value of
 *                   the ASN.1 object.
 */
AsnObject parseAsnObject(uint8_T* buf, uint32_T* offset) {
    int lclOffset = 0;
	AsnObject out;
		
	out.tag = parseTagField(buf, &lclOffset);
	out.length = parseLengthField(buf + lclOffset, &lclOffset);
	out.value = buf + lclOffset;

	// Increment the offset
	*offset += (lclOffset + out.length);
	
	return out;
}

/**
 * Decode an ASN.1 DER/BER encoded tag value from a pointer to it's containing
 * byte. Parses both short and long form tag values up to 32 bits in length.
 *
 * @params tagLoc A pointer to the byte containing the start of the tag value
 * @params offset A pointer to an integer which will contain an offset into a
 *                byte array. This will be updated with the new offset after the
 *                length value.
 * @return uint32_T An unsigned integer representing the tag value.
 */
uint32_T parseTagField(uint8_T* tagLoc, uint32_T* offset) {
	uint8_T firstByte = *tagLoc & 0x1F;
	if (firstByte <= 30) {
		// Non-extended form, return as is.
		*offset += 1;
		return (uint32_T) firstByte;
	} else {
		// Parse out the extended form tag
		int i = 1;
		uint8_T cur = tagLoc[i];
		uint32_T tag = 0;
		do {
			// Kick off the highest-order bit
			cur = (cur << 1) >> 1;

			// Advance the current tag bits up
			tag <<= 7;

			// Put our current byte in the lowest order bits of the tag
			tag |= cur;

			// Advance to the next byte
			cur = tagLoc[++i];
		} while (cur & 0x80);

		*offset += i;
		return tag;
	}
}

/**
 * Decode an ASN.1 DER/BER encoded length value from a pointer to its containing
 * byte. Parses both short and long form length values up to 32 bits in length.
 *
 * @params lengthLoc A pointer to the first byte of the length value
 * @params offset A pointer to an integer which will contain an offset into a
 *                byte array. This will be updated with the new offset after the
 *                length value.
 * @return uint32_T An unsigned integer representation of the length value
 */
uint32_T parseLengthField(uint8_T* lengthLoc, uint32_T* offset) {
	uint8_T firstByte = *lengthLoc;

	if (!(firstByte & 0x80)) {
		// Short form, return as is
		*offset += 1;
		return firstByte & 0x7F;
	} else {
		// Parse the long form
		uint8_T lengthLength = firstByte & 0x7F;
		uint32_T length = 0;
		int i = 0;

		// Accumulate the subsequent bytes into the length field
		for (i = 0; i < lengthLength; i++) {
			length <<= 8;
			length += lengthLoc[1 + i];
		}

		*offset += (1 + lengthLength);
		return length;
	}
}

