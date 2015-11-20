/* ----------------------------------------------------------------------

	CACC Coordination	

---------------------------------------------------------------------- */

#define	S_FUNCTION_NAME		coordination
#define	S_FUNCTION_LEVEL	2

#include "simstruc.h"
#include "math.h"


#include <stdio.h>
//#include "veh_long.h"
#define NR_END 1
#define FREE_ARG char*
   
#define veh_1   1
#define veh_2   2
#define veh_3   3
#define veh_4   4
#define veh_5   5

#define pltn_size    5
#define N_torc_dat   33
#define N_sens_dat   20
#define N_dsrc_dat   63
#define veh_pos_index   8   // this needs to be corrected
#define Max_comm_loss                   10
#define Max_comm_count                  255



int i,j;
//int subj_veh_man_des;
static int drive_mode_sw=0;
static int comm_loss_count[4]={0,0,0,0};
static int comm_ini_sw[4]={0,0,0,0};
static  double comm_buff[4]={0.0,0.0,0.0,0.0};
static int comm_fault_mode=0;
static int comm_count[pltn_size]={0,0,0,0,0};

        
double *sens_dat;
double *torc_pos_dat;
double **dsrc_dat;

FILE *out_f;

void nrerror(char error_text[]);
double *dvector( int nl, int nh);
void free_dvector(double *v, int nl, int nh);
double **dmatrix(int nrl, int nrh, int ncl, int nch);
void free_dmatrix(double **m, int nrl, int nrh, int ncl, int nch);
int dmin(int a, int b);
int dmax(int a, int b);
double fmin(double a, double b);
double fmax(double a, double b);

//static unsigned short subj_veh_pos=1;

// parameters for output
// static int subj_grp_id=0;
// static int subj_grp_size=5;
// static int subj_grp_mode=0;
static int subj_grp_man_des=0;
static int subj_grp_man_id=0;
static int subj_veh_id=1001;   // unique to each vehicle for debugging
// static int subj_cut_in_flag=0;
static int subj_veh_pos=1;
// static int subj_veh_fault_id=0;
 static int subj_veh_man_des=0;
 static int subj_veh_man_id=0;
// static int subj_comm_count=0;
// double subj_dist_to_lead;
// double subj_rel_v_to_lead;
// double subj_d_gap_des_to_pre;
// double subj_t_gap_des_to_lead;
// double subj_d_gap_des_to_lead;
// double subj_t_gap_to_pre_est;
// double subj_d_gap_to_pre_est;
// double subj_t_gap_to_lead_est;
// double subj_d_gap_to_lead_est;
// double subj_radar_status;
// double subj_dsrc_status;
// double subj_acc_cacc_flag;
// static int pre_veh_man_des=0;
// static int pre_veh_man_id=0;
// double pre_veh_v_comm=0.0;
// static int lead_veh_man_des=0;
// static int lead_veh_man_id=0;
// double lead_veh_v_comm=0.0;


//if (!ssSetNumInputPorts(S, 6)) return;   

static	void	mdlInitializeSizes( SimStruct *S )												// this functin should be called just once atthe start
{

	ssSetNumSFcnParams( S, 0 );																	//
	ssSetNumContStates( S, 0 );																	//
	ssSetNumDiscStates( S, 0 );																	//
	ssSetNumInputPorts( S, 1 );																	//
    ssSetInputPortWidth( S, 0, 21);			
	
    
	
	ssSetInputPortDirectFeedThrough( S, 0, 1 );													//
	
    
	ssSetNumOutputPorts( S, 1 );																//
	ssSetOutputPortWidth( S, 0, 20 );															// sensor-CAN
	
    
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

	ssSetSampleTime( S, 0, 0.1 );																//
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
       // int i; //j; 
        
        sens_dat=dvector(0, N_sens_dat-1);
        torc_pos_dat=dvector(0, N_torc_dat-1);
        dsrc_dat=dmatrix(0, pltn_size-2, 0, N_dsrc_dat-2);
        //for(i=0;i<pltn_size;i++)
         //   dsrc_dat[i]=dvector(0, N_dsrc_dat-1);
         
        // Open files for writing
        out_f=fopen("out_dat.txt","w");
	
    }
#endif /*  MDL_START */

																					//
