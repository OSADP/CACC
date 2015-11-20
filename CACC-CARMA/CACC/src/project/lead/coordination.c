/**************************************************************************
 *
 * Copyright (c) 1996,1997,2015   Regents of the University of California
 *
 **************************************************************************/


/* ----------------------------------------------------------------------

	CACC Coordination	

---------------------------------------------------------------------- */


#define	S_FUNCTION_NAME		coordination
#define	S_FUNCTION_LEVEL	2

#include "simstruc.h"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"

//#include "veh_long.h"
#define NR_END 1
#define FREE_ARG char*

#define Max_comm_loss 10
   
#define veh_1   1
#define veh_2   2
#define veh_3   3
#define veh_4   4
#define veh_5   5


#define pltn_size    2


#define N_jbus_dat   36
#define N_dsrc_dat   71
#define Max_comm_loss   10
//#define Max_comm_count  255


static double subject_veh_id=100; 
static int control_pos_int=1;

static const int veh_id_list[5]={96, 97, 98, 99, 100};

static const double delta_t=0.05;
static double t_comm_pre=0.0, t_comm_lead=0.0;

static int i,j;
static int drive_mode_sw=0;
static int comm_pre_loss_count;
static int comm_pre_ini_sw;
static  double comm_pre_buff;
static int comm_pre_fault=0;
static int comm_lead_loss_count;
static int comm_lead_ini_sw;
static  double comm_lead_buff;
static int comm_lead_fault=0;
static int comm_fault=0;
static int target_flag=0;
static int acc_sw=0, cacc_sw=0;
static double pre_eps_dot=0.0;
static double spd, pre_v, pre_eps, pre_dist_ini;
static double pre_rg_obs,pre_dist, radar_rg; 
static double lead_eps_dot, spd, lead_v, lead_eps;
static double lead_dist_ini, lead_rg_obs,lead_dist, radar_rg, lead_dist; 
static const int N_st_subj=26;
static const int N_st_pre=16;
static const int N_st_lead=16;

// input info
static double *subj_veh;
static double *pre_veh;
static double *lead_veh;
static int *comm_count;
static void nrerror(char error_text[]);
static double *dvector( int nl, int nh);
static void free_dvector(double *v, int nl, int nh);
static int *ivector(long nl, long nh);
static void free_ivector(int *v, long nl, long nh);
static int dmin(int a, int b);
static int dmax(int a, int b);
static double fmin(double a, double b);
static double fmax(double a, double b);


static	void	mdlInitializeSizes( SimStruct *S )												// this functin should be called just once atthe start
{

	ssSetNumSFcnParams( S, 0 );																	//
	ssSetNumContStates( S, 0 );																	//
	ssSetNumDiscStates( S, 0 );																	//
	ssSetNumInputPorts( S, 8 );																	//
    ssSetInputPortWidth( S, 0, 15);	                                                            // Output of Pos info to Coordination
	ssSetInputPortWidth( S, 1, 71);																// JBus data
	ssSetInputPortWidth( S, 2, 71);															    // BSM1
	ssSetInputPortWidth( S, 3, 71);															    // BSM2
    ssSetInputPortWidth( S, 4, 71);															    // BSM3
	ssSetInputPortWidth( S, 5, 71);	                                                            // BSM4
    ssSetInputPortWidth( S, 6, 36);                                                             // BSM5
    ssSetInputPortWidth( S, 7, 1);                                                              // subj veh id
	
	ssSetInputPortDirectFeedThrough( S, 0, 1 );													//
	ssSetInputPortDirectFeedThrough( S, 1, 1 );													//
	ssSetInputPortDirectFeedThrough( S, 2, 1 );													//
    ssSetInputPortDirectFeedThrough( S, 3, 1 );													//
	ssSetInputPortDirectFeedThrough( S, 4, 1 );													//
	ssSetInputPortDirectFeedThrough( S, 5, 1 );													//
    ssSetInputPortDirectFeedThrough( S, 6, 1 );													//
	    
	ssSetNumOutputPorts( S, 4);																//
	ssSetOutputPortWidth( S, 0, 6 );															// coord info on activation
	ssSetOutputPortWidth( S, 1, 26 );															// coord subject veh info
    ssSetOutputPortWidth( S, 2, 17 );															// coord pre veh info
    ssSetOutputPortWidth( S, 3, 17 );															// coord lead veh info
   
	ssSetNumSampleTimes( S, 1 );																//
	ssSetNumRWork( S, 0 );																		//
	ssSetNumIWork( S, 0 );																		//
	ssSetNumPWork( S, 0 );																		//
	ssSetNumModes( S, 0 );																		//
	ssSetNumNonsampledZCs( S, 0 );																//
	ssSetOptions( S, 0 );																		//
}
   
