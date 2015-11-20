/**************************************************************************
 *
 * Copyright (c) 1996,1997,2015   Regents of the University of California
 *
 **************************************************************************/

#define	S_FUNCTION_NAME		dist_contr	
#define	S_FUNCTION_LEVEL	2		


#include "simstruc.h"
#include "math.h"


static double est_dist = 0.0, t_filter = 0.0, k1_gain = 0.2, temp1=0.0;
        static double est_dist_initial=0.0;
        static double radar_f=0.0, radar_f_old=0.0;
        static double prefilter_radar=0.0, prefilter_radar_old=0.0;
        static double radar_rg_old=0.0, alarm_t_filter = 0.0, radar_rg_tmp=0.0;
        static int radar_init1 = 0, radar_init2 = 0;
        static int radar_fault = 0;
        static double error = 0.0, int_error = 0.0;
        static double spd_bsd_rg=0.0;
        static double tau_radar = 3.0/6.28;
        static double radar_rg=0.0;      // out
        static double radar_range=0.0;   // input
        static double run_flag=0.0, cacc_sw=0.0;
        static double des_f_dist=0.0, man_des=0.0;
		static const double delta_t=0.05;

  
     static double spd, pre_v, v_tmp;
     static double distance=0.0, distance_ini=0.0, distance_old=0.0, dist_crt_t=0.0, range_obs=0.0;
     static double eps=0.0, eps_dot=0.0, dist_trans_t=0.0, dist_buff=0.0, initial_dist=0.0;
     static double following_dist=0.0, radar_range_buff=0.0;     
   
     static int  start_sw=1;
     static double temp_dist=0.0;
     static double ref_v=0.0, ref_a=0.0, v_ref=0.0, a_ref=0.0, v0=0.0;
     static double comm_fault=0.0;
     
     
     static double drive_mode=0.0, t_gap=0.0;     
     static double pre_v=0.0, pre_a=0.0;
     static double v=0.0;  // v_out=0.0;
    // static double f_mode=0.0;
     static double max_accel=0.0, max_decel=0.0, max_spd=0.0;

// from control
static double s_1=0.0, s_2=0.0, k1=0.0, k2=0.0, t_flt=0.0, usyn=0.0,a,b;

static double fmin(double, double);
static double fmax(double, double);

