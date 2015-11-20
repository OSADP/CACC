/**************************************************************************
 *
 * Copyright (c) 1996,1997,2015   Regents of the University of California
 *
 **************************************************************************/

#define	S_FUNCTION_NAME		cc_v_ref	
#define	S_FUNCTION_LEVEL	2		


#include "simstruc.h"
#include "math.h"
//#include "size.h"

static double	vt_in;
static double	vt_buf;
static double	v_out, a_out;

static double vxg_in;
static double sp_ini;
static double max_a_in;
static double max_v_in;

static int first=0;
//static double aux1 =0.0;
//static double aux2 =0.0;
static double delta_t =0.05;
static double run_flag=0.0;
static double max_dcc=0.0;

static double fmin(double, double);
static double fmax(double, double);

static	void	mdlInitializeSizes( SimStruct *S )	
{
	//long	board_base;								

	ssSetNumSFcnParams( S, 0 );						
	ssSetNumContStates( S, 0 );						
	ssSetNumDiscStates( S, 0 );						
	ssSetNumInputPorts( S, 1 );						
	ssSetInputPortWidth( S, 0,  6 );				
													
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
	

	vt_in = *uPtrs[0];
	vxg_in = *uPtrs[1];
    max_a_in = *uPtrs[2];
    max_v_in = *uPtrs[3];
    run_flag = *uPtrs[4]; 
    max_dcc = *uPtrs[5]; 
            
 if (vxg_in == 1)
 { 
    if (run_flag > 0.5)
    {
        if (first == 0)
        {
        sp_ini = fmax(3.0, vt_in); // vt_in;     
        v_out = sp_ini; 
        a_out=max_a_in;
        first = 1;       
        }
        if (v_out < max_v_in - 0.1)
        {
            v_out=v_out+delta_t*max_a_in+0.5*max_a_in*delta_t*delta_t;
            a_out=max_a_in;
        }
        else if (v_out > max_v_in + 0.1)  
        {
            v_out=v_out - delta_t*max_dcc;
            a_out=-max_dcc;
        }
        else  
        {
            v_out = max_v_in;  
            a_out=0.0;
        }
    }     
    else if (run_flag < -0.5)
    {
        v_out = v_out - delta_t*max_dcc;
        a_out=- max_dcc;
    }
    else
    {
        v_out = 0.0;
        a_out=0.0;
        first = 0;
    }
    if  ( v_out < 0.05) 
    {
        v_out = 0.0; 
        a_out=0.0;
    }
 }
 else
 {
     first = 0;
     v_out = 0.0;
     a_out=0.0;
 }
     
    y[0] = v_out;
    y[1] = a_out;
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
