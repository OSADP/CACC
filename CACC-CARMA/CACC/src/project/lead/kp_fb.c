/**************************************************************************
 *
 * Copyright (c) 1996,1997,2015   Regents of the University of California
 *
 **************************************************************************/

#define	S_FUNCTION_NAME		kp_fb	// this need to be in each S-function
#define	S_FUNCTION_LEVEL	2		// this need to be in each S-function


#include "simstruc.h"               // A Matlab header file
#include "math.h"
//#include "size.h"

static double	id_in;
static double	sp_in;

static double ref;
static double	ref_prev;			 // ref spd of previous veh

static double kpp = 0.45;           // Control Gain
static double kdp = 0.25;           // Control Gain



static	void	mdlInitializeSizes( SimStruct *S )	// Simulink standard initialization function name
{
	long	board_base;								

	ssSetNumSFcnParams( S, 0 );						
	ssSetNumContStates( S, 0 );						
	ssSetNumDiscStates( S, 0 );						
	ssSetNumInputPorts( S, 1 );		             // Number of input ports corresponding to the block				
	ssSetInputPortWidth( S, 0,  2 );		     // Number of parameters for the port: 2 input parameters		
													
	ssSetInputPortDirectFeedThrough( S, 0, 1 );		
	ssSetNumOutputPorts( S, 1 );			     // Number of input ports corresponding to the block	 		
	ssSetOutputPortWidth( S, 0, 1  );			 // Number of parameters for each port	
													
	ssSetNumSampleTimes( S, 1 );	             // 1 sample time				
	ssSetNumRWork( S, 0 );				
	ssSetNumIWork( S, 0 );				
	ssSetNumPWork( S, 0 );				
	ssSetNumModes( S, 0 );				
	ssSetNumNonsampledZCs( S, 0 );		
	ssSetOptions(S,	0);					

}

static	void	mdlInitializeSampleTimes( SimStruct *S )
{
	ssSetSampleTime( S, 0, INHERITED_SAMPLE_TIME );				 // 0.05, sample time specified for that parameter		
	ssSetOffsetTime( S, 0, 0.0 );				 // off-set time specified for each parameter			
}
					

static	void	mdlOutputs( SimStruct *S, int_T tid )	// Simulink standard output function name
{
	InputRealPtrsType	uPtrs =	ssGetInputPortRealSignalPtrs( S, 0 );   // Simulink standard typeset for RT data reading
	real_T			*y = ssGetOutputPortRealSignal( S, 0 );             // Simulink standard typeset
	
    // as specified above, 2 input parmeters
	id_in = *uPtrs[0];  // measured distance w.r.t. to front veh 
	sp_in = *uPtrs[1];  // subject veh desired D_Gap
    
    if (sp_in < 6.0)      // lower bound of desired D_Gap: original 4; ==> 6.0
    {
        sp_in = 6.0;
    }
    
    ref = id_in - sp_in; // Gap differnce: (desired - measured)
    
    y[0] = kpp * ref + kdp * (ref-ref_prev);        // P+D spd control only; distance regultion
    
    ref_prev = ref;


}


static	void	mdlTerminate( SimStruct *S )		// this must be here	
{
}

#ifdef	MATLAB_MEX_FILE						
	#include	"simulink.c"				
#else								
	#include	"cg_sfun.h"				
#endif
