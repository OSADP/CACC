#define	S_FUNCTION_NAME		thcal	
#define	S_FUNCTION_LEVEL	2		


#include "simstruc.h"
#include "math.h"
//#include "size.h"

static double	vt_in;
static double	vt_buf;
static double	vt_out;

static double vxg_in;
static double sp_ini;
static double max_a_in;
static double max_v_in;

static int first=0;
static double cruise_t=0;
static double aux1 =0.0;
static double aux2 =0.0;
static double delta_t =0.05;

static double fmin(double, double);
static double fmax(double, double);

static	void	mdlInitializeSizes( SimStruct *S )	
{
	//long	board_base;								

	ssSetNumSFcnParams( S, 0 );						
	ssSetNumContStates( S, 0 );						
	ssSetNumDiscStates( S, 0 );						
	ssSetNumInputPorts( S, 1 );						
	ssSetInputPortWidth( S, 0,  4 );				
													
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
	ssSetSampleTime( S, 0, INHERITED_SAMPLE_TIME);						
	ssSetOffsetTime( S, 0, 0.0 );						
}






#define	MDL_INITIALIZE_CONDITIONS				
#if	defined( MDL_INITIALIZE_CONDITIONS )		
static	void	mdlInitializeConditions( SimStruct *S )	
{				

}
#endif							

#define	MDL_START								
#if	defined( MDL_START )
	static	void	mdlStart( SimStruct *S )
	{			

	}
#endif		

static	void	mdlOutputs( SimStruct *S, int_T tid )	
{
	InputRealPtrsType	uPtrs =	ssGetInputPortRealSignalPtrs( S, 0 );
	real_T			*y = ssGetOutputPortRealSignal( S, 0 );	
	

	vt_in = *uPtrs[0];
	vxg_in = *uPtrs[1];
    max_a_in = *uPtrs[2];
    max_v_in = *uPtrs[3];
    
 if (vxg_in == 1)
 {   
    if (first == 0)
    {
        sp_ini = fmax(11.0, vt_in); // vt_in;     
        vt_out = sp_ini; 
        first = 1;
        cruise_t = 0.0;
    }
    if (vt_out < max_v_in - 0.1)
        vt_out=vt_out+delta_t*max_a_in;
    else if (vt_out > max_v_in + 0.1)   
        vt_out=vt_out - delta_t*max_a_in;
    else
    {
        vt_out = max_v_in; 
        cruise_t=cruise_t+ delta_t;
    }
    
    if (cruise_t > 120.0)  //
        vt_out = max_v_in - delta_t*max_a_in;
    if  ( vt_out < 0.05)
        vt_out = 0.0;
 }
 else
 {
     first = 0;
 }
    
    y[0] = vt_out;
}

#undef	MDL_UPDATE				
#if	defined( MDL_UPDATE )	
	static	void	mdlUpdate( SimStruct *S, int_T tid )
	{
	}
#endif		



#undef	MDL_DERIVATIVES		
#if	defined( MDL_DERIVATIVES )	
	static	void	mdlDerivatives( SimStruct *S )	
	{
	}
#endif					


static	void	mdlTerminate( SimStruct *S )					//
{
    
}

#ifdef	MATLAB_MEX_FILE						
	#include	"simulink.c"				
#else								
	#include	"cg_sfun.h"				
#endif

static double fmin(double a, double b)
{
    if (a<b)
        return a;
    else
        return b;
}

static double fmax(double a, double b)
{
    if (a<b)
        return b;
    else
        return a;
}
