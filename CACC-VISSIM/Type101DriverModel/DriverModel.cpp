/*==========================================================================*/
/*  DriverModel.cpp                                  DLL Module for VISSIM  */
/*                                                                          */
/*  Interface module for external driver models.                            */
/*                                                                          */
/*  Version of 2016-03-21                                   Lukas Kautzsch  */
/*==========================================================================*/

/* ---------------------------- Revision Note ------------------------------*/
/* Nov. 2012																*/
/*  CACC Application Enhancement											*/
/*  Modified by Joyoung Lee, University of Virginia.						*/
/*
/* Jul. 2013
/*  CACC Lane changing Model Enhancement
/*  Modified by Joyoung Lee, University of Virginia
/*
/* May. 2014
/*  Algorithm Minor revision
/*  Modified by Joyoung Lee, New Jersey Institute of Technology
/*
/* Nov. 2014
/*  Revised for VISSIM 6 environment, Joyoung Lee NJIT
/*  Note : VISSIM 7 (up to -04) did not work; it kept crashing when the first CACC vehicle reaches at the position of about 80%  of link length.
/*
/* Dec. 2014
/*  Revised for the CACC Simulation Manager program, Joyoung Lee NJIT
/*
/* Feb. 2015
/* Code Block descriptions (input/output flow) were added.
/* July 30. 2015 : Major revision by Peng Su to implement Bart van Arem's algorithm
/* August 2015 : Enhanced from the revision of July 30.

/* Dec. 11: Added HWLong to CACC cars, for mixed traffic simulations. If leading car is not CACC, use HWLong
/* Mar. 21: Get rid of the platoon-size re-organization code, instead, use a loose average platoon size parameter.
/* 		The old re-organization logic kept on creating problems. The new logic works pretty well. 
/*==========================================================================*/


#include "DriverModel.h"
#include <stdio.h>
#include <iostream> 
#include <fstream>
#include <list>
#include <math.h>
#include <iostream>
#include <ctime>
#include <map>
#include <string> 

using namespace std;

/*==========================================================================*/

double  desired_acceleration = 0.0;
double  desired_lane_angle = 0.0; // Radian
long    active_lane_change = 0;
long    veh_active_lane_change = 0;
long    rel_target_lane = 0;
long	veh_rel_target_lane = 0;
double  desired_velocity = 0.0;
long    turning_indicator = 0;
long    vehicle_color = RGB(0, 0, 0);

//long veh_type = 101;
long current_link = 0;
long current_lane = 0;
long lanes_current_link = 0; // Dec. 15. 2014
double timestep = 0.0;
long current_veh = 0;
long vissim_suggestion = 0; // 0 indicates no: not listen to VISSIM, 1 otherwise
long simple_lanechange = 0;
long adj_veh;
long adj_veh_class;
char* dt;
time_t now;
long AdjVehicles[5][5];
double AdjVehiclesWidth[5][5];
double AdjVehiclesSpeed[5][5];
double AdjVehiclesDist[5][5];
double AdjVehiclesAcc[5][5];
long AdjVehiclesLaneChange[5][5];
long AdjVehiclesCurrentLane[5][5];
long AdjVehiclesTargetLane[5][5];

ofstream fout;//("out_newdll.txt",std::ios_base::app);
ofstream fout_ncacc;
ifstream fin;
string str;
char ch;

map<long, int> VehTargetLane;

double Spd_vehicle = 0.0;
double Acc_vehicle = 0.0;
double Leng_vehicle = 0.0;
double MaxAcc_vehicle = 0.0;
double lateral_pos = 0.0;
double lateral_pos_ind = 0.0;
double veh_od = 0.0;
bool WrtFlag = false;
double lane_angle = 0.021;


int MaxPlatoonSize = 5;
double HWShort = 0.6;
double HWLong = 2.0;  //headway when the vehicle in front is not Connected. Ego vehicle is actually in ACC mode.
int leaderID = 0;
int followerID = 0;
double front_dist = 0.0;
double rear_dist = 0.0;
double const head_speed = 26.3889;  // in meters per second

