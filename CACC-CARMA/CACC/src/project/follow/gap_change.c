/**************************************************************************
 *
 * Copyright (c) 1996,1997,2015   Regents of the University of California
 *
 **************************************************************************/

#define	S_FUNCTION_NAME		gap_change	
#define	S_FUNCTION_LEVEL	2		


#include "simstruc.h"
#include "math.h"


static double gap = 600;       // also used as a buffer
static double f_control = 0;
static double acc_sw_p = 0;    // buff for previous time step of the corresponding value
static double vone_p;          // buff for previous time step of the corresponding value
static double cdist_p = 1;     // buff for previous time step of the corresponding value
static double sacel_p = 0;     // buff for previous time step of the corresponding value
static double gap_aux = 1100;  // Estimated current T-Gap based on dist/sp measures
static double ref_gap = 1100;
static double contsafe=0;
static int fcontsafe =0;

static double acc_sw;
static double vdram;
static double vone; //
static double cdist;
static double cspeed;
static double sacel; // switch manual accelerator


static	void	mdlInitializeSizes( SimStruct *S )	
{
	long	board_base;								

	ssSetNumSFcnParams( S, 0 );						
	ssSetNumContStates( S, 0 );						
	ssSetNumDiscStates( S, 0 );						
	ssSetNumInputPorts( S, 1 );			// only 1 input port			
	ssSetInputPortWidth( S, 0,  6 );	// 7 data width for the input port			
													
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
	InputRealPtrsType	uPtrs =	ssGetInputPortRealSignalPtrs( S, 0 );  // simulink struct
	real_T			*y = ssGetOutputPortRealSignal( S, 0 );            // simulink struct
	
    // as specified above: there are 7 input parameters associated with this port
	//sportm = *uPtrs[0];     // sport mode sw
	acc_sw = *uPtrs[0];     // ACC cruise sw
	vdram = *uPtrs[1];      // Driver set T_Gap; 3 T_Gap values for both ACC and CACC: 1, 2, 3
	vone = *uPtrs[2];       // Subj Veh block activation flag;
	cdist = *uPtrs[3];      // measured Current distance
	cspeed = *uPtrs[4];     // Current speed 
	sacel = *uPtrs[5];      // Flag: driver push accel pedal or not
    
    if ((vone-vone_p > 0.5) && (abs(cdist-cdist_p) < 3.0))
        // SW is ON             // Dist change w.r.t. previous time step
    {
        fcontsafe =1;  // not safe
        contsafe =0;
    }    
    
    if (fcontsafe > 0.5)
    {
        vone =0;
        contsafe = contsafe + 1;
        
        if (contsafe > 250)
        {
            contsafe =0;
            fcontsafe =0;
        }
    }
    
// Mpaping: T-Gap => D-Gapchanged;  for initial tests at Leidos    08_28_15    
if ((vdram < 1.5) && (vone > 0.5))       { ref_gap = 1500; } //ACC  in [ms] if ACC set to 1
else if ((vdram < 2.5) && (vone > 0.5))  { ref_gap = 1800; }  // if ACC set to 2; original: 1100; changed by XYLu  on 06_04_14
else if ((vdram > 2.5) && (vone > 0.5))  { ref_gap = 2200; }  // if ACC set to 3; original: 1100; changed by XYLu  on 06_04_14
else                                     { ref_gap = 2400; }

// if ((vdram < 1.5) && (vone > 0.5))       { ref_gap = 900; } //ACC  in [ms]
// else if ((vdram < 2.5) && (vone > 0.5))  { ref_gap = 1600; }  // original: 1100; changed by XYLu  on 06_04_14
// else if ((vdram > 2.5) && (vone > 0.5))  { ref_gap = 2200; }  // original: 1100; changed by XYLu  on 06_04_14
// else if ((vdram < 1.5) && (vone < 0.5))  { ref_gap = 600; } //CACC
// else if ((vdram < 2.5) && (vone < 0.5))  { ref_gap = 900; }
// else                                     { ref_gap = 1100; }

// CACC control will be ON in any of the following cases
//if ( ( (sportm-sportm_p > 0.5) && (acc_sw > 0.5) ) ||    // removed on 09_14_14 by X_Y_Lu
if ( ( (acc_sw> 1.5)  ) ||    
       (acc_sw-acc_sw_p > 0.5) ||
     ( (abs(cdist-cdist_p) > 3.5) && (vone > 0.5) && (cdist > 2.0) ) ||     
     ( (sacel-sacel_p < - 0.5) )  ||  ( (vone-vone_p > 0.5) )	                 ) 
	{ f_control = 1; }  // CACC Conditionally ON
else    // Otherwise, CACC will be OFF           
	{ f_control = 0; }


if (f_control > 0.5) // If CACC is ON, Calculate curret T-Gap and Scale by 1000
    { gap_aux = 1000.0*cdist/cspeed; }   // conditionally updated T-Gap; otherwise, it is 1100
    

if (abs(gap_aux-gap) < 75)  // if differnce between curent & previous step T-Gaps are small enough 
    { gap_aux = gap; }
if (gap_aux < gap)          // if current estimated T-Gap is smaller, update the buffer       
    { gap = gap_aux; }
    
gap_aux = 1100;             // reset to default largest T-Gap value; this is necessary
    


// Gap rt limit; very small change at for each time step since it is scaled by 1000
// if (gap < ref_gap)         
//     { gap = gap + 1; }
// else if (gap > ref_gap)    
//     { gap = gap - 1; }
// else                        
//     { gap = gap; }

// changed since delta_t is 0.05 vs 0.01 for NISSAN CACC
if (gap < ref_gap)         
    { gap = gap + 5; }
else if (gap > ref_gap)    
    { gap = gap - 5; }
else                        
    { gap = gap; }

// Gap bound limit
if (gap < 350)      
    { gap = 350; }
if (gap > 1100)       
    { gap = 1100; }

// Buffer status
//sportm_p = sportm;
acc_sw_p = acc_sw;
vone_p = vone;
cdist_p = cdist;
sacel_p = sacel;

    
      
    y[0] = gap/1000.0;  // Scales back to T-Gap for output
   

}


static	void	mdlTerminate( SimStruct *S )			
{
}


#ifdef	MATLAB_MEX_FILE						
	#include	"simulink.c"				
#else								
	#include	"cg_sfun.h"				
#endif
 
