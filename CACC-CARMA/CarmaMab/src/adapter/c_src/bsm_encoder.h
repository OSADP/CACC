/* TOPR 5 BSM Encoder Header
 * Defines the types used by the BSM encoder as well as providing function
 * definitions for encoding functions.
 */

#ifndef __BSM_ENCODER_H__
#define __BSM_ENCODER_H__

#include "tmwtypes.h"
#include "bsm.h"

#define BSM_MSG_ID 2
#define BSM_BLOB_SIZE 38
#define CACC_DATA_SIZE 169
#define BSM_DATA_BUFFER_SIZE 1472
#define CACC_DATA_BUFFER_SIZE 200

// ASN.1 BSM Tag Numbers
#define TOP_LEVEL_TAG 16
#define MSG_ID_TAG 0
#define BSM_BLOB_TAG 1
#define CACC_DATA_TAG 2

typedef struct {
	uint8_T buf[BSM_DATA_BUFFER_SIZE];
	uint32_T length;
} EncodedBSM;

EncodedBSM encodeBSM(BSM);
uint8_T* encodeBSMBlob(uint8_T*, uint8_T, uint32_T, BSM);
uint8_T* encodeCaccData(uint8_T*, uint8_T, uint32_T, BSM);

#endif
   