static	void	mdlOutputs( SimStruct *S, int_T tid )											// read CAN and comm info and pass out
{

	InputRealPtrsType	torc_in			= ssGetInputPortRealSignalPtrs( S, 0 );   // get RT info of current veh info: dim=33
    InputRealPtrsType	sens_in			= ssGetInputPortRealSignalPtrs( S, 1 );   // get RT info of current veh info: dim=20
	InputRealPtrsType	dsrc1_in		= ssGetInputPortRealSignalPtrs( S, 2 );   // get RT info DSRC: dim=63; already sorted in sequence
	InputRealPtrsType	dsrc2_in		= ssGetInputPortRealSignalPtrs( S, 3 );   // get RT info DSRC:  dim=63; already sorted in sequence
    InputRealPtrsType	dsrc3_in		= ssGetInputPortRealSignalPtrs( S, 4 );   // get RT info DSRC: dim=63; already sorted in sequence
	InputRealPtrsType	dsrc4_in		= ssGetInputPortRealSignalPtrs( S, 5 );   // get RT info DSRC:  dim=63; already sorted in sequence
   
	

	real_T			*y					= ssGetOutputPortRealSignal( S, 0 );    // output CAN data of subject veh
	real_T			*y1					= ssGetOutputPortRealSignal( S, 1 );    // output DSRC of first
	real_T			*y2					= ssGetOutputPortRealSignal( S, 2 );    // output DSRC of 2nd veh
	real_T			*y3					= ssGetOutputPortRealSignal( S, 3 );    // output DSRC of 3rd veh
	real_T			*y4					= ssGetOutputPortRealSignal( S, 4 );    // output DSRC of 4th veh
	real_T			*y5					= ssGetOutputPortRealSignal( S, 5 );    // output coordination parmeters
    real_T			*y6					= ssGetOutputPortRealSignal( S, 6 );    // output TORC data


// CAN data
for (i=0;i<N_sens_dat;i++)
    y[i]=(double)(*sens_in[i]);  // direct feed through 
// Torc data
for (i=0;i<N_torc_dat;i++)
    y6[i]=(double)(*torc_in[i]);  // direct feed through
    
for (i=0;i<N_dsrc_dat;i++) 
{
    if (pltn_size == 2)
    {
      dsrc_dat[0][i]=(double)(*dsrc1_in[i]);
      dsrc_dat[1][i]=0.0;
      dsrc_dat[2][i]=0.0;
      dsrc_dat[3][i]=0.0;
      //dsrc_dat[4][i]=0.0;  
    }
    if (pltn_size == 3)
     {
      dsrc_dat[0][i]=(double)(*dsrc1_in[i]);
      dsrc_dat[1][i]=(double)(*dsrc2_in[i]);
      dsrc_dat[2][i]=0.0;
      dsrc_dat[3][i]=0.0;
      //dsrc_dat[4][i]=0.0;  
    }
     if (pltn_size == 4)
     {
      dsrc_dat[0][i]=(double)(*dsrc1_in[i]);
      dsrc_dat[1][i]=(double)(*dsrc2_in[i]);
      dsrc_dat[2][i]=(double)(*dsrc3_in[i]);
      dsrc_dat[3][i]=0.0;
      //dsrc_dat[4][i]=0.0;  
    }
     if (pltn_size == 5)
     {
      dsrc_dat[0][i]=(double)(*dsrc1_in[i]);
      dsrc_dat[1][i]=(double)(*dsrc2_in[i]);
      dsrc_dat[2][i]=(double)(*dsrc3_in[i]);
      dsrc_dat[3][i]=(double)(*dsrc4_in[i]);
      //dsrc_dat[4][i]=0.0;  
    }
    
}
    subj_veh_pos=1;   // This needs to be determined from positing algorithm
    

    // The index is relative to the subject vehicle: immediate fron: 1; one veh ahead: 2; ... that's why there is an ibdex shift later
    // Such confusion is not necessary, and should be avoided;   XYLU 05_29_14



	switch (subj_veh_pos)
    {            // assign output; output index is determined by vehicle position; different vehicles are differnt
                                         // this has been shifted in the order described by Chris: 
		
        case veh_1:
			for(i=0;i<pltn_size;i++)
            {
                if (dsrc_dat[i][veh_pos_index] == veh_1) // lead veh in a coupled group or a single veh
                {
                    // Add PATH ACC Control Here if we do not use built-in ACC
                    
                }                
            }
            for (j=0;j<N_dsrc_dat;j++)
            {
                y1[j]=0;  
                y2[j]=0;  
                y3[j]=0;            
                y4[j]=0;
            }
            // activation
            y5[0]=1;
            y5[1]=0;
            y5[2]=0;
            y5[3]=0;
            y5[4]=0;
            
            // drive mode
            drive_mode_sw=(*sens_in[13]);  // 0: manual; 1: ACC; 
             
            subj_veh_man_des= dmin(drive_mode_sw,1);    // it can only be manual or ACC                 
            subj_veh_man_id=subj_veh_man_des;
            
            // DSRC comm count
            (comm_count[0])++;
            if (comm_count[0] > Max_comm_count)
                comm_count[0]=0;
            
            // Coordination output     
            y5[5]=0;                                    //grp_id;
            y5[6]=5;                                    //grp_size;
            y5[7]=0;                                    //grp_mode;
            y5[8]=2;                                    //grp_man_des;
            y5[9]=2;                                    //grp_man_id;
            y5[10]=(double)subj_veh_id;                 //veh_id; given unique ID 
            y5[11]=0;                                   //cut_in_flag;
            y5[12]=(double)subj_veh_pos;                //veh_pos_in_grp;
            y5[13]=0;                                   //veh_fault_id;
            y5[14]=(double)subj_veh_man_des;            //veh_man_des;
            y5[15]=(double)subj_veh_man_des;            //veh_man_id;
            y5[16]=comm_count[0];                       //comm_count;
            y5[17]=(*sens_in[2]);                       //dist_to_lead;
            y5[18]=(*sens_in[3]);                       //rel_v_to_lead;
            y5[19]=(*sens_in[1])*(*sens_in[12]);        //d_gap_des_to_pre;
            y5[20]=0;                                   //t_gap_des_to_lead;
            y5[21]=0;                                   //d_gap_des_to_lead;
            y5[22]=(*sens_in[2])/fmax((*sens_in[1]),1);  //t_gap_to_pre_est;
            y5[23]=(*sens_in[2]);                       //d_gap_to_pre_est;
            y5[24]=0;                                   //t_gap_to_lead_est;
            y5[25]=0;                                   //d_gap_to_lead_est;
            y5[26]=0;                                   //radar_status;
            y5[27]=0;                                   //dsrc_status;
            y5[28]=1;                                   //acc_cacc_flag;
            y5[29]=0;                                   //pre_man_des;
            y5[30]=0;                                   //pre_man_id;
            y5[31]=(*sens_in[3]);                       //pre_v_comm;            
            y5[32]=0;                                   //lead_man_des;
            y5[33]=0;                                   //lead_man_id;
            y5[34]=0;                                   //lead_v_comm;

            
            
			break;
            
        case veh_2:
			for(i=0;i<pltn_size;i++)
            {
                if (dsrc_dat[i][veh_pos_index] == veh_2) // immediately front
                {
                    for (j=0;j<N_dsrc_dat;j++)
                        y1[j]=dsrc_dat[0][j];
                }                
            }
            for (j=0;j<N_dsrc_dat;j++)
            {
                y2[j]=0;  
                y3[j]=0;            
                y4[j]=0;
            }
            // activation
            y5[0]=0;
            y5[1]=1;
            y5[2]=0;
            y5[3]=0;
            y5[4]=0;
            
            // drive mode
            drive_mode_sw=(*sens_in[13]);  // 0: manual; 1: ACC; 2: CACC if front veh is ACC
             
            if (drive_mode_sw>1.5 && drive_mode_sw<2.5 && (*dsrc1_in[10]) > 1.5 &&  (*dsrc1_in[10]) < 2.5)            
                subj_veh_man_des= 2;
            else
                subj_veh_man_des=dmin(drive_mode_sw,1);    
                        
            
             // comm fault detection
            if (comm_ini_sw[0] == 0)
            {
                comm_buff[0]=(*dsrc1_in[12]);
                comm_ini_sw[0]=1;
            }           
            if (fabs(comm_buff[0]-(*dsrc1_in[12]))<0.1)
                (comm_loss_count[0])++;
            else 
            {
                comm_loss_count[0]=0;
                comm_buff[0]=(*dsrc1_in[12]);
            }
            if (comm_loss_count[0] > Max_comm_loss)
                comm_fault_mode=1;
                               
            // DSRC comm count
            (comm_count[1])++;
            if (comm_count[1] > Max_comm_count)
                comm_count[1]=0;
            
            // Coordination output     
            y5[5]=0;                                    //grp_id;
            y5[6]=5;                                    //grp_size;
            y5[7]=0;                                    //grp_mode;
            y5[8]=2;                                    //grp_man_des;
            y5[9]=2;                                    //grp_man_id;
            y5[10]=(double)subj_veh_id;                 //veh_id; given unique ID 
            y5[11]=0;                                   //cut_in_flag;
            y5[12]=(double)subj_veh_pos;                //veh_pos_in_grp;
            y5[13]=0;                                   //veh_fault_id;
            y5[14]=(double)subj_veh_man_des;            //veh_man_des;
            y5[15]=(double)subj_veh_man_des;            //veh_man_id;
            y5[16]=comm_count[1];                       //comm_count;
            y5[17]=(*sens_in[2])+(*dsrc1_in[14]);                       //dist_to_lead;
            y5[18]=(*sens_in[3])+(*dsrc1_in[15]);                       //rel_v_to_lead;
            y5[19]=(*sens_in[1])*(*sens_in[12]);        //d_gap_des_to_pre;
            y5[20]=(*sens_in[12])+(*dsrc1_in[19]);         //t_gap_des_to_lead;
            y5[21]=((*sens_in[12])+(*dsrc1_in[19]))*(*sens_in[1]);    //d_gap_des_to_lead;
            y5[22]=(*sens_in[2])/fmax((*sens_in[1]),1);  //t_gap_to_pre_est;
            y5[23]=(*sens_in[2]);                       //d_gap_to_pre_est;
            y5[24]=((*sens_in[2])+(*dsrc1_in[14]))/fmax((*sens_in[1]),1);                                   //t_gap_to_lead_est;
            y5[25]=(*sens_in[2])+(*dsrc1_in[14]);                                   //d_gap_to_lead_est;
            y5[26]=0;                                   //radar_status;
            y5[27]=0;                                   //dsrc_status;
            y5[28]=1;                                   //acc_cacc_flag;
            y5[29]=(*dsrc1_in[10]);                     //pre_man_des;
            y5[30]=(*dsrc1_in[11]);                     //pre_man_id;
            y5[31]=(*dsrc1_in[13]);                     //pre_v_comm;            
            y5[32]=(*dsrc1_in[10]);                     //lead_man_des;
            y5[33]=(*dsrc1_in[11]);                     //lead_man_id;
            y5[34]=(*dsrc1_in[13]);                     //lead_v_comm;
 
			break;
            

		case veh_3:  // 2 vehicle in the front
			for(i=0;i<pltn_size;i++)
            {
                if (dsrc_dat[i][veh_pos_index] == veh_2)
                {
                    for (j=0;j<N_dsrc_dat;j++)
                        y2[j]=dsrc_dat[1][j];
                }
                 if (dsrc_dat[i][veh_pos_index] == veh_1)  // immediately front
                {
                    for (j=0;j<N_dsrc_dat;j++)
                        y1[j]=dsrc_dat[0][j];
                }
            }
            for (j=0;j<N_dsrc_dat;j++)
            {
                y3[j]=0;            
                y4[j]=0;
            }
            // activation
            y5[0]=0;
            y5[1]=0;
            y5[2]=1;
            y5[3]=0;
            y5[4]=0;
            
            // drive mode
            drive_mode_sw=(*sens_in[13]);  // 0: manual; 1: ACC; 2: CACC if front veh is ACC
            
            if (drive_mode_sw>1.5 && drive_mode_sw<2.5 && (*dsrc1_in[10]) > 1.5 &&  (*dsrc1_in[10]) < 2.5 && (*dsrc2_in[10]) > 1.5 &&  (*dsrc2_in[10]) < 2.5)            
                subj_veh_man_des= 2;
            else
                subj_veh_man_des=dmin(drive_mode_sw,1);    
           
            
             // comm fault detection
            if (comm_ini_sw[0] == 0)
            {
                comm_buff[0]=(*dsrc1_in[12]);
                comm_ini_sw[0]=1;
            }
            if (comm_ini_sw[1] == 0)
            {
                comm_buff[1]=(*dsrc2_in[12]);
                comm_ini_sw[1]=1;
            }           
            if (fabs(comm_buff[0]-(*dsrc1_in[12]))<0.1)
                (comm_loss_count[0])++;
            else 
            {
                comm_loss_count[0]=0;
                comm_buff[0]=(*dsrc1_in[12]);
            }
            if (fabs(comm_buff[1]-(*dsrc2_in[12]))<0.1)
                (comm_loss_count[1])++;
            else 
            {
                comm_loss_count[1]=0;
                comm_buff[1]=(*dsrc2_in[12]);
            }
            if (comm_loss_count[0] > Max_comm_loss || comm_loss_count[1] > Max_comm_loss)
                comm_fault_mode=1;                               
            
            // DSRC comm count
            (comm_count[2])++;
            if (comm_count[2] > Max_comm_count)
                comm_count[2]=0;
              
            // Coordination output     
            y5[5]=0;                                    //grp_id;
            y5[6]=5;                                    //grp_size;
            y5[7]=0;                                    //grp_mode;
            y5[8]=2;                                    //grp_man_des;
            y5[9]=2;                                    //grp_man_id;
            y5[10]=(double)subj_veh_id;                 //veh_id; given unique ID 
            y5[11]=0;                                   //cut_in_flag;
            y5[12]=(double)subj_veh_pos;                //veh_pos_in_grp;
            y5[13]=0;                                   //veh_fault_id;
            y5[14]=(double)subj_veh_man_des;            //veh_man_des;
            y5[15]=(double)subj_veh_man_des;            //veh_man_id;
            y5[16]=comm_count[2];                       //comm_count;
            y5[17]=(*sens_in[2])+(*dsrc1_in[14])+(*dsrc2_in[14]);                       //dist_to_lead;
            y5[18]=(*sens_in[3])+(*dsrc1_in[15])+(*dsrc2_in[15]);                       //rel_v_to_lead;
            y5[19]=(*sens_in[1])*(*sens_in[12]);                                        //d_gap_des_to_pre;
            y5[20]=(*sens_in[12])+(*dsrc1_in[19])+(*dsrc2_in[19]);                      //t_gap_des_to_lead;
            y5[21]=((*sens_in[12])+(*dsrc1_in[19])+(*dsrc2_in[19]))*(*sens_in[1]);      //d_gap_des_to_lead;
            y5[22]=(*sens_in[2])/fmax((*sens_in[1]),1);                                 //t_gap_to_pre_est;
            y5[23]=(*sens_in[2]);                                                       //d_gap_to_pre_est;
            y5[24]=((*sens_in[2])+(*dsrc1_in[14])+(*dsrc2_in[14]))/fmax((*sens_in[1]),1);               //t_gap_to_lead_est;
            y5[25]=(*sens_in[2])+(*dsrc1_in[14])+(*dsrc2_in[14]);                                   //d_gap_to_lead_est;
            y5[26]=0;                                   //radar_status;
            y5[27]=0;                                   //dsrc_status;
            y5[28]=1;                                   //acc_cacc_flag;
            y5[29]=(*dsrc2_in[10]);                     //pre_man_des;
            y5[30]=(*dsrc2_in[11]);                     //pre_man_id;
            y5[31]=(*dsrc2_in[13]);                     //pre_v_comm;            
            y5[32]=(*dsrc1_in[10]);                     //lead_man_des;
            y5[33]=(*dsrc1_in[11]);                     //lead_man_id;
            y5[34]=(*dsrc1_in[13]);                     //lead_v_comm;
 
			break;

		case veh_4: //	3 vehicles in the fornt: 
			for(i=0;i<pltn_size;i++)
            {                               
                if (dsrc_dat[i][veh_pos_index] == veh_3)
                {
                    for (j=0;j<N_dsrc_dat;j++)
                        y3[j]=dsrc_dat[2][j];
                }
                if (dsrc_dat[i][veh_pos_index] == veh_2)
                {
                    for (j=0;j<N_dsrc_dat;j++)
                        y2[j]=dsrc_dat[1][j];
                }
                 if (dsrc_dat[i][veh_pos_index] == veh_1)  // immediately front
                {
                    for (j=0;j<N_dsrc_dat;j++)
                        y1[j]=dsrc_dat[0][j];
                }
            }
            for (j=0;j<N_dsrc_dat;j++)
                y4[j]=0;
            // activation
            y5[0]=0;
            y5[1]=0;
            y5[2]=0;
            y5[3]=1;
            y5[4]=0;
            
             // drive mode
            
            drive_mode_sw=(*sens_in[13]);  // 0: manual; 1: ACC; 2: CACC if front veh is ACC
            
            if (drive_mode_sw>1.5 && drive_mode_sw<2.5 && (*dsrc1_in[10]) > 1.5 &&  (*dsrc1_in[10]) < 2.5 && (*dsrc2_in[10]) > 1.5 &&  (*dsrc2_in[10]) < 2.5 &&
                    (*dsrc3_in[10]) > 1.5 &&  (*dsrc3_in[10]) < 2.5)            
                subj_veh_man_des= 2;
            else
                subj_veh_man_des=dmin(drive_mode_sw,1);                        
            
             // comm fault detection
            if (comm_ini_sw[0] == 0)
            {
                comm_buff[0]=(*dsrc1_in[12]);
                comm_ini_sw[0]=1;
            }
            if (comm_ini_sw[1] == 0)
            {
                comm_buff[1]=(*dsrc2_in[12]);
                comm_ini_sw[1]=1;
            } 
            if (comm_ini_sw[2] == 0)
            {
                comm_buff[2]=(*dsrc3_in[12]);
                comm_ini_sw[2]=1;
            }       
            if (fabs(comm_buff[0]-(*dsrc1_in[12]))<0.1)
                (comm_loss_count[0])++;
            else 
            {
                comm_loss_count[0]=0;
                comm_buff[0]=(*dsrc1_in[12]);
            }
            if (fabs(comm_buff[1]-(*dsrc2_in[12]))<0.1)
                (comm_loss_count[1])++;
            else 
            {
                comm_loss_count[1]=0;
                comm_buff[1]=(*dsrc2_in[12]);
            }
            if (fabs(comm_buff[2]-(*dsrc3_in[12]))<0.1)
                (comm_loss_count[2])++;
            else 
            {
                comm_loss_count[2]=0;
                comm_buff[2]=(*dsrc3_in[12]);
            }
            if (comm_loss_count[0] > Max_comm_loss || comm_loss_count[1] > Max_comm_loss || comm_loss_count[2] > Max_comm_loss)
                comm_fault_mode=1;                       
                    
            // DSRC comm count
            (comm_count[3])++;
            if (comm_count[3] > Max_comm_count)
                comm_count[3]=0;
          
            // Coordination output     
            y5[5]=0;                                    //grp_id;
            y5[6]=5;                                    //grp_size;
            y5[7]=0;                                    //grp_mode;
            y5[8]=2;                                    //grp_man_des;
            y5[9]=2;                                    //grp_man_id;
            y5[10]=(double)subj_veh_id;                 //veh_id; given unique ID 
            y5[11]=0;                                   //cut_in_flag;
            y5[12]=(double)subj_veh_pos;                //veh_pos_in_grp;
            y5[13]=0;                                   //veh_fault_id;
            y5[14]=(double)subj_veh_man_des;            //veh_man_des;
            y5[15]=(double)subj_veh_man_des;            //veh_man_id;
            y5[16]=comm_count[3];                       //comm_count;
            y5[17]=(*sens_in[2])+(*dsrc1_in[14])+(*dsrc2_in[14])+(*dsrc3_in[14]);                       //dist_to_lead;
            y5[18]=(*sens_in[3])+(*dsrc1_in[15])+(*dsrc2_in[15])+(*dsrc3_in[15]);                       //rel_v_to_lead;
            y5[19]=(*sens_in[1])*(*sens_in[12]);                                        //d_gap_des_to_pre;
            y5[20]= (*sens_in[12])+(*dsrc1_in[19])+(*dsrc2_in[19])+(*dsrc3_in[19]);                      //t_gap_des_to_lead;
            y5[21]=((*sens_in[12])+(*dsrc1_in[19])+(*dsrc2_in[19])+(*dsrc3_in[19]))*(*sens_in[1]);      //d_gap_des_to_lead;
            y5[22]=(*sens_in[2])/fmax((*sens_in[1]),1);                                 //t_gap_to_pre_est;
            y5[23]=(*sens_in[2]);                                                       //d_gap_to_pre_est;
            y5[24]=((*sens_in[2])+(*dsrc1_in[14])+(*dsrc2_in[14])+(*dsrc3_in[14]))/fmax((*sens_in[1]),1);               //t_gap_to_lead_est;
            y5[25]= (*sens_in[2])+(*dsrc1_in[14])+(*dsrc2_in[14])+(*dsrc3_in[14]);                                   //d_gap_to_lead_est;
            y5[26]=0;                                   //radar_status;
            y5[27]=0;                                   //dsrc_status;
            y5[28]=1;                                   //acc_cacc_flag;
            y5[29]=(*dsrc3_in[10]);                     //pre_man_des;
            y5[30]=(*dsrc3_in[11]);                     //pre_man_id;
            y5[31]=(*dsrc3_in[13]);                     //pre_v_comm;            
            y5[32]=(*dsrc1_in[10]);                     //lead_man_des;
            y5[33]=(*dsrc1_in[11]);                     //lead_man_id;
            y5[34]=(*dsrc1_in[13]);                     //lead_v_comm;

			break;

        case veh_5:  // 4 vehicles in the front: 
			for(i=0;i<pltn_size;i++)
            {                
                if (dsrc_dat[i][veh_pos_index] == veh_4)
                {
                    for (j=0;j<N_dsrc_dat;j++)
                        y4[j]=dsrc_dat[3][j];
                }
                if (dsrc_dat[i][veh_pos_index] == veh_3)
                {
                    for (j=0;j<N_dsrc_dat;j++)
                        y3[j]=dsrc_dat[2][j];
                }
                if (dsrc_dat[i][veh_pos_index] == veh_2)
                {
                    for (j=0;j<N_dsrc_dat;j++)
                        y2[j]=dsrc_dat[1][j];
                }
                 if (dsrc_dat[i][veh_pos_index] == veh_1)  // immediately front
                {
                    for (j=0;j<N_dsrc_dat;j++)
                        y1[j]=dsrc_dat[0][j];
                }
            }
            // activation
            y5[0]=0;
            y5[1]=0;
            y5[2]=0;
            y5[3]=0;
            y5[4]=1;
            
            // drive mode
            drive_mode_sw=(*sens_in[13]);  // 0: manual; 1: ACC; 2: CACC if front veh is ACC 
            
            if (drive_mode_sw>1.5 && drive_mode_sw<2.5 && (*dsrc1_in[10]) > 1.5 &&  (*dsrc1_in[10]) < 2.5 && (*dsrc2_in[10]) > 1.5 &&  (*dsrc2_in[10]) < 2.5 &&
                    (*dsrc3_in[10]) > 1.5 &&  (*dsrc3_in[10]) < 2.5 && (*dsrc4_in[10]) > 1.5 &&  (*dsrc4_in[10]) < 2.5)            
                subj_veh_man_des= 2;
            else
                subj_veh_man_des=dmin(drive_mode_sw,1); 
                        
            // comm fault detection
            if (comm_ini_sw[0] == 0)
            {
                comm_buff[0]=(*dsrc1_in[12]);
                comm_ini_sw[0]=1;
            }
            if (comm_ini_sw[1] == 0)
            {
                comm_buff[1]=(*dsrc2_in[12]);
                comm_ini_sw[1]=1;
            } 
            if (comm_ini_sw[2] == 0)
            {
                comm_buff[2]=(*dsrc3_in[12]);
                comm_ini_sw[2]=1;
            }  
            if (comm_ini_sw[3] == 0)
            {
                comm_buff[3]=(*dsrc4_in[12]);
                comm_ini_sw[3]=1;
            }  
            if (fabs(comm_buff[0]-(*dsrc1_in[12])<0.1))
                (comm_loss_count[0])++;
            else 
            {
                comm_loss_count[0]=0;
                comm_buff[0]=(*dsrc1_in[12]);
            }
            if (fabs(comm_buff[1]-(*dsrc2_in[12])<0.1))
                (comm_loss_count[1])++;
            else 
            {
                comm_loss_count[1]=0;
                comm_buff[1]=(*dsrc2_in[12]);
            }
            if (fabs(comm_buff[2]-(*dsrc3_in[12])<0.1))
                (comm_loss_count[2])++;
            else 
            {
                comm_loss_count[2]=0;
                comm_buff[2]=(*dsrc3_in[12]);
            }
            if (fabs(comm_buff[3]-(*dsrc4_in[12]))<0.1)
                (comm_loss_count[3])++;
            else 
            {
                comm_loss_count[3]=0;
                comm_buff[3]=(*dsrc4_in[12]);
            }
            if (comm_loss_count[0] > Max_comm_loss || comm_loss_count[1] > Max_comm_loss || comm_loss_count[2] > Max_comm_loss || comm_loss_count[3] > Max_comm_loss)
                comm_fault_mode=1;
                        
            // DSRC comm count
            (comm_count[4])++;
            if (comm_count[4] > Max_comm_count)
                comm_count[4]=0;
                            
            // Coordination output     
            y5[5]=0;                                    //grp_id;
            y5[6]=5;                                    //grp_size;
            y5[7]=0;                                    //grp_mode;
            y5[8]=2;                                    //grp_man_des;
            y5[9]=2;                                    //grp_man_id;
            y5[10]=(double)subj_veh_id;                 //veh_id; given unique ID 
            y5[11]=0;                                   //cut_in_flag;
            y5[12]=(double)subj_veh_pos;                //veh_pos_in_grp;
            y5[13]=0;                                   //veh_fault_id;
            y5[14]=(double)subj_veh_man_des;            //veh_man_des;
            y5[15]=(double)subj_veh_man_des;            //veh_man_id;
            y5[16]=comm_count[4];                       //comm_count;
            y5[17]=(*sens_in[2])+(*dsrc1_in[14])+(*dsrc2_in[14])+(*dsrc3_in[14])+(*dsrc4_in[14]);                       //dist_to_lead;
            y5[18]=(*sens_in[3])+(*dsrc1_in[15])+(*dsrc2_in[15])+(*dsrc3_in[15])+(*dsrc4_in[15]);                       //rel_v_to_lead;
            y5[19]=(*sens_in[1])*(*sens_in[12]);                                        //d_gap_des_to_pre;
            y5[20]= (*sens_in[12])+(*dsrc1_in[19])+(*dsrc2_in[19])+(*dsrc3_in[19])+(*dsrc4_in[19]);                      //t_gap_des_to_lead;
            y5[21]=((*sens_in[12])+(*dsrc1_in[19])+(*dsrc2_in[19])+(*dsrc3_in[19])+(*dsrc4_in[19]))*(*sens_in[1]);      //d_gap_des_to_lead;
            y5[22]=(*sens_in[2])/fmax((*sens_in[1]),1);                                 //t_gap_to_pre_est;
            y5[23]=(*sens_in[2]);                                                       //d_gap_to_pre_est;
            y5[24]=((*sens_in[2])+(*dsrc1_in[14])+(*dsrc2_in[14])+(*dsrc3_in[14])+(*dsrc4_in[14]))/fmax((*sens_in[1]),1);               //t_gap_to_lead_est;
            y5[25]= (*sens_in[2])+(*dsrc1_in[14])+(*dsrc2_in[14])+(*dsrc3_in[14])+(*dsrc4_in[14]);                                   //d_gap_to_lead_est;
            y5[26]=0;                                   //radar_status;
            y5[27]=0;                                   //dsrc_status;
            y5[28]=1;                                   //acc_cacc_flag;
            y5[29]=(*dsrc4_in[10]);                     //pre_man_des;
            y5[30]=(*dsrc4_in[11]);                     //pre_man_id;
            y5[31]=(*dsrc4_in[13]);                     //pre_v_comm;            
            y5[32]=(*dsrc1_in[10]);                     //lead_man_des;
            y5[33]=(*dsrc1_in[11]);                     //lead_man_id;
            y5[34]=(*dsrc1_in[13]);                     //lead_v_comm;
			break;	
            
		default :
			for (j=0;j<N_dsrc_dat;j++)
            {
                y1[j]=0;
                y2[j]=0;
                y3[j]=0;
                y4[j]=0;                
            }
            // activation
            y5[0]=1;  // veh 1 always activated
            y5[1]=0;
            y5[2]=0;
            y5[3]=0;
            y5[4]=0;
                        
            y5[5]=0; //grp_id;
            y5[6]=5;  //grp_size;
            y5[7]=0;  //grp_mode;
            y5[8]=2;  //grp_man_des;
            y5[9]=2;  //grp_man_id;
            y5[10]=1;  //veh_id;
            y5[11]=0;  //cut_in_flag;
            y5[12]=1;  //veh_pos_in_grp;
            y5[13]=0;  //veh_fault_id;
            y5[14]=0;  //veh_man_des;
            y5[15]=0;  //veh_man_id;
            y5[16]=0;  //comm_count;
            y5[17]=10;  //dist_to_lead;
            y5[18]=0;  //rel_v_to_lead;
            y5[19]=0;  //d_gap_des_to_pre;
            y5[20]=0;  //t_gap_des_to_lead;
            y5[21]=0;  //d_gap_des_to_lead;
            y5[22]=0;  //t_gap_to_pre_est;
            y5[23]=0;  //d_gap_to_pre_est;
            y5[24]=0;  //t_gap_to_lead_est;
            y5[25]=0;  //d_gap_to_lead_est;
            y5[26]=0;  //radar_status;
            y5[27]=0;  //dsrc_status;
            y5[28]=0;  //acc_cacc_flag;
            y5[29]=0;  //pre_man_des;
            y5[30]=0;  //pre_man_id;
            y5[31]=0;  //pre_v_comm;
            y5[32]=0;  //lead_man_des;
            y5[33]=0;  //lead_man_id;
            y5[34]=0;  //lead_v_comm;
            
			break;
	}   // output block end
   
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
    int i;
    fflush(out_f);
    fclose(out_f);
    
    free_dvector(sens_dat, 0, N_sens_dat-1);
    free_dvector(torc_pos_dat, 0, N_torc_dat-1);    
    free_dmatrix(dsrc_dat, 0, pltn_size-1, 0, N_dsrc_dat-1);
     
}

