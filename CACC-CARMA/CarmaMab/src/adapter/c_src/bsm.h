/* TOPR 5 BSM Struct Header
 *
 * Defines the BSM struct type for usage by other components in the BSM
 * system.
 */

#ifndef __BSM_HEADER__
#define __BSM_HEADER__
#include "tmwtypes.h"

typedef struct {
	// BSM Message
	uint8_T msgID;

	// BSM BLOB payload
	uint8_T		msgCnt;
	uint32_T	id;
	uint16_T	secMark;
	int32_T		lat;
	int32_T		lon;			// Called "long" in standard, but renamed to avoid keyword
	uint16_T	elev;
	uint32_T	accuracy;
	uint16_T	speed;
	uint16_T	heading;
	uint8_T		angle;
	uint8_T		accelSet[7];
	uint16_T	brakes;
	uint8_T		size[3];

	// CACC Data Extension

	uint8_T		flags;
	uint16_T	setSpeed;
	uint8_T		throtPos;
	float		lclPN;
	float		lclPE;
	float		lclPD;
	int16_T		roll;
	int16_T		pitch;
	int16_T		yaw;
	float		hPosAcry;
	float		vPosAcry;
	float		fwrdVel;
	float		rightVel;
	float		downVel;
	float		velAcc;
	int16_T		fwrdAcc;
	int16_T		rightAcc;
	int16_T		dwnAcc;
	
	uint8_T     grpID;
	uint8_T		grpSize;
	uint8_T		grpMode;
	uint8_T		grpManDes;
	uint8_T		grpManID;
	uint8_T		vehID;
	uint8_T		frntCutIn;
	uint8_T		vehGrpPos;
	uint8_T		vehFltMode;
	uint8_T		vehManDes;
	uint8_T		vehManID;
	uint8_T		distToPVeh;
	uint8_T		relSpdPVeh;
	uint8_T		distToLVeh;
	uint8_T		relSpdLVeh;
	uint8_T		desTGapPVeh;
	uint8_T		desTGapLVeh;
	uint8_T		estDisPVeh;
	uint8_T		estDisLVeh;
	uint8_T		desSpd;
	uint16_T	desTrq;

	// Optional CaccData fields for user defined elements
	uint8_T userDE1;
	uint8_T userDE2;
	uint8_T userDE3;
	uint8_T userDE4;
	uint8_T userDE5;
} BSM;

#endif

