/* TOPR 5 CARMA Platform Basic Safety Message Parser Header
 *
 * Simulink S-Function source file for converting a byte array to the parsed
 * values of the BSM. Validates messages as a whole, only outputting for 
 * well-formatted messages. Parses BSM Part I data as well as CACC extension
 * data defined by PATH for TOPR5. Exposes all output fields as a Simulink.Bus
 * object.
 */

#ifndef BSM_VALIDATOR_HEADER
#define BSM_VALIDATOR_HEADER

#include <stdint.h>
#include "bsm.h"


// BSM Field Validation Functions
int validateBsmMsgID(uint8_t);
int validateBsmId(uint32_t);
int validateBsmSecMark(uint16_t);
int validateBsmLat(int32_t);
int validateBsmLon(int32_t);
int validateBsmElev(uint16_t);
int validateBsmAccuracy(uint32_t);
int validateBsmSpeedAndTransmission(uint16_t);
int validateBsmHeading(uint16_t);
int validateBsmAngle(uint8_t);
int validateBsmAccelSet(uint8_t*);
int validateBsmBrakes(uint16_t);
int validateBsmSize(uint8_t*);

int validateBsm(BSM);

#endif

