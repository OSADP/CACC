/* TOPR 5 CARMA Platform Basic Safety Message Validator
 *
 * Validates the contents of a DSRC Basic Safety Message.
 * Performs a field-by-field check of values, ranges, and formats based on J2735
 * standards for DSRC communication, specifically section 5.2 and all referenced
 * fields.
 */

#include <string.h>
#include "tmwtypes.h"
#include "bsm.h"
#include "der_decoder.h"
#include "bsm_parser.h"

BSM parseBasicSafetyMessage(uint8_T* buf, uint32_T len) {
	BSM			bsm;
	uint32_T	offset;
    AsnObject   topLevel;
	AsnObject	msgID;
	AsnObject	bsmBlob;
	AsnObject	caccData;
	uint8_T		msgCnt;	 
	uint32_T	id;		 
	uint16_T	secMark;	 
	int32_T		lat;		 
	int32_T		lon;		 
	int16_T		elev;	 
	uint32_T	accuracy;
	uint16_T	speed;
	uint16_T	heading;
	uint8_T		angle;	 
	uint8_T	    accelSet[7];
	uint16_T	brakes;
	uint8_T		size[3];
	AsnObject	caccFlags_de;   
	AsnObject	setSpeed_de;	   
	AsnObject	throtPos_de;	   
	AsnObject	lclPN_de;	   
	AsnObject   lclPE_de;       
	AsnObject	lclPD_de;	   
	AsnObject	roll_de;		   
	AsnObject	pitch_de;	   
	AsnObject	yaw_de;		   
	AsnObject	hPosAcry_de;	   
	AsnObject	vPosAcry_de;	   
	AsnObject   fwrdVel_de;	   
	AsnObject	rightVel_de;	   
	AsnObject	downVel_de;	   
	AsnObject	velAcc_de;	   
	AsnObject	fwrdAcc_de;	   
	AsnObject	rightAcc_de;	   
	AsnObject	dwnAcc_de;	   
	AsnObject	grpID_de;	   
	AsnObject	grpSize_de;	   
	AsnObject	grpMode_de;	   
	AsnObject	grpManDes_de;   
	AsnObject	grpManID_de;	   
	AsnObject	vehID_de;	   
	AsnObject	frntCutIn_de;   
	AsnObject	vehGrpPos_de;   
	AsnObject	vehFltMode_de;  
	AsnObject	vehManDes_de;   
	AsnObject	vehManID_de;	   
	AsnObject	distToPVeh_de;  
	AsnObject	relSpdPVeh_de;  
	AsnObject	distToLVeh_de;  
	AsnObject	relSpdLVeh_de;  
	AsnObject	desTGapPVeh_de; 
	AsnObject	desTGapLVeh_de; 
	AsnObject	estDisPVeh_de;  
	AsnObject	estDisLVeh_de;  
	AsnObject	desSpd_de;	   
	AsnObject	desTrq_de;
	AsnObject   userDE1_de;
	AsnObject   userDE2_de;
	AsnObject   userDE3_de;
	AsnObject   userDE4_de;
	AsnObject   userDE5_de;
	uint8_T		caccFlags;	
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
	uint8_T		grpID;		
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
	uint8_T		desTrq;
	uint8_T     userDE1;
	uint8_T     userDE2;
	uint8_T     userDE3;
	uint8_T     userDE4;
	uint8_T     userDE5;
	
	// Decode the ASN.1 top-level structure
	offset	 = 0;
    topLevel = parseAsnObject(buf, &offset);
    
    offset = 0;
	msgID	 = parseAsnObject(topLevel.value, &offset);
	bsmBlob	 = parseAsnObject(topLevel.value + offset, &offset);
	caccData = parseAsnObject(topLevel.value + offset, &offset);

	// Decode the BSM blob itself
	memcpy(&msgCnt, &(bsmBlob.value[0]), 1);
	memcpy(&id, &(bsmBlob.value[1]), 4);
	memcpy(&secMark, &(bsmBlob.value[5]), 2);
	memcpy(&lat, &(bsmBlob.value[7]), 4);
	memcpy(&lon, &(bsmBlob.value[11]), 4);
	memcpy(&elev, &(bsmBlob.value[15]), 2);
	memcpy(&accuracy, &(bsmBlob.value[17]), 4);
	memcpy(&speed, &(bsmBlob.value[21]), 2);
	memcpy(&heading, &(bsmBlob.value[23]), 2);
	memcpy(&angle, &(bsmBlob.value[25]), 1);
	memcpy(accelSet, &bsmBlob.value[26], 7);
	memcpy(&brakes, &(bsmBlob.value[33]), 2);
	memcpy(size, &bsmBlob.value[35], 3);


	// Decode caccData BSM part II replacement/extension.
	offset = 0;
	caccFlags_de   = parseAsnObject(caccData.value, &offset);
	setSpeed_de	   = parseAsnObject(caccData.value + offset, &offset);
	throtPos_de	   = parseAsnObject(caccData.value + offset, &offset);
	lclPN_de	   = parseAsnObject(caccData.value + offset, &offset);
	lclPE_de       = parseAsnObject(caccData.value + offset, &offset);
	lclPD_de	   = parseAsnObject(caccData.value + offset, &offset);
	roll_de		   = parseAsnObject(caccData.value + offset, &offset);
	pitch_de	   = parseAsnObject(caccData.value + offset, &offset);
	yaw_de		   = parseAsnObject(caccData.value + offset, &offset);
	hPosAcry_de	   = parseAsnObject(caccData.value + offset, &offset);
	vPosAcry_de	   = parseAsnObject(caccData.value + offset, &offset);
	fwrdVel_de	   = parseAsnObject(caccData.value + offset, &offset);
	rightVel_de	   = parseAsnObject(caccData.value + offset, &offset);
	downVel_de	   = parseAsnObject(caccData.value + offset, &offset);
	velAcc_de	   = parseAsnObject(caccData.value + offset, &offset);
	fwrdAcc_de	   = parseAsnObject(caccData.value + offset, &offset);
	rightAcc_de	   = parseAsnObject(caccData.value + offset, &offset);
	dwnAcc_de	   = parseAsnObject(caccData.value + offset, &offset);
	grpID_de	   = parseAsnObject(caccData.value + offset, &offset);
	grpSize_de	   = parseAsnObject(caccData.value + offset, &offset);
	grpMode_de	   = parseAsnObject(caccData.value + offset, &offset);
	grpManDes_de   = parseAsnObject(caccData.value + offset, &offset);
	grpManID_de	   = parseAsnObject(caccData.value + offset, &offset);
	vehID_de	   = parseAsnObject(caccData.value + offset, &offset);
	frntCutIn_de   = parseAsnObject(caccData.value + offset, &offset);
	vehGrpPos_de   = parseAsnObject(caccData.value + offset, &offset);
	vehFltMode_de  = parseAsnObject(caccData.value + offset, &offset);
	vehManDes_de   = parseAsnObject(caccData.value + offset, &offset);
	vehManID_de	   = parseAsnObject(caccData.value + offset, &offset);
	distToPVeh_de  = parseAsnObject(caccData.value + offset, &offset);
	relSpdPVeh_de  = parseAsnObject(caccData.value + offset, &offset);
	distToLVeh_de  = parseAsnObject(caccData.value + offset, &offset);
	relSpdLVeh_de  = parseAsnObject(caccData.value + offset, &offset);
	desTGapPVeh_de = parseAsnObject(caccData.value + offset, &offset);
	desTGapLVeh_de = parseAsnObject(caccData.value + offset, &offset);
	estDisPVeh_de  = parseAsnObject(caccData.value + offset, &offset);
	estDisLVeh_de  = parseAsnObject(caccData.value + offset, &offset);
	desSpd_de	   = parseAsnObject(caccData.value + offset, &offset);
	desTrq_de	   = parseAsnObject(caccData.value + offset, &offset);
	userDE1_de     = parseAsnObject(caccData.value + offset, &offset);
	userDE2_de     = parseAsnObject(caccData.value + offset, &offset);
	userDE3_de     = parseAsnObject(caccData.value + offset, &offset);
	userDE4_de     = parseAsnObject(caccData.value + offset, &offset);
	userDE5_de     = parseAsnObject(caccData.value + offset, &offset);

	// Decode the value fields of the ASN objects
	memcpy(&caccFlags, caccFlags_de.value, 1);
	memcpy(&setSpeed, setSpeed_de.value, 2);
	memcpy(&throtPos, throtPos_de.value, 1);
	memcpy(&lclPN, lclPN_de.value, 4);
	memcpy(&lclPE, lclPE_de.value, 4);
	memcpy(&lclPD, lclPD_de.value, 4);
	memcpy(&roll, roll_de.value, 2);
	memcpy(&pitch, pitch_de.value, 2);
	memcpy(&yaw, yaw_de.value, 2);
	memcpy(&hPosAcry, hPosAcry_de.value, 4);
	memcpy(&vPosAcry, vPosAcry_de.value, 4);
	memcpy(&fwrdVel, fwrdVel_de.value, 4);
	memcpy(&rightVel, rightVel_de.value, 4);
	memcpy(&downVel, downVel_de.value, 4);
	memcpy(&velAcc, velAcc_de.value, 4);
	memcpy(&fwrdAcc, fwrdAcc_de.value, 2);
	memcpy(&rightAcc, rightAcc_de.value, 2);
	memcpy(&dwnAcc, dwnAcc_de.value, 2);

	grpID		= *grpID_de.value;
	grpSize		= *grpSize_de.value;
	grpMode		= *grpMode_de.value;
	grpManDes	= *grpManDes_de.value;
	grpManID	= *grpManID_de.value;
	vehID		= *vehID_de.value;
	frntCutIn	= *frntCutIn_de.value;
	vehGrpPos	= *vehGrpPos_de.value;
	vehFltMode	= *vehFltMode_de.value;
	vehManDes	= *vehManDes_de.value;
	vehManID	= *vehManID_de.value;
	distToPVeh	= *distToPVeh_de.value;
	relSpdPVeh	= *relSpdPVeh_de.value;
	distToLVeh	= *distToLVeh_de.value;
	relSpdLVeh	= *relSpdLVeh_de.value;
	desTGapPVeh = *desTGapPVeh_de.value;
	desTGapLVeh = *desTGapLVeh_de.value;
	estDisPVeh	= *estDisPVeh_de.value;
	estDisLVeh	= *estDisLVeh_de.value;
	desSpd		= *desSpd_de.value;
	memcpy(&desTrq, desTrq_de.value, 2);
	userDE1     = *userDE1_de.value;
	userDE2     = *userDE2_de.value;
	userDE3     = *userDE3_de.value;
	userDE4     = *userDE4_de.value;
	userDE5     = *userDE5_de.value;
	 
	// Store these values in the BSM struct
	bsm.msgID	 = *msgID.value;
	bsm.msgCnt	 = msgCnt;
	bsm.id		 = id;
	bsm.secMark	 = secMark;
	bsm.lat		 = lat;
	bsm.lon		 = lon;
	bsm.elev	 = elev;
	bsm.accuracy = accuracy;
	bsm.speed	 = speed;
	bsm.heading	 = heading;
	bsm.angle	 = angle;
	memcpy(bsm.accelSet, accelSet, 7);
	bsm.brakes	 = brakes;
	memcpy(bsm.size, size, 3);

	bsm.flags		= caccFlags;
	bsm.setSpeed	= setSpeed;
	bsm.throtPos	= throtPos;
	bsm.lclPN		= lclPN;
	bsm.lclPE		= lclPE;
	bsm.lclPD		= lclPD;
	bsm.roll		= roll;
	bsm.yaw			= yaw;
	bsm.pitch		= pitch;
	bsm.hPosAcry	= hPosAcry;
	bsm.vPosAcry	= vPosAcry;
	bsm.fwrdVel		= fwrdVel;
	bsm.rightVel	= rightVel;
	bsm.downVel		= downVel;
	bsm.velAcc		= velAcc;
	bsm.fwrdAcc		= fwrdAcc;
	bsm.rightAcc	= rightAcc;
	bsm.dwnAcc		= dwnAcc;
	bsm.grpID		= grpID;
	bsm.grpSize		= grpSize;
	bsm.grpMode		= grpMode;
	bsm.grpManDes	= grpManDes;
	bsm.grpManID	= grpManID;
	bsm.vehID		= vehID;
	bsm.frntCutIn	= frntCutIn;
	bsm.vehGrpPos	= vehGrpPos;
	bsm.vehFltMode	= vehFltMode;
	bsm.vehManDes	= vehManDes;
	bsm.vehManID	= vehManID;
	bsm.distToPVeh	= distToPVeh;
	bsm.relSpdPVeh	= relSpdPVeh;
	bsm.distToLVeh	= distToLVeh;
	bsm.relSpdLVeh	= relSpdLVeh;
	bsm.desTGapPVeh = desTGapPVeh;
	bsm.desTGapLVeh = desTGapLVeh;
	bsm.estDisPVeh	= estDisPVeh;
	bsm.estDisLVeh	= estDisLVeh;
	bsm.desSpd		= desSpd;
	bsm.desTrq		= desTrq;
	bsm.userDE1     = userDE1;
	bsm.userDE2     = userDE2;
	bsm.userDE3     = userDE3;
	bsm.userDE4     = userDE4;
	bsm.userDE5     = userDE5;

	return bsm;
}

