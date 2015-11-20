/**************************************************************************
 *
 * Copyright (c) 1996,1997,2015   Regents of the University of California
 *
 **************************************************************************/


#define	S_FUNCTION_NAME		toggle_sw	
#define	S_FUNCTION_LEVEL	2		

#include "simstruc.h"
#include "math.h"



static double  sw_count=0.0;

static double mod_sw;


static	void	mdlInitializeSizes( SimStruct *S )	
{
	long	board_base;								

	ssSetNumSFcnParams( S, 0 );						
	ssSetNumContStates( S, 0 );						
	ssSetNumDiscStates( S, 0 );						
	ssSetNumInputPorts( S, 1 );						
	ssSetInputPortWidth( S, 0,  1 );				
													
	ssSetInputPortDirectFeedThrough( S, 0, 1 );		
	ssSetNumOutputPorts( S, 1 );					
	ssSetOutputPortWidth( S, 0, 1  );	// changed from 2 to 1 due removal of SPRTSMODESW			
													
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
	ssSetSampleTime( S, 0, INHERITED_SAMPLE_TIME );						
	ssSetOffsetTime( S, 0, 0.0 );						
}
					

static	void	mdlOutputs( SimStruct *S, int_T tid )	
{
	InputRealPtrsType	uPtrs =	ssGetInputPortRealSignalPtrs( S, 0 );
	real_T			*y = ssGetOutputPortRealSignal( S, 0 );	
	

	
	sw_count = sw_count + (*uPtrs[0]);  
   
    modf(sw_count/4.0, &mod_sw);
    
    if (mod_sw < 2.5)    // 0: manual; 1: ACC; 2: CACC;  3~1: ACC;  4~0: back to ACC
         y[0] = mod_sw;
    else
         y[0] = 1;      
}

static	void	mdlTerminate( SimStruct *S )			
{
}

#ifdef	MATLAB_MEX_FILE						
	#include	"simulink.c"				
#else								
	#include	"cg_sfun.h"				
#endif
