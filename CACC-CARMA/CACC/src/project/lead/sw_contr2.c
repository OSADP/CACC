/**************************************************************************
 *
 * Copyright (c) 1996,1997,2015   Regents of the University of California
 *
 **************************************************************************/
#define	S_FUNCTION_NAME		sw_contr2	
#define	S_FUNCTION_LEVEL	2		


#include "simstruc.h"
#include "math.h"
//#include "size.h"

//double	sport;
static double  cruise;
//double	sportp;
static double  cruisep;

static double salida;




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
	ssSetSampleTime( S, 0, INHERITED_SAMPLE_TIME );						
	ssSetOffsetTime( S, 0, 0.0 );						
}

static	void	mdlOutputs( SimStruct *S, int_T tid )	
{
	InputRealPtrsType	uPtrs =	ssGetInputPortRealSignalPtrs( S, 0 );
	real_T			*y = ssGetOutputPortRealSignal( S, 0 );	

    cruisep = *uPtrs[0];   // lead (1st) veh man des:  0: manual; 1: ACC; 2: CACC
	cruise = *uPtrs[1];    // Pre (3rd)  veh man des:  0: manual; 1: ACC; 2: CACC
	
   
    salida =0;        // default value
    
    if ( ((cruise > 1.5) && (cruise < 2.5)) && ((cruisep > 1.5) && (cruisep < 2.5)))
    {
        salida =1;
    }    
   
    
    y[0] = salida;
    
}



static	void	mdlTerminate( SimStruct *S )			
{
}

#ifdef	MATLAB_MEX_FILE						
	#include	"simulink.c"				
#else								
	#include	"cg_sfun.h"				
#endif