static	void	mdlInitializeSizes( SimStruct *S )	
{
	//long	board_base;								

	ssSetNumSFcnParams( S, 0 );						
	ssSetNumContStates( S, 0 );						
	ssSetNumDiscStates( S, 0 );						
	ssSetNumInputPorts( S, 1 );						
	ssSetInputPortWidth( S, 0,  12 );				
													
	ssSetInputPortDirectFeedThrough( S, 0, 1 );		
	ssSetNumOutputPorts( S, 1 );					
	ssSetOutputPortWidth( S, 0, 6  );				
													
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
	
    t_gap=*uPtrs[0];
    radar_range=*uPtrs[1];
    v=*uPtrs[2]; 
    pre_v=*uPtrs[3]; 
    pre_a=*uPtrs[4]; 
    man_des=*uPtrs[5];
    drive_mode=*uPtrs[6];
    max_accel=*uPtrs[7];
    max_decel=*uPtrs[8];
    max_spd=*uPtrs[9];
    run_flag=*uPtrs[10];
    comm_fault=*uPtrs[11];
    
   
    
    t_filter += delta_t;
    
//     if (drive_mode < 2.0 || t_filter < 2.0)
//     {
//         t_filter=0.0;
//         v0=v;
//     }
   
    
if (t_filter < 2.0)
{
    v_ref=pre_v+max_accel*delta_t;
    a_ref=max_accel;
    //t_filter=0.0;
    v0=v_ref;
}
else
{
    if (run_flag > 0.5)
    {
        if (v_ref < max_spd - 0.1)
        {		
            v_ref=v_ref+delta_t*max_accel+0.5*max_accel*delta_t*delta_t;
            a_ref=max_accel; 		
        }
        else if (v_ref > max_spd + 0.1)
        {
            v_ref=v_ref - delta_t*max_decel;
            a_ref=max_decel; 
        }
        else
        {
            v_ref = max_spd;
            a_ref= 0.0; 
        } 
    } 
    else if (run_flag < -0.5)
    {
        v_ref=v_ref - delta_t*max_decel;
        a_ref=max_decel;
    }
    else
    {
        v_ref = max_spd;
        a_ref= 0.0; 
    }

}
    
if( v_ref < 0.05)
	v_ref = 0.0;    
    

     if( t_filter < 2*delta_t+0.01 )
              {
                est_dist = radar_range;
                est_dist_initial = radar_range;
                radar_f = radar_range;
                radar_f_old = radar_range;
                radar_rg_old = radar_range;
                prefilter_radar_old = radar_range;
              }
           des_f_dist=v*t_gap;
           k1_gain = 0.45*fmin(1.5,prefilter_radar/(des_f_dist));
                      
           if(man_des == 29 || man_des == 30) 
           {
	           tau_radar = 1.0/6.28;
               k1_gain = 0.05;
           }
          
//            prefilter_radar = radar_range;
//            if( (prefilter_radar - prefilter_radar_old) > 0.2) 
//                 prefilter_radar = prefilter_radar_old + 0.2;
//            else if( (prefilter_radar - prefilter_radar_old) < -0.2) 
//                 prefilter_radar = prefilter_radar_old - 0.2;
//            else;
//            prefilter_radar_old = prefilter_radar;
//            
            prefilter_radar = radar_range;
           if( (prefilter_radar - prefilter_radar_old) > 0.5) 
                prefilter_radar = prefilter_radar_old + 0.5;
           else if( (prefilter_radar - prefilter_radar_old) < -0.5) 
                prefilter_radar = prefilter_radar_old - 0.5;
           else;
           prefilter_radar_old = prefilter_radar;

           radar_f = radar_f_old + (prefilter_radar - radar_f_old) 
                        * delta_t / tau_radar;
           radar_f_old = radar_f;

//            if( fabs(radar_range - radar_rg_old) < 1.0 ) 
//                 radar_fault = 0;
//            else 
//                 radar_fault = 1;
            if( fabs(radar_range - radar_rg_old) < 2.5 ) 
                radar_fault = 0;
           else 
                radar_fault = 1;

           if( radar_fault == 1 )         
                k1_gain = 0.0;   

           radar_rg_old = radar_range;

           est_dist += delta_t * (-(spd - pre_v)
                        + k1_gain * (-est_dist + radar_f) );

           if( radar_fault == 1 )
             {
                error += radar_range - est_dist;
                alarm_t_filter += delta_t;
                if( alarm_t_filter > 1.0 )
                  {
                        if( fabs(error) > 50.0 ) 
                                radar_fault = 1;
                        else
                          {
                                radar_fault = 0;                                
                                error = 0;
                                alarm_t_filter = 0.0;
                          }
                  }
             }

         

           int_error += radar_range - est_dist;
      
           radar_rg = est_dist;    		  
    
     if (t_filter <  3.0)
     {                
//           eps=0.0;
//           distance_ini = radar_range;
//           distance_old = radar_range; 
//           range_obs= radar_range;
//           radar_range_buff=fmax(radar_range,5.0);  
         
          distance_ini = radar_rg;
          distance_old = radar_rg;            
//           ref_v=pre_v;
//           ref_a=pre_a;
     }
     else
     {
        
          if (comm_fault < 0.5)   
          {
//             if (radar_range > radar_range_buff + delta_t*(max_accel+0.05))
//               radar_range=radar_range_buff+ delta_t*(max_accel+0.05); 
//             if (radar_range < radar_range_buff - delta_t*(max_decel-0.05))
//               radar_range=radar_range_buff- delta_t*(max_accel-0.05);            
//             radar_range_buff=radar_range;  
//             eps_dot = pre_v-spd;                                                        
//             eps += eps_dot * delta_t;                                          
//             distance = distance_ini + eps; 
//             a=distance*distance;
//             b=radar_range*radar_range;
//            // range_obs= (a/(a+b))*distance + (b/(a+b))*radar_range;
//             //range_obs=radar_range;
            //range_obs= 0.1*distance + 0.9*radar_range;
            range_obs= radar_rg;     
            ref_v=pre_v;
            ref_a=pre_a;
          }
          else
          {
              range_obs=radar_range;   // we have to use raw radar range                                        
              ref_v = v_ref;
              ref_a=a_ref;
                
              if  ( ref_v < 0.05)
                    ref_v = 0.0;   
          }  
     }
            if( (distance - distance_old) > 0.5)                             
                distance = distance_old + 0.5;
            else if( (distance - distance_old) < -0.5) 
                distance = distance_old - 0.5;
            distance_old = distance; 
                                   
            if (t_filter >7.0)                                 
                {
                    if (dist_crt_t < 6.0)
                       {
                           dist_crt_t +=delta_t;
                           range_obs -= 0.15*delta_t/6.0;      
                       }
                    else
                           range_obs = range_obs;         
                }
            distance = range_obs;                       
          
       if (start_sw == 1)    
	      {	     
            if (t_filter < 0.1)
                {      
                   initial_dist = range_obs;                                          
                   temp_dist = initial_dist;
                }

            else  
                temp_dist = des_f_dist + (initial_dist -  des_f_dist) * exp(-(t_filter-0.0)/20.0);                                 
                   
          if  (fabs(temp_dist - des_f_dist) < 1.0)   
                start_sw =0;
        }  
      else
           temp_dist = des_f_dist; 
    
    // control
    if (drive_mode < 1.5) 
     {
        s_1=0.0;
        s_2=0.0;
     }
     else
     {
        s_1 = -(distance - temp_dist);  
        s_2 = v-ref_v;    
         
        if (t_flt < 8.0)
        {
         k1 = 0.25 + 0.457*t_flt/8.0;
         k2 = 0.5  + 0.207*t_flt/8.0;
        }
        else
        {
         k1=0.707;  
         k2=0.707;
        }       
                                                            
        usyn= -(k1+k2)*s_2 - k1*k2*s_1 + ref_a; 
     }
    
     v_tmp=v0+usyn*delta_t;
//     if (v_tmp > pre_v+6.0)
//         v_tmp = pre_v+6.0;
//     if (v_tmp > max_spd+6.0)
//         v_tmp = max_spd+6.0;
//     
//     if (pre_a < -0.01)
//         v_tmp = pre_v;
    
    y[0]=v_tmp;
    y[1]=s_1;
    y[2]=s_2;
    y[3]=des_f_dist;
    y[4]=distance;
    y[5]=usyn;
    
        
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