static	void	mdlInitializeSampleTimes( SimStruct *S )										//
{

	ssSetSampleTime( S, 0, INHERITED_SAMPLE_TIME );																//
	ssSetOffsetTime( S, 0, 0.0 );																//
}

#define	MDL_INITIALIZE_CONDITIONS																//
#if	defined( MDL_INITIALIZE_CONDITIONS )														//
static	void	mdlInitializeConditions( SimStruct *S )	
{																								//

}
#endif																							//


 
#define MDL_START  /* Change to #undef to remove function */
#if defined(MDL_START)
  /* Function: mdlStart =======================================================
   * Abstract:
   *    This function is called once at start of model execution. If you
   *    have states that should be initialized once, this is the place
   *    to do it.
   */
static void mdlStart(SimStruct *S)
    {
        //int i; //j; 
  
        subj_veh=dvector(0,N_st_subj-1);
        pre_veh=dvector(0,N_st_pre-1);
        lead_veh=dvector(0,N_st_lead-1);      
        comm_count=ivector(0,pltn_size-1);
       
     
                	
    }
#endif /*  MDL_START */

																					//
static	void	mdlOutputs( SimStruct *S, int_T tid )											// read CAN and comm info and pass out
{
    int i, j; 

    real_T			*y;//					// output: activation
    real_T			*y1; //					// output: subjc veh info
    real_T			*y2; //					// output: pre veh info
    real_T			*y3; //					// output: lead veh info
    
    InputRealPtrsType  data_in[8];
   
    data_in[0]= (ssGetInputPortRealSignalPtrs( S, 0 ));   // Veh pos info, dim=15: 
    
   	data_in[1]= (ssGetInputPortRealSignalPtrs( S, 5 ));   // get RT info DSRC:  dim=71; already sorted in sequence
	data_in[2]= (ssGetInputPortRealSignalPtrs( S, 1 ));   // get RT info DSRC: dim=71; already sorted in sequence
	data_in[3]= (ssGetInputPortRealSignalPtrs( S, 3 ));   // get RT info DSRC:  dim=71; already sorted in sequence
    data_in[4]= (ssGetInputPortRealSignalPtrs( S, 4 ));   // get RT info DSRC: dim=71; already sorted in sequence
	data_in[5]= (ssGetInputPortRealSignalPtrs( S, 2 ));   // get RT info DSRC:  dim=71; already sorted in sequence
    
//     for (i=1;i<=5;i++)
//     {
//         for (j=0;j<5;j++)
//         {
//             if ((int)subj_veh_id == veh_id_list[j])
//                 	data_in[i]= (ssGetInputPortRealSignalPtrs( S, j ));  
//         }
//     }
    
    data_in[6]= (ssGetInputPortRealSignalPtrs( S, 6 ));   // get JBus info: dim=36   
    data_in[7]= (ssGetInputPortRealSignalPtrs( S, 7 ));   // get subj veh id: dim=1 

    y	= ssGetOutputPortRealSignal( S, 0 );    // output: activation: dim=6
    y1  = ssGetOutputPortRealSignal( S, 1 );    // output: subj veh info; dim=26
    y2	= ssGetOutputPortRealSignal( S, 2 );    // output: pre veh info; dim=16
    y3	= ssGetOutputPortRealSignal( S, 3 );    // output: lead veh info; dim=16   
	
    
   // to determine subject_veh_id and control_pos
    //subject_veh_id = *(data_in[7][0]);
    
    
//     for (i=0;i<=pltn_size;i++)
//     {
//         if ((int)subject_veh_id == (int)data_in[0][3*i])
//             control_pos_int=(int)data_in[0][3*i+1];            
//     }
    
   
    	
   /////////////////////////////////////////////////////////////////////////////////////////////////////////
   if (pltn_size == 1)
   {
            subj_veh[0]=*(data_in[control_pos_int][42]); //global synch t;
            subj_veh[1]=1; //grp_id;
            subj_veh[2]=pltn_size; //grp_size;
            subj_veh[3]=4; //grp_mode;   // 0: stopped;   1: manual; 2: CC; 3: ACC; 4: CACC
            subj_veh[4]=1; //grp_man_des; // CACC; 2; Pltoon
            subj_veh[5]=1; //grp_man_id;   // CACC; 2; Pltoon
            subj_veh[6]=*(data_in[7][0]); // veh id
            subj_veh[7]=0; // dsrc_status
            subj_veh[8]=0; // fault_mode                                   
            subj_veh[9]=0; //cut_in_flag;
            subj_veh[10]=1; //control_pos;                        
            subj_veh[11]=0; //man_des;
            subj_veh[12]=0; //man_id;              
            subj_veh[13]=0; //drive_mode;
            subj_veh[14]=0; //rel_v_to_pre;
            subj_veh[15]=0; //rel_d_to_pre;
            subj_veh[16]=0; //rel_v_to_lead;
            subj_veh[17]=0; //rel_d_to_lead;            
            subj_veh[18]=0; //des_d_gap_to_pre;
            subj_veh[19]=0; //des_d_gap_to_lead;              
            subj_veh[20]=0; // Emergent_brk_flag
            subj_veh[21]=0; //user_def_1
            subj_veh[22]=0; //user_def_2
            subj_veh[23]=0; //user_def_3
            subj_veh[24]=0; //user_def_4
            subj_veh[25]=0; //user_def_5  
            
        for (i=0;i<N_st_pre;i++)
                pre_veh[i]=0.0;
        for (i=0;i<N_st_lead;i++)
                lead_veh[i]=0.0;                  
   }
   else if (pltn_size == 2)
   {                    
        if  (control_pos_int==1)
        {
            subj_veh[0]=*(data_in[control_pos_int][42]); //global synch t;
            subj_veh[1]=1; //grp_id;
            subj_veh[2]=pltn_size; //grp_size;
            subj_veh[3]=4; //grp_mode;   // 0: stopped;   1: manual; 2: CC; 3: ACC; 4: CACC
            subj_veh[4]=1; //grp_man_des; // CACC; 2; Pltoon
            subj_veh[5]=1; //grp_man_id;   // CACC; 2; Pltoon
            subj_veh[6]=*(data_in[7][0]); // veh id
            subj_veh[7]=0; // dsrc_status
            subj_veh[8]=0; // fault_mode                                   
            subj_veh[9]=0; //cut_in_flag;
            subj_veh[10]=1; //control_pos;                        
            subj_veh[11]=0; //man_des;
            subj_veh[12]=0; //man_id;              
            subj_veh[13]=2; //drive_mode;
            subj_veh[14]=0; //rel_v_to_pre;
            subj_veh[15]=0; //rel_d_to_pre;
            subj_veh[16]=0; //rel_v_to_lead;
            subj_veh[17]=0; //rel_d_to_lead;            
            subj_veh[18]=0; //des_d_gap_to_pre;
            subj_veh[19]=0; //des_d_gap_to_lead;              
            subj_veh[20]=0; // Emergent_brk_flag
            subj_veh[21]=0; //user_def_1
            subj_veh[22]=0; //user_def_2
            subj_veh[23]=0; //user_def_3
            subj_veh[24]=0; //user_def_4
            subj_veh[25]=0; //user_def_5  
            
            for (i=0;i<N_st_pre;i++)
                pre_veh[i]=0.0;
            for (i=0;i<N_st_lead;i++)
                lead_veh[i]=0.0;                      
        }
        else
        {
            subj_veh[0]=*(data_in[control_pos_int][42]); ////global synch t;
            subj_veh[1]=1.0; //grp_id;
            subj_veh[2]=2.0; //grp_size;
            subj_veh[3]=4.0; //grp_mode;
            subj_veh[4]=41.0; //grp_man_des;
            subj_veh[5]=41.0; //grp_man_id;
            subj_veh[6]=*(data_in[7][0]); //veh_id;
            // use BSM1
            if (comm_pre_ini_sw == 0)
            {
               comm_pre_buff=*(data_in[1][40]);  // comm_count
               comm_pre_ini_sw=1;
            }            
            if (fabs(comm_pre_buff-(*(data_in[1][40]))<0.1))
                comm_pre_loss_count++;
            else            
                comm_pre_loss_count=0;
            comm_pre_buff=*(data_in[1][40]);
                   
            if (comm_pre_loss_count > Max_comm_loss)
                comm_pre_fault=1;    
            if (comm_pre_fault==1)
            {
                subj_veh[7]=1.0; // dsrc_status          
                subj_veh[8]=2.0; //veh_fault_id; 0: no fault; 1: radar fault; 2: DSRC fault; 3: both radra and DSCR fault 
            }
            else
            {
                subj_veh[7]=0; // dsrc_status          
                subj_veh[8]=0; //veh_fault_id; 
            }   
            subj_veh[9]=0.0; //cut-in flag;
            subj_veh[10]=control_pos_int; //control_pos;
            subj_veh[11]=41.0; //man_des;              
            subj_veh[12]=41.0; //man_id;
            if ( *(data_in[6][1]) > 0.5 && (*(data_in[6][2]) > 0.5) )  // ACC & CACC sw are ON
                subj_veh[13]=4; //drive_mode;
            else 
                subj_veh[13]=1; //manual        
          
            subj_veh[14]=*(data_in[6][34]); //rel_v_to_pre; radar
            subj_veh[15]=*(data_in[6][33]); //rel_d_to_pre; radar
            
            spd=*(data_in[6][6]);
            radar_rg=*(data_in[6][33]);             
            if (comm_pre_fault==0)
            {   
                t_comm_pre+=delta_t;
                if (t_comm_pre < 1.0)
                    pre_dist_ini=radar_rg;
                pre_v=*(data_in[1][49]);
                pre_eps += (spd - pre_v) * delta_t;                                          
                pre_rg_obs = pre_dist_ini - pre_eps;     
                pre_dist= 0.85* pre_rg_obs + 0.15*radar_rg; 
                lead_dist=pre_dist;
                subj_veh[16]=pre_v-spd; //rel_v_to_lead;
                subj_veh[17]=lead_dist; //rel_d_to_lead;   
            }
            else
            {
                subj_veh[16]=*(data_in[6][34]); //rel_v_to_lead;
                subj_veh[17]=*(data_in[6][33]); //rel_d_to_lead;   
                t_comm_pre=0.0;
            }
            subj_veh[18]=(*(data_in[6][6])) *(*(data_in[6][35])); //des_d_gap_to_pre= T-Gap * v;          
            subj_veh[19]=(*(data_in[6][6])) *(*(data_in[6][35])); //des_d_gap_to_lead;  
      
            subj_veh[20]=0; //Emergent brk flag
            subj_veh[21]=0; //user_def_1
            subj_veh[22]=0; //user_def_2
            subj_veh[23]=0; //user_def_3
            subj_veh[24]=0; //user_def_4
            subj_veh[25]=0; //user_def_5  
            
            pre_veh[0]=((unsigned int)*(data_in[1][0]))& 0x2; //pre_acc_sw;
            pre_veh[1]=((unsigned int)*(data_in[1][0]))& 0x20; //pre_cacc_sw;
            pre_veh[2]=*(data_in[1][26]); //pre_fault_mode;
            pre_veh[3]=*(data_in[1][25]); //pre_control_pos;
            pre_veh[4]=*(data_in[1][0]); //pre_drive_mode;
            pre_veh[5]=*(data_in[1][27]); //pre_man_des;
            pre_veh[6]=*(data_in[1][68]); //pre_a;  // PinPoint accel[55]; but JBus accel []; a_des in [68]
            pre_veh[7]=*(data_in[1][49]); //pre_v;
            pre_veh[8]=*(data_in[1][35]); //pre_d;
            pre_veh[9]=*(data_in[1][37]); //pre_v_ref;
            pre_veh[10]=*(data_in[1][35]); //pre_d_ref;
            pre_veh[11]=*(data_in[1][29]); //pre_radar_range
            pre_veh[12]=*(data_in[1][30]); //pre_radar_rate
            pre_veh[13]=*(data_in[1][57]); //pre_brk_pedal_deflect
            pre_veh[14]=*(data_in[1][37]); //pre_v_des
            pre_veh[15]=*(data_in[1][38]); //pre_torq_des
            pre_veh[16]=*(data_in[1][66]); //pre_run_flag
            
            for (i=0;i<N_st_lead;i++)
                lead_veh[i]=0.0;    
        }
   }
   else //if (pltn_size >= 3)
   { 
        if  (control_pos_int==1)
        {
            subj_veh[0]=*(data_in[control_pos_int][42]); ////global synch t;
            subj_veh[1]=1; //grp_id;
            subj_veh[2]=pltn_size; //grp_size;
            subj_veh[3]=4; //grp_mode;   // 0: stopped;   1: manual; 2: CC; 3: ACC; 4: CACC
            subj_veh[4]=1; //grp_man_des; // CACC; 2; Pltoon
            subj_veh[5]=1; //grp_man_id;   // CACC; 2; Pltoon
            subj_veh[6]=*(data_in[7][0]); // veh id
            subj_veh[7]=0; // fault_mode                                   
            subj_veh[8]=0; //cut_in_flag;
            subj_veh[9]=1; //control_pos;                        
            subj_veh[10]=0; //man_des;
            subj_veh[11]=0; //man_id;              
            subj_veh[12]=0; //drive_mode;
            subj_veh[13]=0; //rel_v_to_pre;
            subj_veh[14]=0; //rel_d_to_pre;
            subj_veh[15]=0; //rel_v_to_lead;
            subj_veh[16]=0; //rel_d_to_lead;            
            subj_veh[17]=0; //des_d_gap_to_pre;
            subj_veh[18]=0; //des_d_gap_to_lead;  
            subj_veh[19]=0; // dsrc_status
            subj_veh[20]=0; // Emergent_brk_flag
            subj_veh[21]=0; //user_def_1
            subj_veh[22]=0; //user_def_2
            subj_veh[23]=0; //user_def_3
            subj_veh[24]=0; //user_def_4
            subj_veh[25]=0; //user_def_5  
            for (i=0;i<N_st_pre;i++)
                pre_veh[i]=0.0;
            for (i=0;i<N_st_lead;i++)
                lead_veh[i]=0.0;                      
        }
        else if  (control_pos_int==2)
        {
            subj_veh[0]=*(data_in[control_pos_int][42]); ////global synch t;
            subj_veh[1]=1.0; //grp_id;
            subj_veh[2]=2.0; //grp_size;
            subj_veh[3]=4.0; //grp_mode;
            subj_veh[4]=41.0; //grp_man_des;
            subj_veh[5]=41.0; //grp_man_id;
            subj_veh[6]=*(data_in[7][0]); //veh_id;
            // use BSM1
            if (comm_pre_ini_sw == 0)
            {
               comm_pre_buff=*(data_in[1][40]);  // comm_count
                comm_pre_ini_sw=1;
            }            
            if (fabs(comm_pre_buff-(*(data_in[1][40]))<0.1))
                comm_pre_loss_count++;
            else            
                comm_pre_loss_count=0;
            comm_pre_buff=*(data_in[1][40]);                   
            if (comm_pre_loss_count > Max_comm_loss)
            {
                comm_pre_fault=0;  // 1 
                comm_fault=0;
            }
            else
            {
                comm_pre_fault=0; 
               comm_fault=0;
            }
            if (comm_pre_fault==1)
            {
                subj_veh[7]=1.0; // dsrc_status          
                subj_veh[8]=2.0; //veh_fault_id; 0: no fault; 1: radar fault; 2: DSRC fault; 3: both radra and DSCR fault 
            }
            else
            {
                subj_veh[7]=0; // dsrc_status          
                subj_veh[8]=0; //veh_fault_id; 
            }   
            subj_veh[9]=0.0; //cut-in flag;
            subj_veh[10]=control_pos_int; //control_pos;
            subj_veh[11]=41.0; //man_des;              
            subj_veh[12]=41.0; //man_id;
            if ( *(data_in[6][1]) > 0.5 && (*(data_in[6][2]) > 0.5) )  // ACC & CACC sw are ON
                subj_veh[13]=4; //drive_mode;
            else 
                subj_veh[13]=1; //manual        
          
            subj_veh[14]=*(data_in[6][34]); //rel_v_to_pre; radar
            subj_veh[15]=*(data_in[6][33]); //rel_d_to_pre; radar
            
            spd=*(data_in[6][6]);
            radar_rg=*(data_in[6][33]);             
            if (comm_pre_fault==0)
            {   
                t_comm_pre+=delta_t;
                if (t_comm_pre < 1.0)
                    pre_dist_ini=radar_rg;
                pre_v=*(data_in[1][49]);
                pre_eps += (spd - pre_v) * delta_t;                                          
                pre_rg_obs = pre_dist_ini - pre_eps;     
                pre_dist= 0.85* pre_rg_obs + 0.15*radar_rg; 
                lead_dist=pre_dist;
                subj_veh[16]=pre_v-spd; //rel_v_to_lead;
                subj_veh[17]=lead_dist; //rel_d_to_lead;   
            }
            else
            {
                subj_veh[16]=*(data_in[6][34]); //rel_v_to_lead;
                subj_veh[17]=*(data_in[6][33]); //rel_d_to_lead;   
                t_comm_pre=0.0;
            }
                                         
            subj_veh[18]=(*(data_in[6][6])) *(*(data_in[6][35])); //des_d_gap_to_pre= T-Gap * v;          
            subj_veh[19]=(*(data_in[6][6])) *(*(data_in[6][35])); //des_d_gap_to_lead;  
      
            subj_veh[20]=0; //Emergent brk flag
            subj_veh[21]=0; //user_def_1
            subj_veh[22]=0; //user_def_2
            subj_veh[23]=0; //user_def_3
            subj_veh[24]=0; //user_def_4
            subj_veh[25]=0; //user_def_5   
            
            pre_veh[0]=((unsigned int)*(data_in[1][0]))& 0x2; //pre_acc_sw;
            pre_veh[1]=((unsigned int)*(data_in[1][0]))& 0x20; //pre_cacc_sw;
            pre_veh[2]=*(data_in[1][26]); //pre_fault_mode;
            pre_veh[3]=*(data_in[1][25]); //pre_control_pos;
            pre_veh[4]=*(data_in[1][0]); //pre_drive_mode;
            pre_veh[5]=*(data_in[1][27]); //pre_man_des;
            pre_veh[6]=*(data_in[1][68]); //pre_a;  // PinPoint accel; but JBus accel not passed; a_des
            pre_veh[7]=*(data_in[1][49]); //pre_v;
            pre_veh[8]=*(data_in[1][35]); //pre_d;
            pre_veh[9]=*(data_in[1][37]); //pre_v_ref;
            pre_veh[10]=*(data_in[1][35]); //pre_d_ref;
            pre_veh[11]=*(data_in[1][29]); //pre_radar_range
            pre_veh[12]=*(data_in[1][30]); //pre_radar_rate
            pre_veh[13]=*(data_in[1][57]); //pre_brk_pedal_deflect
            pre_veh[14]=*(data_in[1][37]); //pre_v_des
            pre_veh[15]=*(data_in[1][38]); //pre_torq_des
            pre_veh[16]=*(data_in[1][66]); //pre_run_flag
            
            lead_veh[0]=(unsigned int)*(data_in[1][0])& 0x2; //lead_acc_sw;
            lead_veh[1]=(unsigned int)*(data_in[1][0])& 0x20; //lead_cacc_sw;
            lead_veh[2]=*(data_in[1][26]); //lead_fault_mode;
            lead_veh[3]=*(data_in[1][25]); //lead_control_pos;
            lead_veh[4]=*(data_in[1][0]); //lead_drive_mode;
            lead_veh[5]=*(data_in[1][27]); //lead_man_des;
            lead_veh[6]=*(data_in[1][68]); //lead_a;  // PinPoint accel; but JBus accel not passed; a_des
            lead_veh[7]=*(data_in[1][49]); //lead_v;
            lead_veh[8]=*(data_in[1][35]); //lead_d;
            lead_veh[9]=*(data_in[1][37]); //lead_v_ref;
            lead_veh[10]=*(data_in[1][35]); //lead_d_ref;
            lead_veh[11]=*(data_in[1][29]); //lead_radar_range
            lead_veh[12]=*(data_in[1][30]); //lead_radar_rate
            lead_veh[13]=*(data_in[1][57]); //lead_brk_pedal_deflect
            lead_veh[14]=*(data_in[1][37]); //lead_v_des
            lead_veh[15]=*(data_in[1][38]); //lead_torq_des 
            lead_veh[16]=*(data_in[1][66]); //lead_run_flag
        }
        else // if (control_pos_int > 2)
        {
            subj_veh[0]=*(data_in[control_pos_int][42]); ////global synch t;
            subj_veh[1]=1.0; //grp_id;
            subj_veh[2]=2.0; //grp_size;
            subj_veh[3]=4.0; //grp_mode;
            subj_veh[4]=41.0; //grp_man_des;
            subj_veh[5]=41.0; //grp_man_id;
            subj_veh[6]=*(data_in[7][0]); //veh_id;
            // use BSM1
            if (comm_pre_ini_sw == 0)
            {
               comm_pre_buff=*(data_in[control_pos_int-1][40]);  // comm_count
                comm_pre_ini_sw=1;
            }            
            if (fabs(comm_pre_buff-(*(data_in[control_pos_int-1][40]))<0.1))
                comm_pre_loss_count++;
            else             
                comm_pre_loss_count=0;
            comm_pre_buff=*(data_in[control_pos_int-1][40]);                      
            if (comm_pre_loss_count > Max_comm_loss)
                comm_pre_fault=0;   
             else
                comm_pre_fault=0;  
            if (comm_lead_ini_sw == 0)
            {
               comm_lead_buff=*(data_in[1][40]);  // comm_count
               comm_lead_ini_sw=1;
            }            
            if (fabs(comm_lead_buff-(*(data_in[1][40]))<0.1))
                comm_lead_loss_count++;
            else            
                comm_lead_loss_count=0;
            comm_lead_buff=*(data_in[1][40]);                      
            if (comm_lead_loss_count > Max_comm_loss)
                comm_lead_fault=0;    
            else
                comm_lead_fault=0;  
            if (comm_pre_fault==0 && comm_lead_fault==1)
                comm_fault=1;
             if (comm_pre_fault==1 && comm_lead_fault==0)
                comm_fault=2;
            if (comm_pre_fault==1 && comm_lead_fault==1)
                comm_fault=3;
            if (comm_pre_fault==0 && comm_lead_fault==0)
                comm_fault=0;
            if (comm_fault>=0.9)
            {
                subj_veh[7]=1.0; // dsrc_status          
                subj_veh[8]=2.0; //veh_fault_id; 0: no fault; 1: radar fault; 2: DSRC fault; 3: both radra and DSCR fault 
            }
            else
            {
                subj_veh[7]=0; // dsrc_status          
                subj_veh[8]=0; //veh_fault_id; 
            }         
            subj_veh[9]=0.0; //cut-in flag;
            subj_veh[10]=control_pos_int; //control_pos;
            subj_veh[11]=41.0; //man_des;              
            subj_veh[12]=41.0; //man_id;
            if ( *(data_in[6][1]) > 0.5 && (*(data_in[6][2]) > 0.5) )  // ACC & CACC sw are ON
                subj_veh[13]=4; //drive_mode;
            else 
                subj_veh[13]=1; //manual        
          
            subj_veh[14]=*(data_in[6][34]); //rel_v_to_pre; radar
            subj_veh[15]=*(data_in[6][33]); //rel_d_to_pre; radar
                      
            spd=*(data_in[6][6]);
            radar_rg=*(data_in[6][33]);   
            if (comm_lead_fault==0)
            {
            	t_comm_lead+=delta_t;
                t_comm_pre+=delta_t;
                if (t_comm_lead < 1.0)
                {
                    pre_dist_ini=radar_rg;
                    lead_dist_ini=radar_rg+(*(data_in[control_pos_int-1][31]));
                }
                pre_v=*(data_in[1][49]);
                pre_eps += (spd - pre_v) * delta_t;                                          
                pre_rg_obs = pre_dist_ini - pre_eps;     
                pre_dist= 0.85* pre_rg_obs + 0.15*radar_rg; 
                
                lead_v=*(data_in[1][49]);
                lead_eps += (spd - lead_v) * delta_t;                                          
                lead_rg_obs = lead_dist_ini - lead_eps;     
                lead_dist= 0.85* lead_rg_obs + 0.15* (*(data_in[control_pos_int-1][31])+pre_dist);                 
                
                subj_veh[16]=lead_v-spd; //rel_v_to_lead;
                subj_veh[17]=lead_dist; //rel_d_to_lead;   
            }       
                       
            subj_veh[18]=(*(data_in[6][6])) *(*(data_in[6][35])); //des_d_gap_to_pre= T-Gap * v;          
            subj_veh[19]= subj_veh[18]+(*(data_in[control_pos_int-1][36])); //des_d_gap_to_lead;  
      
            subj_veh[20]=0; //Emergent brk flag
            subj_veh[21]=0; //user_def_1
            subj_veh[22]=0; //user_def_2
            subj_veh[23]=0; //user_def_3
            subj_veh[24]=0; //user_def_4
            subj_veh[25]=0; //user_def_5  
            
            pre_veh[0]=((unsigned int)*(data_in[control_pos_int-1][0])) & 0x2; //pre_acc_sw;
            pre_veh[1]=((unsigned int)*(data_in[control_pos_int-1][0])) & 0x20; //pre_cacc_sw;
            pre_veh[2]=*(data_in[control_pos_int-1][26]); //pre_fault_mode;
            pre_veh[3]=*(data_in[control_pos_int-1][25]); //pre_control_pos;
            pre_veh[4]=*(data_in[control_pos_int-1][67]); //pre_drive_mode;
            pre_veh[5]=*(data_in[control_pos_int-1][27]); //pre_man_des;
            pre_veh[6]=*(data_in[control_pos_int-1][68]); //pre_a;  // PinPoint accel[55]; but JBus accel not passed; a_des in [68]
            pre_veh[7]=*(data_in[control_pos_int-1][49]); //pre_v;
            pre_veh[8]=*(data_in[control_pos_int-1][35]); //pre_d;
            pre_veh[9]=*(data_in[control_pos_int-1][37]); //pre_v_ref;
            pre_veh[10]=*(data_in[control_pos_int-1][35]); //pre_d_ref;
            pre_veh[11]=*(data_in[control_pos_int-1][29]); //pre_radar_range
            pre_veh[12]=*(data_in[control_pos_int-1][30]); //pre_radar_rate
            pre_veh[13]=*(data_in[control_pos_int-1][57]); //pre_brk_pedal_deflect
            pre_veh[14]=*(data_in[control_pos_int-1][37]); //pre_v_des
            pre_veh[15]=*(data_in[control_pos_int-1][38]); //pre_torq_des
            pre_veh[16]=*(data_in[control_pos_int-1][66]); //pre_run_flag
            
            // Lead veh coord info
            lead_veh[0]=(unsigned int)*(data_in[1][0])& 0x2; //lead_acc_sw;
            lead_veh[1]=(unsigned int)*(data_in[1][0])& 0x20; //lead_cacc_sw;
            lead_veh[2]=*(data_in[1][26]); //lead_fault_mode;
            lead_veh[3]=*(data_in[1][25]); //lead_control_pos;
            lead_veh[4]=*(data_in[1][67]); //lead_drive_mode;
            lead_veh[5]=*(data_in[1][27]); //lead_man_des;
            lead_veh[6]=*(data_in[1][68]); //lead_a;  // PinPoint accel; but JBus accel not passed; we passed a_des
            lead_veh[7]=*(data_in[1][49]); //lead_v;
            lead_veh[8]=*(data_in[1][35]); //lead_d;
            lead_veh[9]=*(data_in[1][37]); //lead_v_ref;
            lead_veh[10]=*(data_in[1][35]); //lead_d_ref;
            lead_veh[11]=*(data_in[1][29]); //lead_radar_range
            lead_veh[12]=*(data_in[1][30]); //lead_radar_rate
            lead_veh[13]=*(data_in[1][57]); //lead_brk_pedal_deflect
            lead_veh[14]=*(data_in[1][37]); //lead_v_des
            lead_veh[15]=*(data_in[1][38]); //lead_torq_des
            lead_veh[16]=*(data_in[1][66]); //lead_run_flag
        }            
   } 
   
   // Output
    for (i=1;i<=pltn_size;i++)
   {
       if (control_pos_int == i)
         y[i]=1; 
       else
         y[i]=0;   
   }
   for (i=0;i<N_st_subj;i++)
       y1[i]=subj_veh[i];   
   for (i=0;i<N_st_pre;i++)
       y2[i]=pre_veh[i];
   for (i=0;i<N_st_lead;i++)
       y3[i]=lead_veh[i];   
  
    
} //end mdlOutputs