/////////////////////////
// NR functions
////////////////////////
void nrerror(char error_text[])
/* Numerical Recipes standard error handler */
{
	fprintf(stderr,"Numerical Recipes run-time error...\n");
	fprintf(stderr,"%s\n",error_text);
	fprintf(stderr,"...now exiting to system...\n");
	exit(1);
}

double *dvector(int nl, int nh)
/* allocate a double vector with subscript range v[nl..nh] */
{
	double *v;
   
	v=(double *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(double)));
	if (!v) nrerror("allocation failure in dvector()");
	return v-nl+NR_END;
}
   
void free_dvector(double *v, int nl, int nh)
/* free a double vector allocated with dvector() */
{
	free((FREE_ARG) (v+nl-NR_END));
}

double **dmatrix(int nrl, int nrh, int ncl, int nch)
/* allocate a double matrix with subscript range m[nrl..nrh][ncl..nch] */
{
	long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
	double **m;
   
	/* allocate pointers to rows */
	m=(double **) malloc((size_t)((nrow+NR_END)*sizeof(double*)));
	if (!m) nrerror("allocation failure 1 in matrix()");
	m += NR_END;
	m -= nrl;
   
	/* allocate rows and set pointers to them */
	m[nrl]=(double *) malloc((size_t)((nrow*ncol+NR_END)*sizeof(double)));
	if (!m[nrl]) nrerror("allocation failure 2 in matrix()");
	m[nrl] += NR_END;
	m[nrl] -= ncl;
   
	for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;
   
	/* return pointer to array of pointers to rows */
	return m;
}

void free_dmatrix(double **m, int nrl, int nrh, int ncl, int nch)
/* free a double matrix allocated by dmatrix() */
{
	free((FREE_ARG) (m[nrl]+ncl-NR_END));
	free((FREE_ARG) (m+nrl-NR_END));
}

int dmin(int a, int b)
{
    if (a<b)
        return a;
    else
        return b;
}

int dmax(int a, int b)
{
    if (a<b)
        return b;
    else
        return a;
}

double fmin(double a, double b)
{
    if (a<b)
        return a;
    else
        return b;
}

double fmax(double a, double b)
{
    if (a<b)
        return b;
    else
        return a;
}

#ifdef	MATLAB_MEX_FILE								//
	#include	"simulink.c"						//
#else                                               //
	#include	"cg_sfun.h"                         //
#endif
