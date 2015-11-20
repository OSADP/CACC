/*
  *
  *   --- THIS FILE GENERATED BY S-FUNCTION BUILDER: 3.0 ---
  *
  *   This file is a wrapper S-function produced by the S-Function
  *   Builder which only recognizes certain fields.  Changes made
  *   outside these fields will be lost the next time the block is
  *   used to load, edit, and resave this file. This file will be overwritten
  *   by the S-function Builder block. If you want to edit this file by hand, 
  *   you must change it only in the area defined as:  
  *
  *        %%%-SFUNWIZ_wrapper_XXXXX_Changes_BEGIN 
  *            Your Changes go here
  *        %%%-SFUNWIZ_wrapper_XXXXXX_Changes_END
  *
  *   For better compatibility with the Simulink Coder, the
  *   "wrapper" S-function technique is used.  This is discussed
  *   in the Simulink Coder User's Manual in the Chapter titled,
  *   "Wrapper S-functions".
  *
  *   Created: Tue Aug 18 15:54:54 2015
  */


/*
 * Include Files
 *
 */
#if defined(MATLAB_MEX_FILE)
#include "tmwtypes.h"
#include "simstruc_types.h"
#else
#include "rtwtypes.h"
#endif
#include "ParseBSM_bus.h"

/* %%%-SFUNWIZ_wrapper_includes_Changes_BEGIN --- EDIT HERE TO _END */
#include <math.h>
#include "bsm_parser.h"
#include <string.h>

#define NAN 0.0

/* %%%-SFUNWIZ_wrapper_includes_Changes_END --- EDIT HERE TO _BEGIN */
#define u_width 1474
#define y_width 1
/*
 * Create external references here.  
 *
 */
/* %%%-SFUNWIZ_wrapper_externs_Changes_BEGIN --- EDIT HERE TO _END */

real32_T convertRelSpd(uint8_T);
real32_T convertElev(uint16_T);
void convertAccuracy(BasicSafetyMessage*, uint32_T);
void convertTransmissionAndSpeed(BasicSafetyMessage*, uint16_T);
void convertSteeringWheelAngle(BasicSafetyMessage*, uint8_T);
void convertAccelSet(BasicSafetyMessage*, uint8_T*);
void convertBrakes(BasicSafetyMessage*, uint16_T);
void convertSize(BasicSafetyMessage*, uint8_T*);

/* %%%-SFUNWIZ_wrapper_externs_Changes_END --- EDIT HERE TO _BEGIN */

/*
 * Output functions
 *
 */

void ParseBSM_Outputs_wrapper(const uint8_T *Buffer,
                          const uint32_T *Length,
                          BasicSafetyMessage *simBSM)
{
/* %%%-SFUNWIZ_wrapper_Outputs_Changes_BEGIN --- EDIT HERE TO _END */
	
	BSM rawMsg;

	// Parse the binary into the raw BSM values
	rawMsg = parseBasicSafetyMessage(Buffer, *Length);

	// Convert to proper formats for usage by Simulink
	simBSM->caccFlags_bitmask = rawMsg.flags;
	simBSM->setSpeed_m_s = rawMsg.setSpeed * 0.02;
	simBSM->throtPos_percent = rawMsg.throtPos * 0.5;
	simBSM->lclPN_mm = rawMsg.lclPN;
	simBSM->lclPE_mm = rawMsg.lclPE;
	simBSM->lclPD_mm = rawMsg.lclPD;
	simBSM->roll_deg = rawMsg.roll * (1.0/32768.0);
	simBSM->pitch_deg = rawMsg.pitch * (1.0/32768.0);
	simBSM->yaw_deg = rawMsg.yaw * (1.0/32768.0);
	simBSM->hPosAcry_m = rawMsg.hPosAcry;
	simBSM->vPosAcry_m = rawMsg.vPosAcry;
	simBSM->frwdVel_m_s = rawMsg.fwrdVel;
	simBSM->rightVel_m_s = rawMsg.rightVel;
	simBSM->downVel_m_s = rawMsg.downVel;
	simBSM->velAcc_m_s = rawMsg.velAcc;
	simBSM->frwdAcc_mm_s_s = rawMsg.fwrdAcc;
	simBSM->rightAcc_mm_s_s = rawMsg.rightAcc;
	simBSM->dwnAcc_mm_s_s = rawMsg.dwnAcc;
	simBSM->grpID = rawMsg.grpID;
	simBSM->grpSize = rawMsg.grpSize;
	simBSM->grpMode = rawMsg.grpMode;
	simBSM->grpManDes = rawMsg.grpManDes;
	simBSM->grpManID = rawMsg.grpManID;
	simBSM->vehID = rawMsg.vehID;
	simBSM->frntCutIn = rawMsg.frntCutIn;
	simBSM->vehGrpPos = rawMsg.vehGrpPos;
	simBSM->vehFltMode = rawMsg.vehFltMode;
	simBSM->vehManDes = rawMsg.vehManDes;
	simBSM->vehManID = rawMsg.vehManID;
	simBSM->distToPVeh_m = rawMsg.distToPVeh;
	simBSM->relSpdPVeh_m_s = convertRelSpd(rawMsg.relSpdPVeh);
	simBSM->distToLVeh_m = rawMsg.distToLVeh;
	simBSM->relSpdLVeh_m_s = convertRelSpd(rawMsg.relSpdLVeh);
	simBSM->desTGapPVeh_s = rawMsg.desTGapPVeh * 0.1;
	simBSM->desTGapLVeh_s = rawMsg.desTGapLVeh * 0.1;
	simBSM->estDisPVeh_m = rawMsg.estDisPVeh;
	simBSM->estDisLVeh_m = rawMsg.estDisLVeh;
	simBSM->desSpeed_m_s = rawMsg.desSpd;
	simBSM->desTrq_N_m = rawMsg.desTrq;

	simBSM->msgID = rawMsg.msgID;
	simBSM->msgCnt = rawMsg.msgCnt;
	simBSM->id = rawMsg.id;
	simBSM->secMark_ms = rawMsg.secMark;
	simBSM->lat_deg = rawMsg.lat / 10000000.0;
	simBSM->lon_deg = rawMsg.lon / 10000000.0;
	simBSM->elev_m = convertElev(rawMsg.elev);
	convertAccuracy(simBSM, rawMsg.accuracy);
	convertTransmissionAndSpeed(simBSM, rawMsg.speed);
	simBSM->heading_deg = rawMsg.heading * 0.0125;
	convertSteeringWheelAngle(simBSM, rawMsg.angle);
	convertAccelSet(simBSM, rawMsg.accelSet);
	convertBrakes(simBSM, rawMsg.brakes);
	convertSize(simBSM, rawMsg.size);
    simBSM->userDE1 = rawMsg.userDE1;
    simBSM->userDE2 = rawMsg.userDE2;
    simBSM->userDE3 = rawMsg.userDE3;
    simBSM->userDE4 = rawMsg.userDE4;
    simBSM->userDE5 = rawMsg.userDE5;
}

