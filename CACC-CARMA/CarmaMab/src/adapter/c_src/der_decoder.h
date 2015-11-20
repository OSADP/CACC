/* Abstract Syntax Notation ver. 1 Distinguished Encoding Rules Decoder Header
 *
 * Provides the necessary types, functions, and datastructures to process DER
 * formatted data, such as messages defined for Connected Vehicle Applications
 * in J2735.
 *
 * Kyle Rush <kyle.rush@leidos.com> 2015 for FHWA TOPR 5
 */

#include "tmwtypes.h"

typedef struct {
	uint8_T class;
	uint8_T constructed;
	uint32_T tag;
	uint32_T length;
	uint8_T* value;
} AsnObject;


AsnObject parseAsnObject(uint8_T*, uint32_T*);
uint32_T parseTagField(uint8_T*, uint32_T*);
uint32_T parseLengthField(uint8_T*, uint32_T*);

