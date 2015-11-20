/**************************************************************************
 *
 * Copyright (c) 1996,1997,2015   Regents of the University of California
 *
 **************************************************************************/

#define	S_FUNCTION_NAME		sw_contr	
#define	S_FUNCTION_LEVEL	2		

#include "simstruc.h"
#include "math.h"


//double	sport_2;
static double  cruise_2;

static double salida_2;




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
	ssSetSampleTime( S, 0, INHERITED_SAMPLE_TIME );		// 0.05				
	ssSetOffsetTime( S, 0, 0.0 );						
}
					

static	void	mdlOutputs( SimStruct *S, int_T tid )	
{
	InputRealPtrsType	uPtrs =	ssGetInputPortRealSignalPtrs( S, 0 );
	real_T			*y = ssGetOutputPortRealSignal( S, 0 );	
	

	//sport_2 = *uPtrs[0];   // pre veh drive mode
	cruise_2 = *uPtrs[0];  // ACC button status: 0: manual; 1: ACC; 2: CACC
   
    salida_2 =0;           // default: 0
    
    //if ((sport_2 > 0.5) && (sport_2 < 1.5) && (cruise_2 > 0.5) && (cruise_2 < 1.5)) // ( they both have value 1)
     if ((cruise_2 > 1.5) && (cruise_2 < 2.5)) // 0: manual;  1: PATH ACC;  2: PATH CACC 
    {
        salida_2 =1;       // activated only when both switches have value 1; otherwise, output 0
    }    
   
    
    y[0] = salida_2;
    
}

static	void	mdlTerminate( SimStruct *S )			
{
}

#ifdef	MATLAB_MEX_FILE						
	#include	"simulink.c"				
#else								
	#include	"cg_sfun.h"				
#endif