map<long, int[7]> platoonState;



/*  &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */




/*==========================================================================*/

BOOL APIENTRY DllMain(HANDLE  hModule,
	DWORD   ul_reason_for_call,
	LPVOID  lpReserved)
{
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

/*==========================================================================*/

DRIVERMODEL_API  int  DriverModelSetValue(long   type,
	long   index1,
	long   index2,
	long   long_value,
	double double_value,
	char   *string_value)
{
	/* Sets the value of a data object of type <type>, selected by <index1> */
	/* and possibly <index2>, to <long_value>, <double_value> or            */
	/* <*string_value> (object and value selection depending on <type>).    */
	/* Return value is 1 on success, otherwise 0.                           */

	switch (type) {
	case DRIVER_DATA_PATH:
	case DRIVER_DATA_TIMESTEP:
		return 1;
	case DRIVER_DATA_TIME:
		timestep = double_value;
		return 1;
	case DRIVER_DATA_VEH_ID:
		current_veh = long_value;
		return 1;
	case DRIVER_DATA_VEH_LANE:
		current_lane = long_value;
		return 1;
	case DRIVER_DATA_VEH_ODOMETER:
		veh_od = double_value;
		return 1;
	case DRIVER_DATA_VEH_LANE_ANGLE:
		return 1;
	case DRIVER_DATA_VEH_LATERAL_POSITION:
		lateral_pos = double_value;
		return 1;
	case DRIVER_DATA_VEH_VELOCITY:
		Spd_vehicle = double_value;
		return 1;
	case DRIVER_DATA_VEH_ACCELERATION:
		Acc_vehicle = double_value;
		return 1;
	case DRIVER_DATA_VEH_LENGTH:
	case DRIVER_DATA_VEH_WIDTH:
	case DRIVER_DATA_VEH_WEIGHT:
		return 1;
	case DRIVER_DATA_VEH_MAX_ACCELERATION:
		MaxAcc_vehicle = double_value;
		return 1;
	case DRIVER_DATA_VEH_TURNING_INDICATOR:
		turning_indicator = long_value;
		return 1;
	case DRIVER_DATA_VEH_CATEGORY:
	case DRIVER_DATA_VEH_PREFERRED_REL_LANE:
	case DRIVER_DATA_VEH_USE_PREFERRED_LANE:
		return 1;
	case DRIVER_DATA_VEH_DESIRED_VELOCITY:
		desired_velocity = double_value;
		return 1;
	case DRIVER_DATA_VEH_X_COORDINATE:
	case DRIVER_DATA_VEH_Y_COORDINATE:
		return 1;
	case DRIVER_DATA_VEH_TYPE:
		//veh_type = long_value;
		return 1;
	case DRIVER_DATA_VEH_COLOR:
		vehicle_color = long_value;
		return 1;
	case DRIVER_DATA_VEH_CURRENT_LINK:
		current_link = long_value;
		return 0; /* (To avoid getting sent lots of DRIVER_DATA_VEH_NEXT_LINKS messages) */
		/* Must return 1 if these messages are to be sent from VISSIM!         */
	case DRIVER_DATA_VEH_NEXT_LINKS:
	case DRIVER_DATA_VEH_ACTIVE_LANE_CHANGE:
		veh_active_lane_change = long_value;
		return 1;
	case DRIVER_DATA_VEH_REL_TARGET_LANE:
		veh_rel_target_lane = long_value;
		return 1;
	case DRIVER_DATA_NVEH_ID:
		AdjVehicles[index1 + 2][index2 + 2] = long_value;
		return 1;
	case DRIVER_DATA_NVEH_LANE_ANGLE:
	case DRIVER_DATA_NVEH_LATERAL_POSITION:
		return 1;
	case DRIVER_DATA_NVEH_DISTANCE:
		AdjVehiclesDist[index1 + 2][index2 + 2] = double_value;
		return 1;
	case DRIVER_DATA_NVEH_REL_VELOCITY:
		AdjVehiclesSpeed[index1 + 2][index2 + 2] = Spd_vehicle - double_value;
		return 1;
	case DRIVER_DATA_NVEH_ACCELERATION:
		AdjVehiclesAcc[index1 + 2][index2 + 2] = double_value;
		return 1;
	case DRIVER_DATA_NVEH_LENGTH:
		AdjVehiclesWidth[index1 + 2][index2 + 2] = double_value; // revised for VISSIM 7. 
		return 1;
	case DRIVER_DATA_NVEH_WIDTH:
		return 1;
	case DRIVER_DATA_NVEH_WEIGHT:
	case DRIVER_DATA_NVEH_TURNING_INDICATOR:
	case DRIVER_DATA_NVEH_CATEGORY:
		return 1;
	case DRIVER_DATA_NVEH_LANE_CHANGE:
		AdjVehiclesLaneChange[index1 + 2][index2 + 2] = long_value;
		return 1;
	case DRIVER_DATA_NO_OF_LANES:
		lanes_current_link = long_value;
		return 1;
	case DRIVER_DATA_LANE_WIDTH:
	case DRIVER_DATA_LANE_END_DISTANCE:
	case DRIVER_DATA_RADIUS:
	case DRIVER_DATA_MIN_RADIUS:
	case DRIVER_DATA_DIST_TO_MIN_RADIUS:
	case DRIVER_DATA_SLOPE:
	case DRIVER_DATA_SLOPE_AHEAD:
	case DRIVER_DATA_SIGNAL_DISTANCE:
	case DRIVER_DATA_SIGNAL_STATE:
	case DRIVER_DATA_SIGNAL_STATE_START:
	case DRIVER_DATA_SPEED_LIMIT_DISTANCE:
	case DRIVER_DATA_SPEED_LIMIT_VALUE:
		return 1;
	case DRIVER_DATA_DESIRED_ACCELERATION:
		desired_acceleration = double_value;
		return 1;
	case DRIVER_DATA_DESIRED_LANE_ANGLE:
		desired_lane_angle = double_value;
		return 1;
	case DRIVER_DATA_ACTIVE_LANE_CHANGE:
		active_lane_change = long_value;
		return 1;
	case DRIVER_DATA_REL_TARGET_LANE:
		rel_target_lane = long_value;
		return 1;
	default:
		return 0;
	}
}

/*--------------------------------------------------------------------------*/

DRIVERMODEL_API  int  DriverModelGetValue(long   type,
	long   index1,
	long   index2,
	long   *long_value,
	double *double_value,
	char   **string_value)
{
	/* Gets the value of a data object of type <type>, selected by <index1> */
	/* and possibly <index2>, and writes that value to <*double_value>,     */
	/* <*float_value> or <**string_value> (object and value selection       */
	/* depending on <type>).                                                */
	/* Return value is 1 on success, otherwise 0.                           */

	switch (type) {
	case DRIVER_DATA_STATUS:
		*long_value = 0;
		return 1;
	case DRIVER_DATA_VEH_TURNING_INDICATOR:
		*long_value = turning_indicator;
		return 1;
	case DRIVER_DATA_VEH_DESIRED_VELOCITY:
		*double_value = desired_velocity;
		return 1;
	case DRIVER_DATA_VEH_COLOR:
		*long_value = vehicle_color;
		return 1;
	case DRIVER_DATA_WANTS_SUGGESTION:  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		*long_value = 0;
		return 1;
	case DRIVER_DATA_DESIRED_ACCELERATION:
		*double_value = desired_acceleration;
		return 1;
	case DRIVER_DATA_DESIRED_LANE_ANGLE:
		*double_value = desired_lane_angle;
		return 1;
	case DRIVER_DATA_ACTIVE_LANE_CHANGE:
		*long_value = active_lane_change;
		return 1;
	case DRIVER_DATA_REL_TARGET_LANE:
		*long_value = rel_target_lane;
		return 1;
	case DRIVER_DATA_SIMPLE_LANECHANGE: //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
		*long_value = 0;
		return 1;
	default:
		return 0;
	}
}


DRIVERMODEL_API  int  DriverModelExecuteCommand(long number)
{
	/* Executes the command <number> if that is available in the driver */
	/* module. Return value is 1 on success, otherwise 0.               */

	switch (number) {
	case DRIVER_COMMAND_INIT:
		now = time(0);
		dt = ctime(&now);
		// the commented-out section below is for debugging purposes.
		//fout_ncacc.open("Debugging_101.csv", std::ios_base::out);
		//fout_ncacc << "EgoID" << "," << "Time" << "," << "LeadingID" << "," << "LeadingSpd" << "," << "D01" << ","<<"Lateral_Pos"<<endl;
		//fout_ncacc << "desired_lane_angle" << "," << "active_lane_change" << "," << "rel_target_lane" << ","<<"veh_rel_target_lane"<<","<< "veh_active_lane_change" << ","<<"lateral_pos"<<endl; //for lane changing test purpose
		//fout_ncacc << "EgoID" << "," << "Time" << "," << "Location" << "," << "Lane" << "," << "Head" << "," << "leaderID" << "," << "followerID" << "," << "front_Connect" << "," << "rear_Connect" << "," << "seq" << "," << "front_dist" << "," << "rear_dist" << endl;
		//fout_ncacc << "EgoID" << "," << "Time" << "," << "HeadID" << "," << "LeadingID" << "," << "LeadingSpd" << "," << "Front_Dist" << ","
		//	<< "EgoID" << "," << "EgoSpd" << "," << "EgoDesiredSpd" << "," << "LocationInPlatoon" << ","
		//	<< "FollowingID" << "," << "FollowingSpd" << "," << "Back_Dist" << "," << endl;
		//fout<<" --------------------- "<<dt;
		//fout.flush();
		
		fin.open("caccconf101.dat", std::ios_base::in);
		if (fin) {
			getline(fin, str);
			MaxPlatoonSize = atoi(str.c_str()); // vehicles
			getline(fin, str);
			HWShort = atof(str.c_str()); // m/s/s
			getline(fin, str);
			HWLong = atof(str.c_str()); // m/s/s
		}
		else
		{
			MaxPlatoonSize = 5;
			HWShort = 0.6;
			HWLong = 2.0;
		}
		fin.close();

		return 1;
	case DRIVER_COMMAND_CREATE_DRIVER:
		VehTargetLane[current_veh] = 0;
		platoonState[current_veh][0] = 0;
		return 1;
	case DRIVER_COMMAND_KILL_DRIVER:
		VehTargetLane.erase(current_veh);
		platoonState.erase(current_veh);
		return 1;
	case DRIVER_COMMAND_MOVE_DRIVER:
		/*  &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */
		
		//fout_ncacc.open("out_newdll_ncacc_" + to_string(current_veh) + ".log", std::ios_base::out);
		//fout_ncacc << "VISSIM SUGGESTION: " << vissim_suggestion<<","; // to check the previous status

		leaderID = AdjVehicles[2][3];
		followerID = AdjVehicles[2][1];
		front_dist = AdjVehiclesDist[2][3];
		rear_dist = -AdjVehiclesDist[2][1];

		VehTargetLane[current_veh] = 1;

		current_lane = lanes_current_link - current_lane + 1;
		lateral_pos_ind = GetLateralPos(lateral_pos);
		
		// Check wether the vehicle is trying to change the lane as they are not allowed until their type is changed.
		if (VehTargetLane[current_veh] == current_lane)
		{
			if (lateral_pos_ind != 0)
			{
				if (lateral_pos_ind>0)
				{
					rel_target_lane = -1;
					active_lane_change = -1;
					desired_lane_angle = active_lane_change * lane_angle;
				}
				else
				{
					rel_target_lane = 1;
					active_lane_change = 1;
					desired_lane_angle = active_lane_change * lane_angle;
				}
				return 1;
			}
			else
			{
				active_lane_change = 0;
				desired_lane_angle = 0.0;
				rel_target_lane = 0;
			}
		}

		
		/*  &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */
		
		
		ControlVehicle();
		
		return 1;
	default:
		return 0;
	}
}



int InitArrays()
{
	for (int i = 0; i <= 4; i++)
	{
		for (int j = 0; j <= 4; j++)
		{
			AdjVehicles[i][j] = 0;
			AdjVehiclesWidth[i][j] = 0;
			AdjVehiclesSpeed[i][j] = 0;
			AdjVehiclesDist[i][j] = 0;
			AdjVehiclesAcc[i][j] = 0;
			AdjVehiclesLaneChange[i][j] = 0;
			AdjVehiclesCurrentLane[i][j] = 0;
			AdjVehiclesTargetLane[i][j] = 0;
		}
	}

	return 0;

}

double GetLateralPos(double latpos)
{
	if (latpos>0.2 || latpos<-0.2)
		return latpos;
	else
		return 0.0;
}


int ControlVehicle()
{
	double temp_hw = 0;
	if (platoonState[current_veh][0] == 0)
	{
		if (rand()%100<20)
			desired_velocity = head_speed;
		else
			desired_velocity = head_speed + 5;
		platoonState[current_veh][0] = 1;
	}
	//It needs to tell if the vehicle in front is Connected or not. Since the API does not provide vehicle type information, vehicle
	//length is used an indicator, i.e. CACC vehicles are less than 3.8m long, while others are longer than 3.8m. The vehicle types in the 
	//VISSIM network need to be set-up accordingly.
	if (AdjVehiclesWidth[2][3]<3.8)
		//When front vehicle is connected, use HWShort, and front vehicle's acceleration is available (AdjVehiclesAcc[2][3])
		desired_acceleration = CACC_Car_Following(current_veh, Acc_vehicle, Spd_vehicle, Leng_vehicle,
		AdjVehicles[2][3], AdjVehiclesAcc[2][3], AdjVehiclesSpeed[2][3], AdjVehiclesWidth[2][3], AdjVehiclesDist[2][3],
		HWShort);
	else
		//When front vehicle is not connected, use HWLong, and front vehicle's acceleration is not available (assuming ZERO)
		desired_acceleration = CACC_Car_Following(current_veh, Acc_vehicle, Spd_vehicle, Leng_vehicle,
		AdjVehicles[2][3], 0 , AdjVehiclesSpeed[2][3], AdjVehiclesWidth[2][3], AdjVehiclesDist[2][3],
		HWLong);
	return 1;
}

double CACC_Car_Following(long lvid0, double a0, double v0, double leng0, long lvid1, double a1, double v1, double leng1, double d01,
	double t_system)
	//Implemented Bart van Arem's MIXIC model
{
	double a_ref = 0;
	double a_ref_v = 0;
	double a_ref_d = 0;
	double vint = desired_velocity;
	double ap = a1;
	double vp = v1;
	double r_ref = 0;
	double r_safe = 0;
	double r_system = 0;
	double r_min = 2; //in meters
	double r = d01 - (leng0 + leng1) / 2;
	double dp = -3; //deceleration capability of the leading vehicle
	double d = -3; //deceleration capability of the ego vehicle

	double k = 0.3;  //based on MIXIC studies
	double ka = 1.0; //
	double kv = 0.58;
	double kd = 0.1;
	double r_ref1 = 0.0;
	double r_ref2 = 0.0; 

	a_ref_v = k*(vint - v0);

	r_safe = v0*v0 / 2 * (1 / dp - 1 / d);
	r_system = t_system*v0;

	r_ref1 = max(r_safe, r_system);
	r_ref2 = max(r_safe, r_min);

	r_ref = max(r_ref1,r_ref2);
	a_ref_d = ka*ap + kv*(vp - v0) + kd*(r - r_ref);

	a_ref = min(a_ref_v, a_ref_d);
	if (lvid1 == -1)
		return a_ref_v;
	else
		return a_ref;
}