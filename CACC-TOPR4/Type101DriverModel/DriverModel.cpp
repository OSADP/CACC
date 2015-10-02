/*==========================================================================*/
/*  DriverModel.cpp                                  DLL Module for VISSIM  */
/*                                                                          */
/*  Interface module for external driver models.                            */
/*                                                                          */
/*  Version of 2015-10-02                                   Peng Patrick Su */
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
double HWLong = 2.0;
int leaderID = 0;
int followerID = 0;
double front_dist = 0.0;
double rear_dist = 0.0;
double const head_speed = 26.8;  // in meters per second
double const catchup_speed = 36;  // in meters per second
double const head_catchup_speed = 35;  // in meters per second
double const D_Thr = 100;
/*  &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */
//7.30.2015 platooning organization logic added 
map<long, int[6]> platoonState;
/* Terminology:
The one in front of ego is called leader, and the one behind is called follower, no matter they are connected or not.
The one in front of ego in a platoon is called predecessor, and the one behind in a platoon is called successor.
A leader is called predecessor if it is connected to Ego. 
A follower is called successorof if it is connected to the ego.
*/

/* platoonState dictionary
0: Head ID. 0 if it is not in platoon. This is the single identifier telling if a vehicle is in a platoon or not
1: Leader ID. 0 if it is leader (no predecessor) or not in platoon
2: Follower ID. 1 if it is the last car (no follower) or not in platoon
3: Connected to leader or not
4: Connected to follower or not
5: location. 1, 2, 3, 4, 5,...
To tell if a car is leader or not, check if platoonState[ID][0]==current_veh
To tell if a car is the tail or not, check if platoonState[ID][2]==0 and platoonState[ID][0]!=0
*/
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
		return 0; 
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
	case DRIVER_DATA_WANTS_SUGGESTION:
		*long_value = 1;
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
	case DRIVER_DATA_SIMPLE_LANECHANGE:
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
		return 1;
	case DRIVER_COMMAND_KILL_DRIVER:
		VehTargetLane.erase(current_veh);
		platoonState.erase(current_veh);
		return 1;
	case DRIVER_COMMAND_MOVE_DRIVER:
		/*  &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */
	
		leaderID = AdjVehicles[2][3];
		followerID = AdjVehicles[2][1];
		platoonState[current_veh][1] = leaderID;
		platoonState[current_veh][2] = followerID;
		front_dist = AdjVehiclesDist[2][3];
		rear_dist = -AdjVehiclesDist[2][1];


		// Added by JL from here--------------------

		VehTargetLane[current_veh] = 1;

		if (current_link == 19 || current_link == 1636)
			VehTargetLane[current_veh] = 2;

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
		// ------- to here. 


		if (platoonState[current_veh][0] == 0) //ego vehicle is not in platoon yet
		{
			if (leaderID == -1 || platoonState[leaderID][0] == 0) //ego car has no leader or the leader is not in platoon, and it can be head itself
			{
				desired_velocity = head_speed;
				platoonState[current_veh][0] = current_veh;
				platoonState[current_veh][3] = 0;
				platoonState[current_veh][4] = 0;
				platoonState[current_veh][5] = 1;
			}
			else
			{
				if (platoonState[leaderID][5] < MaxPlatoonSize) // if the leader's platoon has not reached size limit
				{
					if (front_dist > D_Thr)
					{
						desired_velocity = head_speed;
						platoonState[current_veh][0] = current_veh;
						platoonState[current_veh][3] = 0;
						platoonState[current_veh][5] = 1;
					}
					else
					{
						desired_velocity = catchup_speed;
						platoonState[current_veh][0] = platoonState[leaderID][0];
						platoonState[current_veh][3] = 1;
						platoonState[current_veh][5] = platoonState[leaderID][5] + 1;
						//if (platoonState[current_veh][5] > MaxPlatoonSize)
							//fout_ncacc << "MAx size exceeded!" << "," << "not in platoon" << endl;
					}
				}
				else //leader's platoon has reached size limit. Ego can be head itself
				{
					desired_velocity = head_speed;
					platoonState[current_veh][0] = current_veh;
					platoonState[current_veh][3] = 0;
					platoonState[current_veh][5] = 1;
				}
			}
		}
		/*  &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */
		else
		{
			if (platoonState[current_veh][0] == current_veh) // it is head. Does not have a whole platoon to join a new head yet. 7.31.2015. This might change.
			{
				desired_velocity = head_speed;
				if (rear_dist > D_Thr)
				{
					platoonState[current_veh][3] = 0;
					platoonState[current_veh][4] = 0;
					platoonState[current_veh][5] = 1;
					platoonState[followerID][0] = followerID;
					platoonState[followerID][3] = 0;
					platoonState[followerID][5] = 1;
				}
				else
				{
					platoonState[current_veh][3] = 0;
					platoonState[current_veh][4] = 1;
					platoonState[current_veh][5] = 1;
					platoonState[followerID][0] = current_veh;
					platoonState[followerID][3] = 1;
					platoonState[followerID][5] = 2;
				}
			}
			else //current_veh is not head
			{
				if (platoonState[current_veh][5] > MaxPlatoonSize) // if MaxPlatoonSize has been exceeded, split the platoon. 
				{
					desired_velocity = head_speed;
					platoonState[leaderID][4] = 0;
					if (rear_dist > D_Thr)
					{
						platoonState[current_veh][0] = current_veh;
						platoonState[current_veh][3] = 0;
						platoonState[current_veh][4] = 0;
						platoonState[current_veh][5] = 1;
						platoonState[followerID][0] = followerID;
						platoonState[followerID][3] = 0;
						platoonState[followerID][5] = 1;
					}
					else
					{
						platoonState[current_veh][0] = current_veh;
						platoonState[current_veh][3] = 0;
						platoonState[current_veh][4] = 1;
						platoonState[current_veh][5] = 1;
						platoonState[followerID][0] = current_veh;
						platoonState[followerID][3] = 1;
						platoonState[followerID][5] = 2;
					}
				}
				else
				if (platoonState[current_veh][5] == MaxPlatoonSize) //if MaxPlatoonSize has been reached, set the following one as new head
				{
					if (front_dist > D_Thr)
					{
						platoonState[leaderID][4] = 0;
						desired_velocity = head_speed;
						platoonState[current_veh][0] = current_veh;
						platoonState[current_veh][3] = 0;
						platoonState[current_veh][5] = 1;
						if (rear_dist > D_Thr)
						{
							platoonState[followerID][0] = followerID;
							platoonState[followerID][3] = 0;
							platoonState[followerID][5] = 1;
						}
						else
						{
							platoonState[current_veh][4] = 1;
							platoonState[followerID][0] = current_veh;
							platoonState[followerID][3] = 1;
							platoonState[followerID][5] = 2;
						}
					}
					else
					{
						desired_velocity = catchup_speed;
						platoonState[current_veh][0] = platoonState[leaderID][0];
						platoonState[current_veh][3] = 1;
						//platoonState[current_veh][5] = platoonState[leaderID][5]+1;

						platoonState[followerID][0] = followerID;
						platoonState[followerID][3] = 0;
						platoonState[followerID][5] = 1;
					}
				}
				else // MaxPlatoonSize has not been reached, attached more vehicles to the platoon
				{
					desired_velocity = catchup_speed;
					platoonState[leaderID][4] = 1;
					platoonState[current_veh][0] = platoonState[leaderID][0];
					platoonState[current_veh][3] = 1;
					platoonState[current_veh][5] = platoonState[leaderID][5] + 1;
					if (rear_dist > D_Thr)
					{
						platoonState[current_veh][4] = 0;
						platoonState[followerID][0] = followerID;
						platoonState[followerID][3] = 0;
						platoonState[followerID][5] = 1;
					}
					else
					{
						if (platoonState[current_veh][5] < MaxPlatoonSize) //connect ego to follower
						{
							platoonState[current_veh][4] = 1;
							platoonState[followerID][0] = platoonState[current_veh][0];
							platoonState[followerID][3] = 1;
							platoonState[followerID][5] = platoonState[current_veh][5] + 1;
						}
						else  //make follower a new Head
						{
							platoonState[current_veh][4] = 0;
							platoonState[followerID][0] = followerID;
							platoonState[followerID][3] = 0;
							platoonState[followerID][5] = 1;
						}
					}
				}
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
	if (platoonState[current_veh][0] == current_veh)
		temp_hw = HWLong; // 
	else
		temp_hw = HWShort; // headway for inter-CACC 
	desired_acceleration = CACC_Car_Following(current_veh, Acc_vehicle, Spd_vehicle, Leng_vehicle,
		AdjVehicles[2][3], AdjVehiclesAcc[2][3], AdjVehiclesSpeed[2][3], AdjVehiclesWidth[2][3], AdjVehiclesDist[2][3],
		temp_hw);

	return 1;
}

double CACC_Car_Following(long lvid0, double a0, double v0, double leng0, long lvid1, double a1, double v1, double leng1, double d01,
	double t_system)
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

	r_safe = v0*v0 / 2 * (1 / dp - 1 / d); //currently not used, since dp and d are the same for now
	r_system = t_system*v0;

	//r_ref = max(r_safe, r_system, r_min); // Peng: Check the input variables whether they are correcr or not. 
	r_ref1 = max(r_safe, r_system);
	r_ref2 = max(r_safe, r_min);

	r_ref = max(r_ref1,r_ref2); // Peng: Check the input variables whether they are correcr or not. 
	a_ref_d = ka*ap + kv*(vp - v0) + kd*(r - r_ref);

	a_ref = min(a_ref_v, a_ref_d);
	if (lvid1 == -1)
		return a_ref_v;
	else
		return a_ref;
}


