/*==========================================================================*/
/*  DriverModel.cpp                                  DLL Module for VISSIM  */
/*                                                                          */
/*  Interface module for external driver models.                            */
/*  Dummy version that simply sends back VISSIM's suggestions to VISSIM.    */
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
double	desired_lane_angle_tmp = 0.0;
long    active_lane_change = 0;
long    rel_target_lane = 0;
long	veh_rel_target_lane = 0;
double  desired_velocity = 0.0;
long    turning_indicator = 0;
long    vehicle_color = RGB(0, 0, 0);

long veh_type = 104;
long current_link = 0;
long current_lane = 0;
long lanes_current_link = 0; // Dec. 15. 2014
double timestep = 0.0;
long current_veh = 0;
long vissim_suggestion = 1; // 0 indicates no: not listen to VISSIM, 1 otherwise
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
double val = 0.0;

map<long, int> VehStatus; // 0: false, >1: true
map<long, int> VehTargetLane;
map<long, int> LeadingVehs;
map<long, int> VISSIMSUG;



long LeadingVeh_ID = 0;
double LeadingVeh_Spd = 0.0;
double LeadingVeh_Acc = 0.0;
double DistLeadingVeh = 0.0;
double DistTargetLeadingVeh = 0.0;

long FollowingVeh_ID = 0;
double FollowingVeh_Spd = 0.0;
double FollowingVeh_Acc = 0.0;
double DistFollowingVeh = 0.0;
double DistTargetFollowingVeh = 0.0;

double sim_time = 0.0;
double Spd_vehicle = 0.0;
double Len_vehicle = 0.0;

