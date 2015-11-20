/**************************************************************************
 *
 * Copyright (c) 1996,1997,2015   Regents of the University of California
 *
 **************************************************************************/

#define	S_FUNCTION_NAME		kl_fb_3	
#define	S_FUNCTION_LEVEL	2		


#include "simstruc.h"
#include "math.h"


static double	id_in_3;
static double	sp_in_3;

static double ref_3;
static double	ref_prev_3;

static double kpl_3 = 0.1;
static double kdl_3 = 0.05;



static	void	mdlInitializeSizes( SimStruct *S )	// standar def in simstruc.h
{
	long	board_base;								

	ssSetNumSFcnParams( S, 0 );						
	ssSetNumContStates( S, 0 );						
	ssSetNumDiscStates( S, 0 );						
	ssSetNumInputPorts( S, 1 );						
	ssSetInputPortWidth( S, 0,  2 );				
													
	ssSetInputPortDirectFeedThrough( S, 0, 1 );		
	ssSetNumOutputPorts( S, 1 );					
	ssSetOutputPortWidth( S, 0, 1  );			
													
	ssSetNumSampleTimes( S, 1 );					
	ssSetNumRWork( S, 0 );				
	ssSetNumIWork( S, 0 );				
	ssSetNumPWork( S, 0 );				
	ssSetNumModes( S, 0 );				
	ssSetNumNonsampledZCs( S, 0 );		
	ssSetOptions(S,	0);					

}

static	void	mdlInitializeSampleTimes( SimStruct *S )
{
	ssSetSampleTime( S, 0, INHERITED_SAMPLE_TIME);		// 0.05				
	ssSetOffsetTime( S, 0, 0.0 );						
}

static	void	mdlOutputs( SimStruct *S, int_T tid )	// standard name in simulink.c
{
	InputRealPtrsType	uPtrs =	ssGetInputPortRealSignalPtrs( S, 0 );
	real_T			*y = ssGetOutputPortRealSignal( S, 0 );	
	

	id_in_3 = *uPtrs[0];
	sp_in_3 = *uPtrs[1];
    
    if (sp_in_3 < 6.0) // original: 4; ==> 6.0
    {
        sp_in_3 = 6.0;
    }
    
    ref_3 = id_in_3 - sp_in_3;  // Gap differnce: (desired - measured) of the subject veh (i.e. 3rd veh) w.r.t to the leader
    
    y[0] = kpl_3 * ref_3 + kdl_3 * (ref_3-ref_prev_3);	   // P+D spd control only; distance regultion w.r.t. the leader
    
    ref_prev_3 = ref_3;


}


static	void	mdlTerminate( SimStruct *S )		// standard name in simulink.c	
{
}

#ifdef	MATLAB_MEX_FILE						
	#include	"simulink.c"				
#else								
	#include	"cg_sfun.h"				
#endif