real32_T convertRelSpd(uint8_T encoded) {
	return (encoded - 64) * 0.625;
}

real32_T convertElev(uint16_T encoded) {
	if (encoded >= 0x0 && encoded <= 0xEFFF) {
		// Above the reference ellipsoid
		return encoded * 0.1;
	} else if (encoded >= 0xF001 && encoded <= 0xFFFF) {
		// Below the reference ellipsoid
		return ((encoded - 0xF001) * 0.1) - 409.5;
	} else {
		// Unknown elevation
		return NAN; // Throw a NaN
	}
}

void convertAccuracy(BasicSafetyMessage* output, uint32_T accuracy) {
	output->semi_major_accuracy_m = ((accuracy >> 24) & 0xFF)* 0.05;
	output->semi_minor_accuracy_m = ((accuracy >> 16) & 0xFF) * 0.05;
	output->semi_major_accuracy_orientation_deg
		= (accuracy & 0x0000FFFF) * 0.0054932479;
}

void convertTransmissionAndSpeed(BasicSafetyMessage* output, uint16_T speed) {
	uint16_T speed1;
	output->transmission_enum = (speed  >> 14) & 0x3;
	speed1 = (speed & 0x3FFF);
	if (speed1 < 8191) {
		output->speed_m_s = speed1 * 0.02;
	} else {
		output->speed_m_s = NAN; // Throw a NaN
	}
}

void convertSteeringWheelAngle(BasicSafetyMessage* output, uint8_T angle) {
	if (angle == 0x7F) {
		output->angle_deg = NAN;
	} else {
		int8_T angle1 = (int8_T) angle;
		output->angle_deg = angle1 * 1.5;
	}
}

void convertAccelSet(BasicSafetyMessage* output, uint8_T* accelSet) {
	int16_T lonAcc;
	int16_T latAcc;
	int8_T vertAcc;
	int16_T yawRate;

	memcpy(&lonAcc, accelSet, 2);
	memcpy(&latAcc, accelSet + 2, 2);
	memcpy(&vertAcc, accelSet + 4, 1);
	memcpy(&yawRate, accelSet + 5, 2);

	output->lat_acceleration_m_s_s = latAcc * 0.01;
	output->long_acceleration_m_s_s = lonAcc * 0.01;
	output->vertical_acceleration_g = (vertAcc - 50) * 0.02;
	output->yaw_rate_deg_s = yawRate * 0.01;
}

void convertBrakes(BasicSafetyMessage* output, uint16_T brakes) {
	output->wheelBrakes_bitmask = ((brakes >> 12) & 0xF);
	output->wheelBrakesUnavailable = ((brakes >> 11) & 0x1);
	output->tractionControlState_enum = ((brakes >> 8) & 0x3);;
	output->anti_lock_brake_status_enum = ((brakes >> 6) & 0x3);
	output->stability_control_status_enum = ((brakes >> 4) & 0x3);
	output->brake_boost_enum = ((brakes >> 2) & 0x3);
	output->auxiliary_brakes_status_enum = (brakes & 0x3);
}

void convertSize(BasicSafetyMessage* output, uint8_T* size) {
	uint8_T width;
	uint16_T length;

	memcpy(&width, size, 1);
	memcpy(&length, size + 1, 2);

	output->vehicle_width_cm = width;

	// TODO: Fix, should be vehicle_height_cm
	output->vehicle_height_cm = length;
}

/* %%%-SFUNWIZ_wrapper_Outputs_Changes_END --- EDIT HERE TO _BEGIN */
