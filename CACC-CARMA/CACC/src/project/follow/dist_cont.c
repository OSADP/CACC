/**************************************************************************
 *
 * Copyright (c) 1996,1997,2015   Regents of the University of California
 *
 **************************************************************************/


#define	S_FUNCTION_NAME		dist_cont	// this need to be in each S-function
#define	S_FUNCTION_LEVEL	2	        // this need to be in each S-function	


#include "simstruc.h"
#include "math.h"

static double flag;
static double sp;
static double sp_diff;
static double decel;
static double CC_speed;
static double d_real;
static double CACC_active;

static double sp_diff_ini;
static double sp_ini;
static double counter = 0;

static double sp_ref = 0;
static int fcontrol=0;
static const double delta_t=0.05;
static const double max_accel=1.2;




static	void	mdlInitializeSizes( SimStruct *S )	
{
	long	board_base;								

	ssSetNumSFcnParams( S, 0 );						
	ssSetNumContStates( S, 0 );						
	ssSetNumDiscStates( S, 0 );						
	ssSetNumInputPorts( S, 1 );						
	ssSetInputPortWidth( S, 0,  7 );			
													
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
	

	flag = (*uPtrs[0]);              // control flag: diff of (measured dist - desired dist)
	sp = *uPtrs[1];                  // measured subj veh speed
	sp_diff = *uPtrs[2];             // estimated relative spd:(subj_veh_v - pre_veh_v)
	decel = *uPtrs[3];               // Max deceleration
	CC_speed = *uPtrs[4];            // driver set CC speed
	d_real = *uPtrs[5];              // dist error: (measured distance -desired dist)
	CACC_active = *uPtrs[6];         // CACC activation botton
       
  if (flag > 4.0)
      fcontrol = 1;
  else if (flag < -0.5)
      fcontrol = -1;
  else
      fcontrol = 0;
      
//     if (fcontrol == 0)
//     {
//         if (sp_diff > 1.55)
//         {
//             sp_ref = sp;
//         }
//         else
//         {
//         sp_ref = CC_speed-0.01;
//         }
//         counter = 0;
//         sp_ini = sp;
//         sp_diff_ini = sp_diff;
//     }  
//     if (flag > 3.0)
//     {
//         fcontrol = 1;
//     }
//     
    
    if (fcontrol == 1) // original: ((flag < 0.5) || (fcontrol > 0.5))
    {
        
        //if ((d_real > 1.0) )
       // {
            if (sp_diff > 5.0)
                sp_ref = (sp-sp_diff) + 0.01*max_accel * delta_t;
            else if (sp_diff > 4.0)
                sp_ref = (sp-sp_diff) + 0.03*max_accel * delta_t;
            else if (sp_diff > 2.0)
                sp_ref = (sp-sp_diff) + 0.06*max_accel * delta_t;
            else if (sp_diff > 1.5)
                sp_ref = (sp-sp_diff) + 0.09*max_accel * delta_t;
            else if (sp_diff > -1.5)
                sp_ref = (sp-sp_diff) + 0.1*max_accel * delta_t;
            else
                sp_ref = (sp-sp_diff) + 0.15*max_accel * delta_t;
       // }              
        if ((d_real < 1.0) )
        {
           // if (sp_diff > 1.55)            
                sp_ref = (sp-sp_diff);     
               // sp_ref = sp;
            //else       
            //    sp_ref = CC_speed-0.01;        
        }
    }
    else if (fcontrol == -1)
    {
            if (sp_diff > 5.0)
                sp_ref = (sp-sp_diff) - 0.2*decel*delta_t;
            if (sp_diff > 3.0)
                sp_ref = (sp-sp_diff) - 0.15*decel*delta_t;
            else if (sp_diff > 2.0)
                sp_ref = (sp-sp_diff) - 0.1*decel*delta_t;
            else if (sp_diff > 1.5)
                sp_ref = (sp-sp_diff) - 0.09*decel*delta_t;
            else if (sp_diff > 1.0)
                sp_ref = (sp-sp_diff) - 0.08*decel*delta_t;
            else if (sp_diff > 0.5)
                sp_ref = (sp-sp_diff) - 0.05*decel*delta_t;
            else
                sp_ref = (sp-sp_diff);
    }
    else
    {
//          if (sp_diff > 1.55)            
//             sp_ref = sp;       
//          else       
//             sp_ref = CC_speed-0.01;   
         sp_ref = sp;
    }
            
//         if (flag < 0.5)
//         {
//             // sp_ref = sp_ini - sp_diff_ini * decel * ( counter  / (100.0 * sp_diff_ini) );   // Original by Vicente
//             sp_ref = sp_ini - sp_diff_ini * decel * ( counter  / (20.0 * sp_diff_ini) );   // it is gradually decreasing the gap and the relative speed - Gap Closing Controller; XYL       
//             if (sp_ref < ( sp - sp_diff  + 1.0) )   // limit reduction rate
//             {
//                 sp_ref = sp - sp_diff + 1.0;
//             }
//             counter = counter + 1;  
//         }  
//         
//         if (sp_diff < -0.6)  // tmp removal; by XYLU 09_01_15
//         {
//         if (sp_diff > 1.55)
//         {
//             sp_ref = sp;
//         }
//         else
//         {
//         sp_ref = CC_speed-0.01;
//         }
//         counter = 0;
//         sp_ini = sp;
//         sp_diff_ini = sp_diff;
//         fcontrol = 0;
//       
    
//      
//     /*This is for the case in which the vehicle switch to ACC mode to avoid to send wrong references to the preceding one*/
//     if (CACC_active < 1.5)   //  0.5=> 1.5: CACC is activated only when man_des=2; changed by X_Y_Lu on 09_14_14
//     {
//         sp_ref = sp;
//     }
    
      
    y[0] = sp_ref;


}

static	void	mdlTerminate( SimStruct *S )			
{
}

#ifdef	MATLAB_MEX_FILE						
	#include	"simulink.c"				
#else								
	#include	"cg_sfun.h"				
#endif
