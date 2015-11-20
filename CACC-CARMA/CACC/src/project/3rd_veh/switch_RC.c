/**************************************************************************
 *
 * Copyright (c) 1996,1997,2015   Regents of the University of California
 *
 **************************************************************************/

#define	S_FUNCTION_NAME		switch_RC	
#define	S_FUNCTION_LEVEL	2		


#include "simstruc.h"
#include "math.h"


static double	dist;
static double sp_error;
static double platoon_pos;
static double tg_error;
static double changeACC;

static double r_vs_c=1;   // Radar vs. Communication; 1: radar;  0: communication

static double dist_buff;
static double fcheck=1;   // healthy status: 1 - radar error; 0: healthy
static double total_sp_error;
static int count=0;




static	void	mdlInitializeSizes( SimStruct *S )	
{
	long	board_base;								

	ssSetNumSFcnParams( S, 0 );						
	ssSetNumContStates( S, 0 );						
	ssSetNumDiscStates( S, 0 );						
	ssSetNumInputPorts( S, 1 );						
	ssSetInputPortWidth( S, 0,  5 );	// changed from 5 to 4 on 06_09_14			
													
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
	ssSetSampleTime( S, 0, INHERITED_SAMPLE_TIME );   // this will running at 10Hz	xyl on 08_23_15					
	ssSetOffsetTime( S, 0, 0.0 );						
}

static	void	mdlOutputs( SimStruct *S, int_T tid )	
{
	InputRealPtrsType	uPtrs =	ssGetInputPortRealSignalPtrs( S, 0 );
	real_T			*y = ssGetOutputPortRealSignal( S, 0 );	
	

	dist = *uPtrs[0];          // dist to preceding veh from Lidar
	sp_error = *uPtrs[1];      // subject vehicle spd error: averaged over wheel speeds error and lidar measured relative speed
	platoon_pos = *uPtrs[2];   // platoon position: lead=1
	tg_error = *uPtrs[3];      // T-Gap error w.r.t set Tiem Gap
	changeACC = *uPtrs[4];     // DVI ACC button: 1 - using ACC; 0-Not using ACC
    

    if (count > 499)
    {
        if (total_sp_error < 0.6)
        {
            fcheck = 0;        // check rada/lidar error
        }
        total_sp_error = 0;    // accumulated speed err
        count = 0;    
        
    }
    
    if ((fcheck > 0.5) && (platoon_pos > 1.5) && (platoon_pos < 5.5))  // this only apply to 2nd or upstream vehs; not the leader
    {
        total_sp_error = total_sp_error + abs(sp_error)/500.0;
        if (total_sp_error > 0.6)
        {
            count = 499;
        }
        count = count + 1;
    }    
   
    
    if (dist_buff < 0.5) // lidar dist measure of previous T-step has error
    {
        r_vs_c = 1;      // use DSRC info if platoon_pos > 1.5
        fcheck = 1;      // signal lidar error
        count = 0;
    }
    
    if (dist_buff > 0.5)
    {   
        if ( (abs ( dist_buff - dist ) < 3.5) && (fcheck < 0.5) ) // health condition
        {
            r_vs_c = 0;  // Using communication
            count = 0;
        }
        if ( abs ( dist_buff - dist ) > 3.5 ) // if dist measure jump too large; also consodered lidar error
        {
            r_vs_c = 1;  // Using lidar/radar
            fcheck = 1;
        }
        
    }
      
    if (platoon_pos < 1.5)
    {
        r_vs_c = 1;  // Using lidar/radar for a single veh
    }       
    
    /* This is for the case when the vehicle is following a vehicle in the other lane instead of its preceding one */
      
    if (tg_error > 0.1)  // if TG is large, then considered error
    {
            r_vs_c = 1;
            fcheck = 1;
    }       
    
    /* This is for the case when the leading vehicle is neither CACC mode nor ACC mode 
      
    if (changeACC < 0.5)
    {
            r_vs_c = 1;
            fcheck = 1;
    }       
    */
    
    
    y[0] = r_vs_c; // 0: if lidar is heathy; 1: Comm is OK
    
    dist_buff = dist;


}

static	void	mdlTerminate( SimStruct *S )			
{
}

#ifdef	MATLAB_MEX_FILE						
	#include	"simulink.c"				
#else								
	#include	"cg_sfun.h"				
#endif
