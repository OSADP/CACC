/* TOPR 5 CARMA Platform Basic Safety Message Encoder
 *
 * Encodes the values of the Basic Safety Message and CACC data extension.
 * Takes in the values of the elements in the BSM part 1 and the caccData
 * extension defined in CACC_R0.1.asn and encodes them in DER format for
 * consumption by other vehicles (specifically using bsm_parser.c)
 */


#include <string.h>
#include "tmwtypes.h"
#include "der_encoder.h"
#include "bsm_encoder.h"

/**
 * Encodes a BSM struct into a 214-byte long byte array using DER encoding
 * rules.
 *
 * @params bsm The BSM to be encoded.
 * @return EncodedBSM A struct containing a fixed length 214-byte array
 *                    containing the encoded BSM.
 */
EncodedBSM encodeBSM(BSM bsm) {
	EncodedBSM out = {0};
	uint8_T* next;
	uint8_T* next1;
	
	uint8_T buf[BSM_DATA_BUFFER_SIZE] = {0};
	
	// DSRCMsgID
	next = encodeU8(buf, CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE,
							 MSG_ID_TAG, BSM_MSG_ID);

	// BSMblob
	next = encodeBSMBlob(next, CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE,
								  BSM_BLOB_TAG, bsm);

	// CaccData
	next = encodeCaccData(next, CLASS_CONTEXT_SPECIFIC |CLASS_CONSTRUCTED,
								   CACC_DATA_TAG, bsm);

	// Encode top-level BSM structure
	next1 = encodeSequence(out.buf, CLASS_UNIVERSAL | CLASS_CONSTRUCTED,
								   TOP_LEVEL_TAG, next - buf);

	memcpy(next1, buf, next - buf);
	out.length = next1 + (next - buf) - out.buf;
	
	return out;
}

/**
 * Encode the a BSM BLOB object at the current location.
 *
 * @params loc The location to begin the encoding
 * @params tag The ASN.1 tag to use
 * @params BSM the BSM struct to pull data from
 * @return uint8_T* A pointer to the next available space in the buffer for
 *                  encoding.
 */
uint8_T* encodeBSMBlob(uint8_T* loc, uint8_T class, uint32_T tag, BSM bsm) {
	uint8_T blob[BSM_BLOB_SIZE] = {0};
	uint8_T* next;
	uint32_T offset = 0;

	// BSM Blob Packing
	memcpy(blob, &bsm.msgCnt, 1);
	offset += 1;
	memcpy(blob + offset, &bsm.id, 4);
	offset += 4;
	memcpy(blob + offset, &bsm.secMark, 2);
	offset += 2;
	memcpy(blob + offset, &bsm.lat, 4);
	offset += 4;
	memcpy(blob + offset, &bsm.lon, 4);
	offset += 4;
	memcpy(blob + offset, &bsm.elev, 2);
	offset += 2;
	memcpy(blob + offset, &bsm.accuracy, 4);
	offset += 4;
	memcpy(blob + offset, &bsm.speed, 2);
	offset += 2;
	memcpy(blob + offset, &bsm.heading, 2);
	offset += 2;
	memcpy(blob + offset, &bsm.angle, 1);
	offset += 1;
	memcpy(blob + offset, &bsm.accelSet, 7);
	offset += 7;
	memcpy(blob + offset, &bsm.brakes, 2);
	offset += 2;
	memcpy(blob + offset, &bsm.size, 3);

	// Encode the actual packed bytes in the ASN.1 structure.
	next = encodeU8Arr(loc, class, tag, blob, BSM_BLOB_SIZE);

	return next;
}

/**
 * Encode the CaccData extension to the BSM.
 *
 * @params loc The location to encode the CaccData Extension
 * @params tag The ASN.1 tag to use
 * @params bsm the BSM struct to pull data from
 * @return uint8_T* A pointer to the next available space in the buffer for
 *                  encoding.
 */
uint8_T* encodeCaccData(uint8_T* loc, uint8_T class, uint32_T tag, BSM bsm) {
	// Temporary data storage buffer
	uint8_T buf[CACC_DATA_BUFFER_SIZE] = {0};
	uint8_T* next1;
	
	// Encode the actual values
	uint8_T* next = encodeU8(buf,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 4,
			 bsm.flags);
	next = encodeU16(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 5,
			 bsm.setSpeed);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE,  6,
			 bsm.throtPos);
	next = encodeF32(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 7,
			 bsm.lclPN);
	next = encodeF32(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 8,
			 bsm.lclPE);
	next = encodeF32(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 9,
			 bsm.lclPD);
	next = encodeI16(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 10,
			 bsm.roll);
	next = encodeI16(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 11,
			 bsm.pitch);
	next = encodeI16(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 12,
			 bsm.yaw);
	next = encodeF32(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 13,
			 bsm.hPosAcry);
	next = encodeF32(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 14,
			 bsm.vPosAcry);
	next = encodeF32(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 15,
			 bsm.fwrdVel);
	next = encodeF32(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 16,
			 bsm.rightVel);
	next = encodeF32(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 17,
			 bsm.downVel);
	next = encodeF32(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 18,
			 bsm.velAcc);
	next = encodeI16(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 19,
			 bsm.fwrdAcc);
	next = encodeI16(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 20,
			 bsm.rightAcc);
	next = encodeI16(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 21,
			 bsm.dwnAcc);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 22,
			 bsm.grpID);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 23,
			 bsm.grpSize);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 24,
			 bsm.grpMode);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 25,
			 bsm.grpManDes);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 26,
			 bsm.grpManID);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 27,
			 bsm.vehID);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 28,
			 bsm.frntCutIn);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 29,
			 bsm.vehGrpPos);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 30,
			 bsm.vehFltMode);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 31,
			 bsm.vehManDes);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 32,
			 bsm.vehManID);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 33,
			 bsm.distToPVeh);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 34,
			 bsm.relSpdPVeh);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 35,
			 bsm.distToLVeh);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 36,
			 bsm.relSpdLVeh);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 37,
			 bsm.desTGapPVeh);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 38,
			 bsm.desTGapLVeh);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 39,
			 bsm.estDisPVeh);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 40,
			 bsm.estDisLVeh);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 41,
			 bsm.desSpd);
	next = encodeU16(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 42,
			 bsm.desTrq);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 43,
			 bsm.userDE1);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 44,
			 bsm.userDE2);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 45,
			 bsm.userDE3);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 46,
			 bsm.userDE4);
	next = encodeU8(next,
			 CLASS_CONTEXT_SPECIFIC | CLASS_PRIMITIVE, 47,
			 bsm.userDE5);

	next1 = encodeSequence(loc, CLASS_CONTEXT_SPECIFIC | CLASS_CONSTRUCTED,
									tag, next - buf);
	memcpy(next1, buf, next - buf);

	return next1 + (next - buf);
}

