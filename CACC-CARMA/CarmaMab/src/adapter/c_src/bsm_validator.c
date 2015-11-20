/* TOPR 5 CARMA Platform Basic Safety Message Validator
 *
 * Validates the contents of a DSRC Basic Safety Message.
 * Performs a field-by-field check of values, ranges, and formats based on J2735
 * standards for DSRC communication, specifically section 5.2 and all referenced
 * fields.
 */

#include <stdint.h>
#include "bsm.h"
#include "der_decoder.h"
#include "bsm_validator.h"

/**
 * Validates the MsgId component.
 *
 * Ensures the value of MsgId is equal to 2 (the ID for BSM)
 * @params msgID The msgID component of the BSM
 * @return int Non-zero if the msgID is valid, zero otherwise.
 */ 
int validateBsmMsgID(uint8_t msgID) {
	return msgID == 2;
}

/**
 * Validates the id component.
 *
 * @params id The temporary ID component of the BSM
 * @return int Non-zero if the id is valid, zero otherwise.
 */
int validateBsmId(uint32_t id) {
	// All 4 byte sequences are valid identifiers
	return 1;
}

/**
 * Validates the SecMark component of the BSM
 *
 * Ensures the value of SecMark is within the range [0, 65535]
 * @params secMark The secMark component of the BSM
 * @return int Non-zero if the secMark is valid, zero otherwise.
 */
int validateBsmSecMark(uint16_t secMark) {
    return (secMark >= 0) && (secMark <= 65535);
}

/**
 * Validates the Latitude component of the BSM
 *
 * Ensures the value of Latitude is within the range [-900000000, 900000001]
 * @params lat The latitude component of the BSM
 * @return int Non-zero if the lat is valid, zero otherwise.
 */
int validateBsmLat(int32_t lat) {
    return (lat >= -900000000) && (lat <= 900000001);
}

/**
 * Validates the Longitude component of the BSM
 *
 * Ensures the value of the Longitude is within the range [-1800000000, 1800000001]
 * @params lon The longitude component of the BSM
 * @return int Non-zero if the lon is valid, zero otherwise.
 */
int validateBsmLon(int32_t lon) {
    return (lon >= -1800000000) && (lon <= 1800000001);
}

/**
 * Validates the Elevation component of the BSM
 *
 * @params elev The elevation component of the BSM
 * @return int Non-zero if the elev is valid, zero otherwise.
 */
int validateBsmElev(uint16_t elev) {
	// All 2 byte sequences are valid elevation values.
    return 1;
}

/**
 * Validates the Accuracy component of the BSM
 *
 * @params accuracy The accuracy component of the BSM
 * @return int Non-zero if the accuracy is valid, zero otherwise.
 */
int validateBsmAccuracy(uint32_t accuracy) {
	// Any 4 byte sequence is a valid BSM accuracy value
    return 1;
}

/**
 * Validates the Speed and Transmission component of the BSM
 *
 * @params speed  The speed and transmission components of the BSM
 * @return int Non-zero if the speed and transmission state is valid, zero otherwise.
 */
int validateBsmSpeedAndTransmission(uint16_t speedAndTransmission) {
	// Parse the individual data elements out of the short.
    uint16_t transmission = (speedAndTransmission << 13) >> 16;
	uint16_t speed = speedAndTransmission >> 3;
	
	return ((speed >= 0 && speed <= 8191) &&
			(transmission >= 0 && transmission <= 7));
	
}

/**
 * Validates the Heading component of the BSM
 *
 * @params heading The heading component of the BSM
 * @return int Non-zero if the heading is valid, zero otherwise.
 */
int validateBsmHeading(uint16_t heading) {
    return (heading >= 0 && heading <= 28800);
}

/**
 * Validates the Angle component of the BSM
 *
 * @params angle The angle component of the BSM
 * @return int Non-zero if the heading is valid, zero otherwise.
 */
int validateBsmAngle(uint8_t angle) {
	// All bytes are valid BSM SteeringWheelAngle
    return 1;
}

/**
 * Validates the AccelSet component of the BSM
 *
 * @params accelSet The accelset component of the BSM
 * @return int Non-zero if the accelSet is valid, zero otherwise.
 */
int validateBsmAccelSet(uint8_t* accelSet) {
	int offset = 0;
	// Parse the ASN.1 DER data frame
	AsnObject accelSet_df = parseAsnObject(accelSet, &offset);

	offset = 0; // Reset to local frame offset, rather than global ASN.1 structure offset.
	AsnObject longAccel_de = parseAsnObject(accelSet_df.value, &offset);
	AsnObject latAccel_de = parseAsnObject(accelSet_df.value + offset, &offset);
	AsnObject vertAccel_de =  parseAsnObject(accelSet_df.value + offset, &offset);
	AsnObject yawRate_de = parseAsnObject(accelSet_df.value + offset, &offset);

	// Cast parsed values to integer types
	int32_t longAccel = (int32_t) (*longAccel_de.value);
	int32_t latAccel = (int32_t) (*latAccel_de.value);
	int32_t vertAccel = (int32_t) (*vertAccel_de.value);
	int32_t yawRate = (int32_t) (*yawRate_de.value);
	

	// Validate their ranges
	return ((longAccel >= -2000 && longAccel <= 2001) &&
			(latAccel >= -2000 && latAccel <= 2001) &&
			(vertAccel >= -127 && vertAccel <= 127) &&
			(yawRate >= -32767 && yawRate <= 32767));
}

/**
 * Validates the Brakes component of the BSM
 *
 * @params brakes The brakes component of the BSM
 * @return int Non-zero if the brakes component is valid, zero otherwise.
 */
int validateBsmBrakes(uint16_t brakes) {
	// All two byte sequences are valid Brake data frames.
    return 1;
}

/**
 * Validates the Size component of the BSM
 *
 * @params size The size component of the BSM
 * @return int Non-zero if the size component is valid, zero otherwise.
 */
int validateBsmSize(uint8_t* size) {
	int offset = 0;
	
    // TODO: Validate my assumption about the size of these BSM elements
	AsnObject size_df = parseAsnObject(size, &offset);

	// TODO: Verify the validity of this pointer arithmetic
	int offset1 = 0;
	AsnObject width_de = parseAsnObject(size_df.value, &offset1);
	AsnObject length_de = parseAsnObject(size_df.value + offset1, &offset1);

	uint32_t width = (uint32_t) (*width_de.value);
	uint32_t length = (uint32_t) (*length_de.value);

	return ((width >= 0 && width <= 1023) && (length >= 0 && length <= 16383));
}

/**
 * Performs a field-by-field validation of the BSM blob as per J2735
 * Uses the above-defined functions to validate each component in terms of
 * ranges, types, packings, and acceptable values.
 *
 * @params bsm a BSM struct to be validated
 * @return Non-zero if all components of the BSM are valid, zero otherwise.
 */
int validateBsm(BSM bsm) {	
	return (validateBsmMsgID(bsm.msgID) &&
			validateBsmId(bsm.id) &&
			validateBsmSecMark(bsm.secMark) &&
			validateBsmLat(bsm.lat) &&
			validateBsmLon(bsm.lon) &&
			validateBsmElev(bsm.elev) &&
			validateBsmAccuracy(bsm.accuracy) &&
			validateBsmSpeedAndTransmission(bsm.speed) &&
			validateBsmHeading(bsm.heading) &&
			validateBsmAngle(bsm.angle) &&
			validateBsmAccelSet(bsm.accelSet) &&
			validateBsmBrakes(bsm.brakes) &&
			validateBsmSize(bsm.size));
}