#undef	MDL_UPDATE                                                  //
#if	defined( MDL_UPDATE )                                           //
static	void	mdlUpdate( SimStruct *S, int_T tid )				//
{
}
#endif										



#undef	MDL_DERIVATIVES								//
#if	defined( MDL_DERIVATIVES )						//
static	void	mdlDerivatives( SimStruct *S )					
{
}
#endif										


static	void	mdlTerminate( SimStruct *S )					//
{
    //int i;
    
     
    free_dvector(subj_veh, 0, N_st_subj-1);
    free_dvector(pre_veh, 0, N_st_pre-1);
    free_dvector(lead_veh, 0, N_st_lead-1);
    free_ivector(comm_count, 0, pltn_size-1);
   
       
     
}

/////////////////////////
// NR functions
////////////////////////
static void nrerror(char error_text[])
/* Numerical Recipes standard error handler */
{
	fprintf(stderr,"Numerical Recipes run-time error...\n");
	fprintf(stderr,"%s\n",error_text);
	fprintf(stderr,"...now exiting to system...\n");
	exit(1);
}

static double *dvector(int nl, int nh)
/* allocate a double vector with subscript range v[nl..nh] */
{
	double *v;
   
	v=(double *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(double)));
	if (!v) nrerror("allocation failure in dvector()");
	return v-nl+NR_END;
}
   
static void free_dvector(double *v, int nl, int nh)
/* free a double vector allocated with dvector() */
{
	free((FREE_ARG) (v+nl-NR_END));
}

static int *ivector(long nl, long nh)
/* allocate an int vector with subscript range v[nl..nh] */
{
	int *v;
   
	v=(int *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(int)));
	if (!v) nrerror("allocation failure in ivector()");
	return v-nl+NR_END;
}
static void free_ivector(int *v, long nl, long nh)
/* free an int vector allocated with ivector() */
{
	free((FREE_ARG) (v+nl-NR_END));
}

static int dmin(int a, int b)
{
    if (a<b)
        return a;
    else
        return b;
}

static int dmax(int a, int b)
{
    if (a<b)
        return b;
    else
        return a;
}

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





#ifdef	MATLAB_MEX_FILE							
	#include	"simulink.c"					
#else                                             
	#include	"cg_sfun.h"                       
#endif