double lane_angle = 0.021;
double lateral_pos = 0.0;
double lateral_pos_ind = 0.0;
double veh_od = 0.0;
bool WrtFlag = false;
int tlane = 0;
double leadingCriticalGap = 0.0;
double laggingCriticalGap = 0.0;
double leadingMaxSpdDiff = 0.0;
double laggingMaxSpdDiff = 0.0;
int Toggle = 0;



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
		return 1;
	case DRIVER_DATA_VEH_LENGTH:
		Len_vehicle = double_value;
		return 1;
	case DRIVER_DATA_VEH_WIDTH:
	case DRIVER_DATA_VEH_WEIGHT:
		return 1;
	case DRIVER_DATA_VEH_MAX_ACCELERATION:
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
		veh_type = long_value;
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
		// AdjVehiclesWidth[index1+2][index2+2] = double_value; 
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
		desired_lane_angle_tmp = double_value;
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
		//*long_value = 0;
		*long_value = vissim_suggestion;
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

		//fout.open("out_newdll.dat", std::ios_base::out);

		fin.open("caccconf104.dat", std::ios_base::in);
		if (fin)
		{
			getline(fin, str);
			leadingCriticalGap = atof(str.c_str()); // second
			getline(fin, str);
			laggingCriticalGap = atof(str.c_str()); // second
			getline(fin, str);
			leadingMaxSpdDiff = atof(str.c_str()); // m/s
			getline(fin, str);
			laggingMaxSpdDiff = atof(str.c_str()); // m/s
			getline(fin, str);
			Toggle = atoi(str.c_str()); // 0 by Critical Gap; 1 by Speed Differential and 2 by both
		}
		else
		{
			// No configuration data exists. Use defaults
			leadingCriticalGap = 0.6;
			laggingCriticalGap = 0.8;
			leadingMaxSpdDiff = 5 * 0.447; // MPH --> m/sK
			laggingMaxSpdDiff = 8 * 0.447;
			Toggle = 2; //// 0 by Critical Gap; 1 by Speed Differential and 2 by both
		}
		fin.close();

		return 1;
	case DRIVER_COMMAND_CREATE_DRIVER:
		VehStatus[current_veh] = 0;
		VehTargetLane[current_veh] = 0;
		LeadingVehs[current_veh] = 0;
		VISSIMSUG[current_veh] = 1; // follow VISSIM until it starts to change the lane
		return 1;
	case DRIVER_COMMAND_KILL_DRIVER:
		VehStatus.erase(current_veh);
		VehTargetLane.erase(current_veh);
		LeadingVehs.erase(current_veh);
		VISSIMSUG.erase(current_veh);

		return 1;
	case DRIVER_COMMAND_MOVE_DRIVER:

		vissim_suggestion = VISSIMSUG[current_veh];
		current_lane = lanes_current_link - current_lane + 1;

		VehTargetLane[current_veh] = 1;

		lateral_pos_ind = GetLateralPos(lateral_pos);

		if (VehStatus[current_veh] == 0 && lateral_pos_ind != 0)
		{

			if (lateral_pos_ind>0) // To the right
			{

				rel_target_lane = -1;
				active_lane_change = -1;
				desired_lane_angle = active_lane_change * lane_angle;
			}
			else // To the left
			{

				rel_target_lane = 1;
				active_lane_change = 1;
				desired_lane_angle = active_lane_change * lane_angle;
			}
			return 1;
		}

		if (current_lane == 2)
		{
			int flag = 1; // false 
			double gap1 = 0.0;
			double gap2 = 0.0;
			double diff1 = 0.0;
			double diff2 = 0.0;

			DistLeadingVeh = AdjVehiclesDist[3][3];
			LeadingVeh_Acc = AdjVehiclesAcc[3][3];
			LeadingVeh_Spd = AdjVehiclesSpeed[3][3] + 0.00001; // to make non-zero denominator
			LeadingVeh_ID = AdjVehicles[3][3];


			DistFollowingVeh = AdjVehiclesDist[3][1];
			FollowingVeh_Acc = AdjVehiclesAcc[3][1];
			FollowingVeh_Spd = AdjVehiclesSpeed[3][1]+0.00001; // to make non-zero denominator
			FollowingVeh_ID = AdjVehicles[3][1];

			Spd_vehicle = Spd_vehicle + 0.00001; // to make non-zero denominator

			if (LeadingVeh_ID == -1 && FollowingVeh_ID == -1)//No CACC cars on the target lane. Just get in immediately. 
			{

				flag = 0;
			}
			else // Otherwise, check the condition
			{
				if (LeadingVeh_ID == -1 && FollowingVeh_ID > 0)
				{
					gap2 = (-1.0*DistFollowingVeh) / FollowingVeh_Spd;
					diff2 = FollowingVeh_Spd - Spd_vehicle;
					if (gap2 > 3)
						flag = 0;
					else
					if (gap2>laggingCriticalGap && diff2<laggingMaxSpdDiff)
						flag = 0;
				}
				else if (LeadingVeh_ID > 0 && FollowingVeh_ID == -1)
				{
					gap1 = DistLeadingVeh / Spd_vehicle;
					diff1 = Spd_vehicle - LeadingVeh_Spd;
					if (gap1 > 1)
						flag = 0;
					else
					if (gap1 > leadingCriticalGap && diff1 < leadingMaxSpdDiff)
						flag = 0;
				}
				else if (LeadingVeh_ID > 0 && FollowingVeh_ID > 0)
				{
					gap1 = DistLeadingVeh / Spd_vehicle;
					gap2 = (-1.0*DistFollowingVeh) / FollowingVeh_Spd;
					diff1 = Spd_vehicle - LeadingVeh_Spd;
					diff2 = FollowingVeh_Spd - Spd_vehicle;
					if (gap1 > 1 && gap2 > 3)
						flag = 0;
					else
					if (gap1 > leadingCriticalGap && gap2 > laggingCriticalGap && diff1<leadingMaxSpdDiff && diff2<laggingMaxSpdDiff)
						flag = 0;
				}
				else
					flag = 1;
			}

			if (Spd_vehicle < 50 / 3.6)
				flag = 1; // not allow to change lane if the speed is too slow...

			if (flag == 0)
			{
				active_lane_change = 1;
				rel_target_lane = 1;
				desired_lane_angle = active_lane_change * lane_angle;
				VehStatus[current_veh] = -1;
				VISSIMSUG[current_veh] = 0;
			}
		}
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


/*==========================================================================*/
/*  Ende of DriverModel.cpp                                                 */
/*==========================================================================*/
