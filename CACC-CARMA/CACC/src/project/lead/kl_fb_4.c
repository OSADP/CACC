/**************************************************************************
 *
 * Copyright (c) 1996,1997,2015   Regents of the University of California
 *
 **************************************************************************/

#define	S_FUNCTION_NAME		kl_fb_4 	
#define	S_FUNCTION_LEVEL	2		


#include "simstruc.h"
#include "math.h"
//#include "size.h"

static double	id_in_4;
static double	sp_in_4;

static double ref_4;
static double	ref_prev_4;

static double kpl_4 = 0.1;
static double kdl_4 = 0.05;



static	void	mdlInitializeSizes( SimStruct *S )	
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
	ssSetSampleTime( S, 0, INHERITED_SAMPLE_TIME );	  // 0.05					
	ssSetOffsetTime( S, 0, 0.0 );						
}

static	void	mdlOutputs( SimStruct *S, int_T tid )	
{
	InputRealPtrsType	uPtrs =	ssGetInputPortRealSignalPtrs( S, 0 );
	real_T			*y = ssGetOutputPortRealSignal( S, 0 );	
	

	id_in_4 = *uPtrs[0];
	sp_in_4 = *uPtrs[1];
    
    if (sp_in_4 < 4)
    {
        sp_in_4 = 4;
    }
    
    ref_4 = id_in_4 - sp_in_4;  // Gap differnce: (desired - measured) of the subject veh (i.e. 3rd veh) w.r.t to the leader
    
    y[0] = kpl_4 * ref_4 + kdl_4 * (ref_4-ref_prev_4);		 // P+D spd control only; distance regultion w.r.t. the leader
    
    ref_prev_4 = ref_4;


}

static	void	mdlTerminate( SimStruct *S )			
{
}

#ifdef	MATLAB_MEX_FILE						
	#include	"simulink.c"				
#else								
	#include	"cg_sfun.h"				
#endif
