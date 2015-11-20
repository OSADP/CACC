#define	S_FUNCTION_NAME		run_dist	
#define	S_FUNCTION_LEVEL	2		


#include "simstruc.h"
#include "math.h"
//#include "size.h"


static double	v_in=0.0;
static double	a_in=0.0;
static double	run_flag=1.0, max_dcc=0.0, max_spd=0.0, cacc_sw=0.0;
static double	run_d=0.0, stop_dist=0.0,  stop_period=0.0;
static int flag_ini=1;
static const double delta_t=0.05;
static double track_length=1700.0; //[m]

static	void	mdlInitializeSizes( SimStruct *S )	
{
	//long	board_base;								

	ssSetNumSFcnParams( S, 0 );						
	ssSetNumContStates( S, 0 );						
	ssSetNumDiscStates( S, 0 );						
	ssSetNumInputPorts( S, 1 );						
	ssSetInputPortWidth( S, 0, 6 );				
													
	ssSetInputPortDirectFeedThrough( S, 0, 1 );		
	ssSetNumOutputPorts( S, 1 );					
	ssSetOutputPortWidth( S, 0, 2  );				
													
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
	
   
	v_in = *uPtrs[0];
	a_in = *uPtrs[1];
    max_dcc=*uPtrs[2];
    max_spd=*uPtrs[3];
    cacc_sw=*uPtrs[4];
    track_length=*uPtrs[5];

stop_period=2.0*(max_spd) / (max_dcc);                                          // Used after Nov. 25 02
stop_dist = track_length - ((max_spd)*stop_period - 0.25*max_dcc*stop_period*stop_period); 
    
if (flag_ini==1 && run_flag==0)
    {       
       run_d=0.0;
       flag_ini=0;
    }    
if (v_in > 0.5) 
{
    run_d=run_d+delta_t*v_in+0.5*a_in*delta_t*delta_t;         
}	
if ((run_d> 0.0) && (run_d < stop_dist))
   run_flag=1.0;
else if (run_d > stop_dist)
   run_flag = -1.0;
else
   run_flag =0.0; 

if (v_in < 0.5 || cacc_sw < 0.5)
{
    run_flag=0.0;    
}
   
if (cacc_sw > 0.5)
   flag_ini=1;   
else
   run_d=0.0; 
    
 y[0] = run_d;
 y[1] = run_flag;
} // func end

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
