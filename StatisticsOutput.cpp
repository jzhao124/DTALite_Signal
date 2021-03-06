//  Portions Copyright 2010 Xuesong Zhou

//   If you help write or modify the code, please also list your names here.
//   The reason of having copyright info here is to ensure all the modified version, as a whole, under the GPL 
//   and further prevent a violation of the GPL.

// More about "How to use GNU licenses for your own software"
// http://www.gnu.org/licenses/gpl-howto.html

//    This file is part of DTALite.

//    DTALite is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    DTALite is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with DTALite.  If not, see <http://www.gnu.org/licenses/>.

// DTALite.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "DTALite.h"
#include "GlobalData.h"
#include <iostream>
#include <fstream>
#include <omp.h>
#include <algorithm>

extern int g_SimulationDuration;
using namespace std;
#define MAX_LOS_SIZE 8
enum Link_MOE {MOE_none,MOE_volume, MOE_speed, MOE_queue_length, MOE_safety,MOE_user_defined,MOE_density,MOE_traveltime,MOE_capacity, MOE_speedlimit, MOE_reliability, MOE_fftt, MOE_length, MOE_queuelength,MOE_fuel,MOE_vehicle, MOE_volume_copy, MOE_speed_copy, MOE_density_copy, MOE_emissions, MOE_ECO2,MOE_NOX, MOE_CO,MOE_HC};


std::string g_CreateFileName(std::string common_file_name, bool Day2DayOutputFlag = false, int iteration = 0, bool with_CSV = true)
{
	std::string file_name = common_file_name;

	if (Day2DayOutputFlag == true)
	{
		CString day_str;
		day_str.Format("_day_%02d", iteration);

		file_name += CString2StdString(day_str);
	}
	if (with_CSV == true)
	{
	
		file_name += ".csv";

	}
	return file_name;
}

void g_OutputMOEData(int iteration, bool Day2DayOutputFlag)
{	
	if(g_EmissionDataOutputFlag == 1)
	{
		cout << "     outputing emssions data... " << endl;
		OutputEmissionData();
	}

	ofstream output_NetworkTDMOE_file;
	cout << "     outputing output_NetworkTDMOE.csv... " << endl;

	output_NetworkTDMOE_file.open(g_CreateFileName("output_NetworkTDMOE", Day2DayOutputFlag, iteration));
	//	output_ODImpact_file.open ("output_ImpactedOD.csv");
	if(output_NetworkTDMOE_file.is_open ())
	{
		OutputNetworkMOEData(output_NetworkTDMOE_file);
		output_NetworkTDMOE_file.close();
	}else
	{
		cout << "Error: File output_NetworkTDMOE.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();	
	}

	bool bStartWithEmptyFile = true;
	cout << "     outputing output_agent.csv... " << endl;
	OutputVehicleTrajectoryData( g_CreateFileName("output_agent", Day2DayOutputFlag, iteration+1), 
		g_CreateFileName("output_trip", Day2DayOutputFlag, iteration+1), iteration+1, true, false);
		

	ofstream output_ODMOE_file;

	cout << "     outputing output_ODMOE.csv... " << endl;
	output_ODMOE_file.open(g_CreateFileName("output_ODMOE", Day2DayOutputFlag, iteration, true));
	//	output_ODImpact_file.open ("output_ImpactedOD.csv");
	if(output_ODMOE_file.is_open ())
	{
		OutputODMOEData(output_ODMOE_file,1,0);
		output_ODMOE_file.close();
	}else
	{
		cout << "Error: File output_ODMOE.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();	
	}

	ofstream output_MovementMOE_file;
	cout << "     outputing output_MovementMOE.csv... " << endl;
	output_MovementMOE_file.open(g_CreateFileName("output_MovementMOE", Day2DayOutputFlag, iteration), fstream::app);
	//	output_ODImpact_file.open ("output_ImpactedOD.csv");
	if (output_MovementMOE_file.is_open ())
	{
		OutputMovementMOEData(output_MovementMOE_file, g_SimulationDuration);
	} else
	{
		cout << "Error: File output_MovementMOE.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();	
	}


	cout << "     outputing output_LinkTDMOE.csv... " << endl;
	OutputLinkMOEData(g_CreateFileName("output_LinkTDMOE", Day2DayOutputFlag, iteration), iteration, bStartWithEmptyFile);

	CString GISTypeString;
		int ColorCode = 1;
		bool no_curve_flag = false;
		float height_ratio = 0.1;
		g_ExportLink3DLayerToKMLFiles("output_simulationdata.kml", GISTypeString, ColorCode, no_curve_flag, height_ratio);


	cout << "     MOE output complete... " << endl;

	int TimeDependentRoutingPolicyEOutputFlag = g_GetPrivateProfileInt("output", "time_dependent_RoutingPolicy_file", 0, g_DTASettingFileName);

	if(TimeDependentRoutingPolicyEOutputFlag)
	{
	for (int departure_time_index = 0; departure_time_index < g_NumberOfSPCalculationPeriods; departure_time_index++)
		{

			int departure_time = g_AssignmentIntervalStartTimeInMin[departure_time_index];
			int  departure_end_time = g_AssignmentIntervalEndTimeInMin[departure_time_index];

			int hour = departure_time / 60;
			int min = (departure_time - hour * 60);

	CString str;
	str.Format("output_routing_policy_%02dh%02dm00s.csv", hour,min);

	ofstream output_routing_policy_file;

	output_routing_policy_file.open (str);
	//	output_ODImpact_file.open ("output_ImpactedOD.csv");
	if(output_routing_policy_file.is_open ())
	{
		cout << "     outputing " << str << " ..." << endl;

		g_OutputTimeDependentRoutingPolicyData(output_routing_policy_file, departure_time, departure_end_time, g_output_OD_path_MOE_cutoff_volume);
		output_routing_policy_file.close();
	}else
	{
		cout << "Error: File " <<  str << " cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();	
	}
	}


	// global routing policy

	{
	CString str;
	str.Format("output_routing_policy.csv");

	ofstream output_routing_policy_file;

	output_routing_policy_file.open(str);
	//	output_ODImpact_file.open ("output_ImpactedOD.csv");
	if (output_routing_policy_file.is_open())
	{

		output_routing_policy_file << "from_zone_id,to_zone_id,demand_type,information_type,departure_start_time_in_min,departure_end_time_in_min,path_no,number_of_vehicles,ratio,node_sequence,link_sequence,node_sum_reference,distance_reference,travel_time_reference" << endl;

		for (int departure_time_index = 0; departure_time_index < g_NumberOfSPCalculationPeriods; departure_time_index++)
		{

			int departure_time = g_AssignmentIntervalStartTimeInMin[departure_time_index];
			int  departure_end_time = g_AssignmentIntervalEndTimeInMin[departure_time_index];

			int hour = departure_time / 60;
			int min = (departure_time - hour * 60);
			g_OutputRoutingPolicyData(output_routing_policy_file, departure_time, departure_end_time, g_output_OD_path_MOE_cutoff_volume);

		}
		output_routing_policy_file.close();
	}
	}

	// global link path data

	{
		CString str;
		str.Format("output_path_link_mapping.csv");

		ofstream output_routing_policy_file;

		output_routing_policy_file.open(str);
		//	output_ODImpact_file.open ("output_ImpactedOD.csv");
		if (output_routing_policy_file.is_open())
		{
			output_routing_policy_file << "from_zone_id,to_zone_id,departure_start_time_in_interval,path_no,link_id,ratio" << endl;

			for (int departure_time_index = 0; departure_time_index < g_NumberOfSPCalculationPeriods; departure_time_index++)
			{

				int departure_time = g_AssignmentIntervalStartTimeInMin[departure_time_index];
				int  departure_end_time = g_AssignmentIntervalEndTimeInMin[departure_time_index];

				int hour = departure_time / 60;
				int min = (departure_time - hour * 60);
				g_OutputLinkPathProportionData(output_routing_policy_file, departure_time, departure_end_time, g_output_OD_path_MOE_cutoff_volume);

			}
			output_routing_policy_file.close();
		}
	}

	{
		CString str;
		str.Format("output_link_proportion.csv");

		ofstream output_routing_policy_file;

		output_routing_policy_file.open(str);
		//	output_ODImpact_file.open ("output_ImpactedOD.csv");
		if (output_routing_policy_file.is_open())
		{
			output_routing_policy_file << "from_zone_id,to_zone_id,departure_start_time_in_interval,link_id,ratio" << endl;

			for (int departure_time_index = 0; departure_time_index < g_NumberOfSPCalculationPeriods; departure_time_index++)
			{

				int departure_time = g_AssignmentIntervalStartTimeInMin[departure_time_index];
				int  departure_end_time = g_AssignmentIntervalEndTimeInMin[departure_time_index];

				int hour = departure_time / 60;
				int min = (departure_time - hour * 60);
				g_OutputLinkProportionData(output_routing_policy_file, departure_time, departure_end_time, g_output_OD_path_MOE_cutoff_volume);

			}
			output_routing_policy_file.close();
		}
	}

	}
	
	g_TimeDependentODMOEOutputFlag = g_GetPrivateProfileInt("output", "time_dependent_ODMOE_file", 0, g_DTASettingFileName);
	g_AggregationTimetInterval = g_GetPrivateProfileInt("output", "time_dependent_ODMOE_aggregation_time_interval", 30, g_DTASettingFileName);

	if (g_TimeDependentODMOEOutputFlag == 1)
	{
		ofstream output_ODTDMOE_file;

		std::string file_name = g_CreateFileName("output_ODTDMOE", Day2DayOutputFlag, iteration, false);
		output_ODTDMOE_file.open(file_name);
		//	output_ODImpact_file.open ("output_ImpactedOD.csv");
		int department_time_interval = max(5, g_GetPrivateProfileInt("output", "time_dependent_ODMOE_aggregation_time_interval", 30, g_DTASettingFileName));
		if (output_ODTDMOE_file.is_open())
		{
			cout << "     outputing output_ODTDMOE.csv... " << endl;

			output_ODTDMOE_file.close();
		}
		else
		{
			cout << "Error: File output_ODTDMOE.csv cannot be opened.\n It might be currently used and locked by EXCEL." << endl;
			g_ProgramStop();
		}
	
		g_SkimMatrixGenerationForAllDemandTypes(file_name, true, 0);

	}


}
void OutputMovementMOEData(ofstream &output_MovementMOE_file, int g_SimulationDuration)
{

	output_MovementMOE_file << "node_id,incoming_link_from_node_id,outgoing_link_to_node_id,turning_direction,movement_hourly_capacity,total_vehicle_count,avg_vehicle_delay_in_sec "<< endl;
	double hours = max(g_SimulationDuration / 60.0, 0.01);
	for(int i = 0; i < g_NodeVector.size(); i++)
	{
		for (std::map<string, DTANodeMovement>::iterator iter = g_NodeVector[i].m_MovementMap.begin(); 
			iter != g_NodeVector[i].m_MovementMap.end(); iter++)
		{
			float avg_delay = max(0, iter->second.GetAvgDelay_In_Min ()*60); // *60 to convert min to seconds
			output_MovementMOE_file << iter->second.in_link_to_node_id << "," << iter->second.in_link_from_node_id <<"," << iter->second.out_link_to_node_id << "," << iter->second.turning_direction << ","<< iter->second.movement_hourly_capacity << ",";

			output_MovementMOE_file << iter->second.total_vehicle_count / (hours * 0.8) << "," << avg_delay << endl;
		}

		if (i % 100 == 0) {
			cout << " outputing " << i << " movement MOE... " << endl;
		}
	}

}

void OutputODMOEData(ofstream &output_ODMOE_file,int cut_off_volume, int arrival_time_window_begin_time_in_min)
{

	ODStatistics** ODMOEArray = NULL;

	int total_number_of_zones = g_ZoneMap.size();

	ODMOEArray = AllocateDynamicArray<ODStatistics>(total_number_of_zones,total_number_of_zones);

	int i,j;
	for(i = 0; i< total_number_of_zones; i++)
		for(j = 0; j< total_number_of_zones; j++)
		{

					ODMOEArray[i][j].OriginZoneNumber  = 0;
					ODMOEArray[i][j].DestinationZoneNumber  = 0; 
					ODMOEArray[i][j].TotalVehicleSize = 0;
					ODMOEArray[i][j].TotalCompleteVehicleSize = 0;
					ODMOEArray[i][j].TotalTravelTime = 0;
					ODMOEArray[i][j].TotalDistance =0;
			
		}

	std::map<int, DTAVehicle*>::iterator iterVM;
	for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
	{

		DTAVehicle* pVehicle = iterVM->second;

		int origin_zone_no = g_ZoneMap[pVehicle->m_OriginZoneID].m_ZoneSequentialNo;
		int destination_zone_no = g_ZoneMap[pVehicle->m_DestinationZoneID].m_ZoneSequentialNo;

		ODMOEArray[origin_zone_no][destination_zone_no].TotalVehicleSize+=1;
		if(pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete && pVehicle-> m_ArrivalTime >=   arrival_time_window_begin_time_in_min)  // with physical path in the network
		{


			ODMOEArray[origin_zone_no][destination_zone_no].OriginZoneNumber = pVehicle->m_OriginZoneID;
			ODMOEArray[origin_zone_no][destination_zone_no].DestinationZoneNumber = pVehicle->m_DestinationZoneID;


			ODMOEArray[origin_zone_no][destination_zone_no].TotalCompleteVehicleSize+=1;
			ODMOEArray[origin_zone_no][destination_zone_no].TotalTravelTime += pVehicle->m_TripTime;
			ODMOEArray[origin_zone_no][destination_zone_no].TotalDistance += pVehicle->m_Distance;
		}
	}


	output_ODMOE_file << "from_zone_id,to_zone_id,number_of_agents,number_of_agents_completing_trips,trip_time_in_min,distance" << endl;

	for(i = 0; i< total_number_of_zones; i++)
		for(j = 0; j< total_number_of_zones; j++)
		{

			if(ODMOEArray[i][j].TotalCompleteVehicleSize>=cut_off_volume)
			{
				output_ODMOE_file << 
					ODMOEArray[i][j].OriginZoneNumber  << "," << 
					ODMOEArray[i][j].DestinationZoneNumber  << "," << 
					ODMOEArray[i][j].TotalVehicleSize <<"," <<
					ODMOEArray[i][j].TotalCompleteVehicleSize <<"," <<
					ODMOEArray[i][j].TotalTravelTime/ max(1,ODMOEArray[i][j].TotalCompleteVehicleSize) << "," <<
					ODMOEArray[i][j].TotalDistance /  max(1,ODMOEArray[i][j].TotalCompleteVehicleSize) << endl;
			}
		}
		if(ODMOEArray !=NULL)
			DeallocateDynamicArray<ODStatistics>(ODMOEArray,total_number_of_zones,total_number_of_zones);

}

void OutputTimeDependentODMOEData(ofstream &output_ODMOE_file, int department_time_intreval,int end_of_period,int cut_off_volume)
{
	
	// step 1: collect statistics based on vehicle file


	output_ODMOE_file << "from_zone_id,to_zone_id,demand_type,departure_time,number_of_agents,travel_time_in_min,distance" << endl;

	ODStatistics*** ODMOE3dArray = NULL;

	int StatisticsIntervalSize = (g_DemandLoadingEndTimeInMin - g_DemandLoadingStartTimeInMin)/department_time_intreval;

	cout << "allocating memory for time-dependent ODMOE data...for " << g_ODZoneIDSize << " X " <<  g_ODZoneIDSize << "zones for " << StatisticsIntervalSize << " 15-min time intervals" << endl;
	ODMOE3dArray = Allocate3DDynamicArray<ODStatistics>(g_ODZoneIDSize + 1, g_ODZoneIDSize + 1, StatisticsIntervalSize);
	cout << "end of allocating memory for time-dependent ODMOE data."<< endl;

	for (int demand_type = 1; demand_type <= g_DemandTypeVector.size(); demand_type++)
	{


		for (int o = 0; o < g_ODZoneIDSize; o++)
		for (int d = 0; d < g_ODZoneIDSize; d++)
			for(int StatisticsInterval = 0; StatisticsInterval < StatisticsIntervalSize; StatisticsInterval++)
		{
		
			ODMOE3dArray[o][d][StatisticsInterval].TotalVehicleSize = 0;
			ODMOE3dArray[o][d][StatisticsInterval].TotalTravelTime = 0;
			ODMOE3dArray[o][d][StatisticsInterval].TotalDistance = 0;
		}

	std::map<int, DTAVehicle*>::iterator iterVM;
	for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
	{

	DTAVehicle* pVehicle = iterVM->second;

	if(pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete)  // with physical path in the network
	{
		if (pVehicle->m_DemandType != demand_type)
			continue;

		int StatisticsInterval = (pVehicle->m_DepartureTime - g_DemandLoadingStartTimeInMin)/department_time_intreval;

	if(StatisticsInterval >= StatisticsIntervalSize)  // avoid out of bound errors
	StatisticsInterval = StatisticsIntervalSize-1;

	ASSERT(pVehicle->m_OriginZoneID <= g_ODZoneNumberSize);
	ASSERT(pVehicle->m_DestinationZoneID <= g_ODZoneNumberSize);

	int o_no = g_ZoneNumber2NoVector[pVehicle->m_OriginZoneID];
	int d_no = g_ZoneNumber2NoVector[pVehicle->m_DestinationZoneID];

	ODMOE3dArray[o_no][d_no][StatisticsInterval].TotalVehicleSize+=1;
	ODMOE3dArray[o_no][d_no][StatisticsInterval].TotalTravelTime += pVehicle->m_TravelTime ;
	ODMOE3dArray[o_no][d_no][StatisticsInterval].TotalDistance += pVehicle->m_Distance;

	}
	}
	

	cout << "output od-based MOE statistics..." << endl;

	// step 2: output od-based MOE statistics
	for(int i = 0; i< g_ODZoneIDSize; i++)
	for (int j = 0; j <  g_ODZoneIDSize; j++)
	for(int t=0; t<StatisticsIntervalSize; t++)
	{
		int number_of_vehicles = ODMOE3dArray[i][j][t].TotalVehicleSize;

		if(ODMOE3dArray[i][j][t].TotalVehicleSize >=1)
		{
			float avg_travel_time = ODMOE3dArray[i][j][t].TotalTravelTime / max(1, number_of_vehicles);
			float avg_travel_distance = ODMOE3dArray[i][j][t].TotalDistance / max(1, number_of_vehicles);

		output_ODMOE_file << g_ZoneNo2NumberVector[i] << 
			"," << g_ZoneNo2NumberVector[j] << "," << demand_type << "," <<
		g_DemandLoadingStartTimeInMin + t*+ department_time_intreval << "," << 
		number_of_vehicles << ","
		<<  avg_travel_time  << "," 
			<< avg_travel_distance << "," << endl;
		}
	}
	}

	if(ODMOE3dArray !=NULL)
		Deallocate3DDynamicArray<ODStatistics>(ODMOE3dArray, g_ODZoneIDSize + 1, g_ODZoneIDSize + 1);


}


void g_OutputTimeDependentRoutingPolicyData(ofstream &output_PathMOE_file, int DemandLoadingStartTimeInMin, int DemandLoadingEndTimeInMin,
	int cut_off_volume)
{

	output_PathMOE_file << "from_zone_id,to_zone_id,demand_type,information_type,departure_start_time_in_min,departure_end_time_in_min,path_no,ratio,node_sequence,link_sequence,node_sum_reference,distance_reference,travel_time_reference" << endl;

	ODPathSet** ODPathSetVector = NULL;

	ODPathSetVector = AllocateDynamicArray<ODPathSet>(g_ODZoneIDSize+1,g_ODZoneIDSize+1);

	int total_demand_type = g_DemandTypeVector.size();

	total_demand_type = 1; // demand type 1 only for simplity

	for (int demand_type = 1; demand_type <= total_demand_type; demand_type++)
	{

		for (int i = 0; i < g_ODZoneIDSize; i++)
		for (int j = 0; j < g_ODZoneIDSize; j++)
		{
			ODPathSetVector[i][j].PathSet.clear();

		}

		std::map<int, DTAVehicle*>::iterator iterVM;
		for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{

			DTAVehicle* pVehicle = iterVM->second;

			if (pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete
				&& pVehicle->m_DepartureTime >= DemandLoadingStartTimeInMin
				&& pVehicle->m_DepartureTime < DemandLoadingEndTimeInMin)
			{
				ODPathSetVector[g_ZoneMap[pVehicle->m_OriginZoneID].m_ZoneSequentialNo][g_ZoneMap[pVehicle->m_DestinationZoneID].m_ZoneSequentialNo].TotalVehicleSize += 1;
			}


		}


		for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{

			DTAVehicle* pVehicle = iterVM->second;

			int OrgZoneSequentialNo = g_ZoneMap[pVehicle->m_OriginZoneID].m_ZoneSequentialNo;
			int DestZoneSequentialNo = g_ZoneMap[pVehicle->m_DestinationZoneID].m_ZoneSequentialNo;

			if (pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete && ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].TotalVehicleSize >= cut_off_volume
				&& pVehicle->m_DemandType == demand_type
				&& pVehicle->m_DepartureTime >= DemandLoadingStartTimeInMin
				&& pVehicle->m_DepartureTime < DemandLoadingEndTimeInMin)
			{
				std::vector<PathStatistics> Vehicle_ODTPathSet;

				Vehicle_ODTPathSet = ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet;

				bool ExistPath_Flag = false;
				int PathNo = 0;
				for (std::vector<PathStatistics>::iterator IterPS = Vehicle_ODTPathSet.begin(); IterPS != Vehicle_ODTPathSet.end(); IterPS++)
				{
					if (pVehicle->m_NodeNumberSum == IterPS->NodeSums && pVehicle->m_NodeSize == IterPS->m_NodeNumberArray.size()) //existing path
					{
						ExistPath_Flag = true;
						break;
					}

					PathNo++;
				}

				if (ExistPath_Flag)				// if old path, add statistics
				{
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet[PathNo].TotalVehicleSize += 1;
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet[PathNo].TotalDistance += pVehicle->m_Distance;
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet[PathNo].TotalFFTT += pVehicle->m_TripFFTT;
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet[PathNo].TotalTravelTime += (pVehicle->m_ArrivalTime - pVehicle->m_DepartureTime);
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet[PathNo].TotalEmissions += pVehicle->CO2;
				}
				else				           // if new path, add
				{
					PathStatistics vehicle_PathStatistics;

					vehicle_PathStatistics.Origin_ZoneID = pVehicle->m_OriginZoneID;
					vehicle_PathStatistics.Destination_ZoneID = pVehicle->m_DestinationZoneID;

					vehicle_PathStatistics.TotalVehicleSize = 1;
					vehicle_PathStatistics.TotalDistance = pVehicle->m_Distance;
					vehicle_PathStatistics.TotalFFTT = pVehicle->m_TripFFTT;

					vehicle_PathStatistics.TotalTravelTime = (pVehicle->m_ArrivalTime - pVehicle->m_DepartureTime);
					vehicle_PathStatistics.TotalEmissions = pVehicle->CO2;
					vehicle_PathStatistics.NodeSums = pVehicle->m_NodeNumberSum;


					if (pVehicle->m_NodeNumberSum == 217)
						TRACE("");

					// add node id along the path
					int NodeID = g_LinkVector[pVehicle->m_LinkAry[0].LinkNo]->m_FromNodeID;  // first node
					int NodeName = g_NodeVector[NodeID].m_NodeNumber;
					vehicle_PathStatistics.m_NodeNumberArray.push_back(NodeName);
					for (int j = 0; j < pVehicle->m_NodeSize - 1; j++)
					{
						int LinkID = pVehicle->m_LinkAry[j].LinkNo;
						int OrgLinkID = g_LinkVector[LinkID]->m_OrgLinkID;
						NodeID = g_LinkVector[LinkID]->m_ToNodeID;
						NodeName = g_NodeVector[NodeID].m_NodeNumber;
						vehicle_PathStatistics.m_NodeNumberArray.push_back(NodeName);
						vehicle_PathStatistics.m_LinkIDArray.push_back(OrgLinkID);

						
					}
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet.push_back(vehicle_PathStatistics);

				}
			}
		}

		// create time-dependent network
		g_TimeDependentNetwork_MP[0].BuildPhysicalNetwork(0, -1, g_TrafficFlowModelFlag, false, 0);  // use simulated travel time as time-dependent travel time 

		//
		for (int i = 0; i <= g_ODZoneIDSize; i++)
		{

			for (int j = 0; j <= g_ODZoneIDSize; j++)
				{
					if (i == j)  //skip intra zone demand
						continue;

					int PathNo = 1;
					for (std::vector<PathStatistics>::iterator IterPS = ODPathSetVector[i][j].PathSet.begin(); IterPS != ODPathSetVector[i][j].PathSet.end(); IterPS++)
					{
						if (IterPS->TotalVehicleSize >= cut_off_volume)
						{
							//int vehicles_completing_trips=ODPathSetVector[i][j].PathSet[PathNo].TotalVehicleSize;
							//int Total_TravelTime=ODPathSetVector[i][j].PathSet[PathNo].TotalTravelTime;
							//int NodeSums=ODPathSetVector[i][j].PathSet[PathNo].NodeSums;
							int vehicles_completing_trips = IterPS->TotalVehicleSize;
							float Total_TravelTime = IterPS->TotalTravelTime;
							float Total_FreeFlowTravelTime = IterPS->TotalFFTT;
							float Total_Distance = IterPS->TotalDistance;
							float Total_emissions = IterPS->TotalEmissions;
							int NodeSums = IterPS->NodeSums;
							float FlowRatio = vehicles_completing_trips*1.0 / max(1, ODPathSetVector[i][j].TotalVehicleSize);

							output_PathMOE_file << IterPS->Origin_ZoneID << "," << IterPS->Destination_ZoneID << "," << demand_type << ",0,"
								<< DemandLoadingStartTimeInMin << "," << DemandLoadingEndTimeInMin << ",";
							output_PathMOE_file << PathNo << "," << FlowRatio << ",";

							for (int n = 0; n < IterPS->m_NodeNumberArray.size(); n++)
							{
								output_PathMOE_file << IterPS->m_NodeNumberArray[n];

								if (n != IterPS->m_NodeNumberArray.size() - 1)
									output_PathMOE_file << ";";

							}
							output_PathMOE_file << ",";
							
							for (int n = 0; n < IterPS->m_LinkIDArray.size(); n++)
							{
								output_PathMOE_file << IterPS->m_LinkIDArray[n];

								if (n != IterPS->m_LinkIDArray.size() - 1)
									output_PathMOE_file << ";";

							}

							output_PathMOE_file << "," << IterPS->NodeSums << "," << Total_Distance*1.0f / vehicles_completing_trips <<
								"," << Total_TravelTime*1.0f / vehicles_completing_trips;

							output_PathMOE_file << "," << endl;
						}
						PathNo++;
					}
					

					}
				}
			
			// NOW	FOR INFORMATION TYPE 1
		// comment out for now, not outputting real time routing policy
		//	for (int i = 0; i <= g_ODZoneIDSize; i++)
		//	{

		//		int origin_node_indx = g_ZoneMap[i].GetRandomOriginNodeIDInZone((0) / 100.0f);  // use pVehicle->m_AgentID/100.0f as random number between 0 and 1, so we can reproduce the results easily

		//		if (origin_node_indx >= 0) // convert node number to internal node id
		//		{

		//			g_TimeDependentNetwork_MP[0].TDLabelCorrecting_DoubleQueue(origin_node_indx,
		//				i,
		//				DemandLoadingStartTimeInMin,
		//				demand_type, DEFAULT_VOT, false, false, true);

		//			for (int j = 0; j <= g_ODZoneIDSize; j++)
		//			{
		//				if (i == j)  //skip intra zone demand
		//					continue;

		//				/// fetch shortest path for information_type = 1
		//				int dest_node_index = g_ZoneMap[j].GetRandomDestinationIDInZone((0) / 100.0f);
		//				if (dest_node_index >= 0) // convert node number to internal node id
		//				{

		//					float TravelTime = g_TimeDependentNetwork_MP[0].LabelCostAry[dest_node_index];
		//					float TravelCost = g_TimeDependentNetwork_MP[0].LabelCostAry[dest_node_index];

		//					float TravelDistance = g_TimeDependentNetwork_MP[0].LabelDistanceAry[dest_node_index];
		//					float TravelDollarCost = g_TimeDependentNetwork_MP[0].LabelDollarCostAry[dest_node_index];


		//					///////////////////////////////////////////////////
		//					// fetch node sequence

		//					std::vector<int > path_node_sequence;

		//					int NodeSize = 0;
		//					int PredNode = g_TimeDependentNetwork_MP[0].NodePredAry[dest_node_index];

		//					int node_number = g_NodeVector[dest_node_index].m_NodeNumber;
		//					path_node_sequence.push_back(node_number);

		//					while (PredNode != -1) // scan backward in the predessor array of the shortest path calculation results
		//					{
		//						if (NodeSize >= MAX_NODE_SIZE_IN_A_PATH - 1)
		//						{

		//							break;
		//						}
		//						node_number = g_NodeVector[PredNode].m_NodeNumber;
		//						path_node_sequence.push_back(node_number);

		//						PredNode = g_TimeDependentNetwork_MP[0].NodePredAry[PredNode];

		//					}
		//					//end of fetch shortest path

		//					output_PathMOE_file << i << "," << j << "," << demand_type << ",1,"
		//						<< DemandLoadingStartTimeInMin << "," << DemandLoadingEndTimeInMin << ",";
		//					output_PathMOE_file << "1" << "," << "1.0" << ",";

		//					std::reverse(path_node_sequence.begin(), path_node_sequence.end());
		//					int node_sum = 0;
		//					for (int n = 0; n < path_node_sequence.size(); n++)
		//					{
		//						output_PathMOE_file << path_node_sequence[n];
		//						node_sum += path_node_sequence[n];
		//						if (n != path_node_sequence.size() - 1)
		//							output_PathMOE_file << ";";

		//					}

		//					output_PathMOE_file << "," << node_sum << "," << TravelDistance <<
		//						"," << TravelTime;

		//					output_PathMOE_file << "," << endl;


		//				}
		//			}
		//		}
		//}
	}
		if(ODPathSetVector!=NULL)
			DeallocateDynamicArray<ODPathSet>(ODPathSetVector,g_ODZoneIDSize+1,g_ODZoneIDSize+1);
}


void g_OutputRoutingPolicyData(ofstream &output_PathMOE_file, int DemandLoadingStartTimeInMin, int DemandLoadingEndTimeInMin,
	int cut_off_volume)
{


	ODPathSet** ODPathSetVector = NULL;

	ODPathSetVector = AllocateDynamicArray<ODPathSet>(g_ODZoneIDSize + 1, g_ODZoneIDSize + 1);

	int total_demand_type = g_DemandTypeVector.size();

	total_demand_type = 1; // demand type 1 only for simplity

	for (int demand_type = 1; demand_type <= total_demand_type; demand_type++)
	{

		for (int i = 0; i < g_ODZoneIDSize; i++)
			for (int j = 0; j < g_ODZoneIDSize; j++)
			{
				ODPathSetVector[i][j].PathSet.clear();

			}

		std::map<int, DTAVehicle*>::iterator iterVM;
		for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{

			DTAVehicle* pVehicle = iterVM->second;

			if (pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete
				&& pVehicle->m_DepartureTime >= DemandLoadingStartTimeInMin
				&& pVehicle->m_DepartureTime < DemandLoadingEndTimeInMin)
			{
				ODPathSetVector[g_ZoneMap[pVehicle->m_OriginZoneID].m_ZoneSequentialNo][g_ZoneMap[pVehicle->m_DestinationZoneID].m_ZoneSequentialNo].TotalVehicleSize += 1;
			}


		}


		for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{

			DTAVehicle* pVehicle = iterVM->second;

			int OrgZoneSequentialNo = g_ZoneMap[pVehicle->m_OriginZoneID].m_ZoneSequentialNo;
			int DestZoneSequentialNo = g_ZoneMap[pVehicle->m_DestinationZoneID].m_ZoneSequentialNo;

			if (pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete && ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].TotalVehicleSize >= cut_off_volume
				&& pVehicle->m_DemandType == demand_type
				&& pVehicle->m_DepartureTime >= DemandLoadingStartTimeInMin
				&& pVehicle->m_DepartureTime < DemandLoadingEndTimeInMin)
			{
				std::vector<PathStatistics> Vehicle_ODTPathSet;

				Vehicle_ODTPathSet = ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet;

				bool ExistPath_Flag = false;
				int PathNo = 0;
				for (std::vector<PathStatistics>::iterator IterPS = Vehicle_ODTPathSet.begin(); IterPS != Vehicle_ODTPathSet.end(); IterPS++)
				{
					if (pVehicle->m_NodeNumberSum == IterPS->NodeSums && pVehicle->m_NodeSize == IterPS->m_NodeNumberArray.size()) //existing path
					{
						ExistPath_Flag = true;
						break;
					}

					PathNo++;
				}

				if (ExistPath_Flag)				// if old path, add statistics
				{
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet[PathNo].TotalVehicleSize += 1;
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet[PathNo].TotalDistance += pVehicle->m_Distance;
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet[PathNo].TotalFFTT += pVehicle->m_TripFFTT;
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet[PathNo].TotalTravelTime += (pVehicle->m_ArrivalTime - pVehicle->m_DepartureTime);
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet[PathNo].TotalEmissions += pVehicle->CO2;
				}
				else				           // if new path, add
				{
					PathStatistics vehicle_PathStatistics;

					vehicle_PathStatistics.Origin_ZoneID = pVehicle->m_OriginZoneID;
					vehicle_PathStatistics.Destination_ZoneID = pVehicle->m_DestinationZoneID;

					vehicle_PathStatistics.TotalVehicleSize = 1;
					vehicle_PathStatistics.TotalDistance = pVehicle->m_Distance;
					vehicle_PathStatistics.TotalFFTT = pVehicle->m_TripFFTT;

					vehicle_PathStatistics.TotalTravelTime = (pVehicle->m_ArrivalTime - pVehicle->m_DepartureTime);
					vehicle_PathStatistics.TotalEmissions = pVehicle->CO2;
					vehicle_PathStatistics.NodeSums = pVehicle->m_NodeNumberSum;


					if (pVehicle->m_NodeNumberSum == 217)
						TRACE("");

					// add node id along the path
					int NodeID = g_LinkVector[pVehicle->m_LinkAry[0].LinkNo]->m_FromNodeID;  // first node
					int NodeName = g_NodeVector[NodeID].m_NodeNumber;
					vehicle_PathStatistics.m_NodeNumberArray.push_back(NodeName);
					for (int j = 0; j < pVehicle->m_NodeSize - 1; j++)
					{
						int LinkID = pVehicle->m_LinkAry[j].LinkNo;
						int OrgLinkID = g_LinkVector[LinkID]->m_OrgLinkID;
						NodeID = g_LinkVector[LinkID]->m_ToNodeID;
						NodeName = g_NodeVector[NodeID].m_NodeNumber;
						vehicle_PathStatistics.m_NodeNumberArray.push_back(NodeName);
						vehicle_PathStatistics.m_LinkIDArray.push_back(OrgLinkID);


					}
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet.push_back(vehicle_PathStatistics);

				}
			}
		}

																								 //
		for (int i = 0; i <= g_ODZoneIDSize; i++)
		{

			for (int j = 0; j <= g_ODZoneIDSize; j++)
			{
				if (i == j)  //skip intra zone demand
					continue;

				int PathNo = 1;
				for (std::vector<PathStatistics>::iterator IterPS = ODPathSetVector[i][j].PathSet.begin(); IterPS != ODPathSetVector[i][j].PathSet.end(); IterPS++)
				{
					if (IterPS->TotalVehicleSize >= cut_off_volume)
					{
						//int vehicles_completing_trips=ODPathSetVector[i][j].PathSet[PathNo].TotalVehicleSize;
						//int Total_TravelTime=ODPathSetVector[i][j].PathSet[PathNo].TotalTravelTime;
						//int NodeSums=ODPathSetVector[i][j].PathSet[PathNo].NodeSums;
						int vehicles_completing_trips = IterPS->TotalVehicleSize;
						float Total_TravelTime = IterPS->TotalTravelTime;
						float Total_FreeFlowTravelTime = IterPS->TotalFFTT;
						float Total_Distance = IterPS->TotalDistance;
						float Total_emissions = IterPS->TotalEmissions;
						int NodeSums = IterPS->NodeSums;
						float FlowRatio = vehicles_completing_trips*1.0 / max(1, ODPathSetVector[i][j].TotalVehicleSize);

						output_PathMOE_file << IterPS->Origin_ZoneID << "," << IterPS->Destination_ZoneID << "," << demand_type << ",0,"
							<< DemandLoadingStartTimeInMin << "," << DemandLoadingEndTimeInMin << ",";
						output_PathMOE_file << PathNo << ","<< vehicles_completing_trips << ","  << FlowRatio << ",";

						for (int n = 0; n < IterPS->m_NodeNumberArray.size(); n++)
						{
							output_PathMOE_file << IterPS->m_NodeNumberArray[n];

							if (n != IterPS->m_NodeNumberArray.size() - 1)
								output_PathMOE_file << ";";

						}
						output_PathMOE_file << ",";

						for (int n = 0; n < IterPS->m_LinkIDArray.size(); n++)
						{
							output_PathMOE_file << IterPS->m_LinkIDArray[n];

							if (n != IterPS->m_LinkIDArray.size() - 1)
								output_PathMOE_file << ";";

						}

						output_PathMOE_file << "," << IterPS->NodeSums << "," << Total_Distance*1.0f / vehicles_completing_trips <<
							"," << Total_TravelTime*1.0f / vehicles_completing_trips;

						output_PathMOE_file << "," << endl;
					}
					PathNo++;
				}


			}
		}


	}
	if (ODPathSetVector != NULL)
		DeallocateDynamicArray<ODPathSet>(ODPathSetVector, g_ODZoneIDSize + 1, g_ODZoneIDSize + 1);
}

void g_OutputLinkPathProportionData(ofstream &output_PathMOE_file, int DemandLoadingStartTimeInMin, int DemandLoadingEndTimeInMin,
	int cut_off_volume)
{


	ODPathSet** ODPathSetVector = NULL;

	ODPathSetVector = AllocateDynamicArray<ODPathSet>(g_ODZoneIDSize + 1, g_ODZoneIDSize + 1);

	int total_demand_type = g_DemandTypeVector.size();

	total_demand_type = 1; // demand type 1 only for simplity

	for (int demand_type = 1; demand_type <= total_demand_type; demand_type++)
	{

		for (int i = 0; i < g_ODZoneIDSize; i++)
			for (int j = 0; j < g_ODZoneIDSize; j++)
			{
				ODPathSetVector[i][j].PathSet.clear();

			}

		std::map<int, DTAVehicle*>::iterator iterVM;
		for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{

			DTAVehicle* pVehicle = iterVM->second;

			if (pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete
				&& pVehicle->m_DepartureTime >= DemandLoadingStartTimeInMin
				&& pVehicle->m_DepartureTime < DemandLoadingEndTimeInMin)
			{
				ODPathSetVector[g_ZoneMap[pVehicle->m_OriginZoneID].m_ZoneSequentialNo][g_ZoneMap[pVehicle->m_DestinationZoneID].m_ZoneSequentialNo].TotalVehicleSize += 1;
			}


		}


		for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{

			DTAVehicle* pVehicle = iterVM->second;

			int OrgZoneSequentialNo = g_ZoneMap[pVehicle->m_OriginZoneID].m_ZoneSequentialNo;
			int DestZoneSequentialNo = g_ZoneMap[pVehicle->m_DestinationZoneID].m_ZoneSequentialNo;

			if (pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete && ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].TotalVehicleSize >= cut_off_volume
				&& pVehicle->m_DemandType == demand_type
				&& pVehicle->m_DepartureTime >= DemandLoadingStartTimeInMin
				&& pVehicle->m_DepartureTime < DemandLoadingEndTimeInMin)
			{
				std::vector<PathStatistics> Vehicle_ODTPathSet;

				Vehicle_ODTPathSet = ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet;

				bool ExistPath_Flag = false;
				int PathNo = 0;
				for (std::vector<PathStatistics>::iterator IterPS = Vehicle_ODTPathSet.begin(); IterPS != Vehicle_ODTPathSet.end(); IterPS++)
				{
					if (pVehicle->m_NodeNumberSum == IterPS->NodeSums && pVehicle->m_NodeSize == IterPS->m_NodeNumberArray.size()) //existing path
					{
						ExistPath_Flag = true;
						break;
					}

					PathNo++;
				}

				if (ExistPath_Flag)				// if old path, add statistics
				{
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet[PathNo].TotalVehicleSize += 1;
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet[PathNo].TotalDistance += pVehicle->m_Distance;
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet[PathNo].TotalFFTT += pVehicle->m_TripFFTT;
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet[PathNo].TotalTravelTime += (pVehicle->m_ArrivalTime - pVehicle->m_DepartureTime);
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet[PathNo].TotalEmissions += pVehicle->CO2;
				}
				else				           // if new path, add
				{
					PathStatistics vehicle_PathStatistics;

					vehicle_PathStatistics.Origin_ZoneID = pVehicle->m_OriginZoneID;
					vehicle_PathStatistics.Destination_ZoneID = pVehicle->m_DestinationZoneID;

					vehicle_PathStatistics.TotalVehicleSize = 1;
					vehicle_PathStatistics.TotalDistance = pVehicle->m_Distance;
					vehicle_PathStatistics.TotalFFTT = pVehicle->m_TripFFTT;

					vehicle_PathStatistics.TotalTravelTime = (pVehicle->m_ArrivalTime - pVehicle->m_DepartureTime);
					vehicle_PathStatistics.TotalEmissions = pVehicle->CO2;
					vehicle_PathStatistics.NodeSums = pVehicle->m_NodeNumberSum;


					if (pVehicle->m_NodeNumberSum == 217)
						TRACE("");

					// add node id along the path
					int NodeID = g_LinkVector[pVehicle->m_LinkAry[0].LinkNo]->m_FromNodeID;  // first node
					int NodeName = g_NodeVector[NodeID].m_NodeNumber;
					vehicle_PathStatistics.m_NodeNumberArray.push_back(NodeName);
					for (int j = 0; j < pVehicle->m_NodeSize - 1; j++)
					{
						int LinkID = pVehicle->m_LinkAry[j].LinkNo;
						int OrgLinkID = g_LinkVector[LinkID]->m_OrgLinkID;
						NodeID = g_LinkVector[LinkID]->m_ToNodeID;
						NodeName = g_NodeVector[NodeID].m_NodeNumber;
						vehicle_PathStatistics.m_NodeNumberArray.push_back(NodeName);
						vehicle_PathStatistics.m_LinkIDArray.push_back(OrgLinkID);


					}
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet.push_back(vehicle_PathStatistics);

				}
			}
		}

		//
		for (int i = 0; i <= g_ODZoneIDSize; i++)
		{

			for (int j = 0; j <= g_ODZoneIDSize; j++)
			{
				if (i == j)  //skip intra zone demand
					continue;

				int PathNo = 1;
				for (std::vector<PathStatistics>::iterator IterPS = ODPathSetVector[i][j].PathSet.begin(); IterPS != ODPathSetVector[i][j].PathSet.end(); IterPS++)
				{
					if (IterPS->TotalVehicleSize >= cut_off_volume)
					{
						//int vehicles_completing_trips=ODPathSetVector[i][j].PathSet[PathNo].TotalVehicleSize;
						//int Total_TravelTime=ODPathSetVector[i][j].PathSet[PathNo].TotalTravelTime;
						//int NodeSums=ODPathSetVector[i][j].PathSet[PathNo].NodeSums;
						int vehicles_completing_trips = IterPS->TotalVehicleSize;
						float Total_TravelTime = IterPS->TotalTravelTime;
						float Total_FreeFlowTravelTime = IterPS->TotalFFTT;
						float Total_Distance = IterPS->TotalDistance;
						float Total_emissions = IterPS->TotalEmissions;
						int NodeSums = IterPS->NodeSums;
						float FlowRatio = vehicles_completing_trips*1.0 / max(1, ODPathSetVector[i][j].TotalVehicleSize);

						for (int n = 0; n < IterPS->m_LinkIDArray.size(); n++)
						{
							output_PathMOE_file << IterPS->Origin_ZoneID << ". ," << IterPS->Destination_ZoneID << ". ,"
								<< DemandLoadingStartTimeInMin / 15 << ". ," << PathNo << ". ," <<
								IterPS->m_LinkIDArray[n] << " ," << FlowRatio << endl;
						}

						output_PathMOE_file << " " << endl;
					}
					PathNo++;
				}


			}
		}


	}
	if (ODPathSetVector != NULL)
		DeallocateDynamicArray<ODPathSet>(ODPathSetVector, g_ODZoneIDSize + 1, g_ODZoneIDSize + 1);
}
void g_OutputLinkProportionData(ofstream &output_PathMOE_file, int DemandLoadingStartTimeInMin, int DemandLoadingEndTimeInMin,
	int cut_off_volume)
{


	ODPathSet** ODPathSetVector = NULL;

	ODPathSetVector = AllocateDynamicArray<ODPathSet>(g_ODZoneIDSize + 1, g_ODZoneIDSize + 1);

	int total_demand_type = g_DemandTypeVector.size();

	total_demand_type = 1; // demand type 1 only for simplity

	for (int demand_type = 1; demand_type <= total_demand_type; demand_type++)
	{

		for (int i = 0; i < g_ODZoneIDSize; i++)
			for (int j = 0; j < g_ODZoneIDSize; j++)
			{
				ODPathSetVector[i][j].PathSet.clear();

			}

		std::map<int, DTAVehicle*>::iterator iterVM;
		for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{

			DTAVehicle* pVehicle = iterVM->second;

			if (pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete
				&& pVehicle->m_DepartureTime >= DemandLoadingStartTimeInMin
				&& pVehicle->m_DepartureTime < DemandLoadingEndTimeInMin)
			{
				ODPathSetVector[g_ZoneMap[pVehicle->m_OriginZoneID].m_ZoneSequentialNo][g_ZoneMap[pVehicle->m_DestinationZoneID].m_ZoneSequentialNo].TotalVehicleSize += 1;
			}


		}


		for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{

			DTAVehicle* pVehicle = iterVM->second;

			int OrgZoneSequentialNo = g_ZoneMap[pVehicle->m_OriginZoneID].m_ZoneSequentialNo;
			int DestZoneSequentialNo = g_ZoneMap[pVehicle->m_DestinationZoneID].m_ZoneSequentialNo;

			if (pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete && ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].TotalVehicleSize >= cut_off_volume
				&& pVehicle->m_DemandType == demand_type
				&& pVehicle->m_DepartureTime >= DemandLoadingStartTimeInMin
				&& pVehicle->m_DepartureTime < DemandLoadingEndTimeInMin)
			{
				std::vector<PathStatistics> Vehicle_ODTPathSet;

				Vehicle_ODTPathSet = ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet;

				bool ExistPath_Flag = false;
				int PathNo = 0;
				for (std::vector<PathStatistics>::iterator IterPS = Vehicle_ODTPathSet.begin(); IterPS != Vehicle_ODTPathSet.end(); IterPS++)
				{
					if (pVehicle->m_NodeNumberSum == IterPS->NodeSums && pVehicle->m_NodeSize == IterPS->m_NodeNumberArray.size()) //existing path
					{
						ExistPath_Flag = true;
						break;
					}

					PathNo++;
				}

				if (ExistPath_Flag)				// if old path, add statistics
				{
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet[PathNo].TotalVehicleSize += 1;
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet[PathNo].TotalDistance += pVehicle->m_Distance;
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet[PathNo].TotalFFTT += pVehicle->m_TripFFTT;
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet[PathNo].TotalTravelTime += (pVehicle->m_ArrivalTime - pVehicle->m_DepartureTime);
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet[PathNo].TotalEmissions += pVehicle->CO2;
				}
				else				           // if new path, add
				{
					PathStatistics vehicle_PathStatistics;

					vehicle_PathStatistics.Origin_ZoneID = pVehicle->m_OriginZoneID;
					vehicle_PathStatistics.Destination_ZoneID = pVehicle->m_DestinationZoneID;

					vehicle_PathStatistics.TotalVehicleSize = 1;
					vehicle_PathStatistics.TotalDistance = pVehicle->m_Distance;
					vehicle_PathStatistics.TotalFFTT = pVehicle->m_TripFFTT;

					vehicle_PathStatistics.TotalTravelTime = (pVehicle->m_ArrivalTime - pVehicle->m_DepartureTime);
					vehicle_PathStatistics.TotalEmissions = pVehicle->CO2;
					vehicle_PathStatistics.NodeSums = pVehicle->m_NodeNumberSum;


					if (pVehicle->m_NodeNumberSum == 217)
						TRACE("");

					// add node id along the path
					int NodeID = g_LinkVector[pVehicle->m_LinkAry[0].LinkNo]->m_FromNodeID;  // first node
					int NodeName = g_NodeVector[NodeID].m_NodeNumber;
					vehicle_PathStatistics.m_NodeNumberArray.push_back(NodeName);
					for (int j = 0; j < pVehicle->m_NodeSize - 1; j++)
					{
						int LinkID = pVehicle->m_LinkAry[j].LinkNo;
						int OrgLinkID = g_LinkVector[LinkID]->m_OrgLinkID;
						NodeID = g_LinkVector[LinkID]->m_ToNodeID;
						NodeName = g_NodeVector[NodeID].m_NodeNumber;
						vehicle_PathStatistics.m_NodeNumberArray.push_back(NodeName);
						vehicle_PathStatistics.m_LinkIDArray.push_back(OrgLinkID);


					}
					ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo].PathSet.push_back(vehicle_PathStatistics);

				}
			}
		}

		//
		for (int i = 0; i <= g_ODZoneIDSize; i++)
		{

			for (int j = 0; j <= g_ODZoneIDSize; j++)
			{
				if (i == j || ODPathSetVector[i][j].PathSet.size() == 0)  //skip 
					continue;
				
				std::map<int, float> link_porportion_map;


				for (std::vector<PathStatistics>::iterator IterPS = ODPathSetVector[i][j].PathSet.begin(); IterPS != ODPathSetVector[i][j].PathSet.end(); IterPS++)
				{
					for (int n = 0; n < IterPS->m_LinkIDArray.size(); n++)
					{

						int vehicles_completing_trips = IterPS->TotalVehicleSize;
						float FlowRatio = vehicles_completing_trips*1.0 / max(1, ODPathSetVector[i][j].TotalVehicleSize);
						if (link_porportion_map.find(IterPS->m_LinkIDArray[n]) != link_porportion_map.end())
						{ // defined
							link_porportion_map[IterPS->m_LinkIDArray[n]] = link_porportion_map[IterPS->m_LinkIDArray[n]] + FlowRatio;
						}
						else
						{
							link_porportion_map[IterPS->m_LinkIDArray[n]] = FlowRatio;
						}

					}
				}
				for (std::map<int, float>::iterator iter_link = link_porportion_map.begin(); iter_link != link_porportion_map.end(); iter_link++)
				{

					output_PathMOE_file << ODPathSetVector[i][j].PathSet[0].Origin_ZoneID << ". ," << 
						ODPathSetVector[i][j].PathSet[0].Destination_ZoneID<< ". ,"
						<< DemandLoadingStartTimeInMin / 15 << ". ," <<
						iter_link ->first << " ," << iter_link->second << endl;

				}

			}
		}


	}
	if (ODPathSetVector != NULL)
		DeallocateDynamicArray<ODPathSet>(ODPathSetVector, g_ODZoneIDSize + 1, g_ODZoneIDSize + 1);
}



bool g_ReadTimeDependentRoutingPolicyData(std::string file_name, int DemandLoadingStartTimeInMin)
{

	if (g_ODPathSetVector == NULL)
		return false;

	CCSVParser parser_routing_policy;
	if (parser_routing_policy.OpenCSVFile(file_name, false))
	{

		g_LogFile << "Reading file " << file_name << endl;
		// clean the memory
		for(int i = 0; i<g_ODZoneIDSize; i++)
		{
			for(int j = 0; j<g_ODZoneIDSize; j++)
			for (int it = 0; it < _max_info_type; it++)
			{	g_ODPathSetVector[i][j][it].PathSet .clear ();
			}
		
		}

		
		
		int record_count = 0;

		while(parser_routing_policy.ReadRecord())
		{

			int from_zone_id = 0;
			int to_zone_id = 0;
			float ratio = 0;

			int information_type = 0;

			parser_routing_policy.GetValueByFieldNameRequired("from_zone_id",from_zone_id);
			parser_routing_policy.GetValueByFieldNameRequired("to_zone_id",to_zone_id);
			parser_routing_policy.GetValueByFieldNameRequired("information_type", information_type);

			if (information_type < 0)
				information_type = 0;

			if (information_type >= 2 )  // we only allow types 0 and 1
				information_type = 1;


			if(g_ZoneMap.find( from_zone_id)== g_ZoneMap.end() || g_ZoneMap.find( to_zone_id)== g_ZoneMap.end() )
			{
				continue;
			}

			parser_routing_policy.GetValueByFieldNameRequired("ratio",ratio);

			float departure_start_time_in_min = 0;
			parser_routing_policy.GetValueByFieldNameRequired("departure_start_time_in_min",departure_start_time_in_min);

			float departure_end_time_in_min = 0;
			parser_routing_policy.GetValueByFieldNameRequired("departure_end_time_in_min",departure_end_time_in_min);


			std::vector<int> path_node_sequence;
			string path_node_sequence_str; 
			parser_routing_policy.GetValueByFieldName("node_sequence",path_node_sequence_str);

			if(path_node_sequence_str.size ()>0)
			{
				path_node_sequence = ParseLineToIntegers(path_node_sequence_str);

				PathStatistics element;

				int OrgZoneSequentialNo = g_ZoneMap[from_zone_id].m_ZoneSequentialNo;
				int DestZoneSequentialNo = g_ZoneMap[to_zone_id].m_ZoneSequentialNo;

				element.Origin_ZoneID = from_zone_id;
				element.Destination_ZoneID = to_zone_id;
				element.Ratio = ratio;

				for(int j = 0; j< path_node_sequence.size(); j++)
				{
					element.m_NodeNumberArray .push_back (path_node_sequence[j]);
				}
				g_ODPathSetVector[OrgZoneSequentialNo][DestZoneSequentialNo][information_type].PathSet.push_back(element);

			}
		}


		
		// calculate cumulative ratio 
		for(int i = 0; i<g_ODZoneIDSize; i++)
		{
			for(int j = 0; j<g_ODZoneIDSize; j++)
			{	
				for (int it = 0; it < _max_info_type; it++)
				{
				
				float cumulative_ratio = 0;
				for(int k = 0; k< g_ODPathSetVector[i][j][it].PathSet.size(); k++)
				{
				
					cumulative_ratio += g_ODPathSetVector[i][j][it].PathSet[k].Ratio;
					g_ODPathSetVector[i][j][it].PathSet[k].CumulativeRatio = cumulative_ratio;

				
				}
				}
			}
		
		}

		// after reading, apply this routing ratio to all vehicles in the OD pair during this time interval
		g_ApplyExternalPathInput(DemandLoadingStartTimeInMin);
	
		return true;
	}
	return false;
}



int g_OutputSimulationSummary(float& AvgTripTime, float& AvgDistance, float& AvgSpeed, float& AvgCost, EmissionStatisticsData &emission_data,
							  int InformationClass=-1, int DemandType=-1,int VehicleType = -1, int DepartureTimeInterval = -1)
{
	float TripTimeSum = 0;
	float TravelCostSum = 0;
	float DistanceSum = 0;
	int VehicleCount = 0;

	emission_data.Init ();

	for (vector<DTAVehicle*>::iterator v = g_VehicleVector.begin(); v != g_VehicleVector.end();v++)
	{

		DTAVehicle* pVehicle = (*v);
		if(pVehicle->m_bComplete )  // vehicle completes the trips
		{
			if( (pVehicle->m_InformationType == InformationClass ||InformationClass==-1)
				&& (pVehicle->m_DemandType == DemandType ||DemandType==-1)
				&& (pVehicle->m_VehicleType  == VehicleType ||VehicleType==-1)
				&& (g_FindAssignmentIntervalIndexFromTime(pVehicle->m_DepartureTime) == DepartureTimeInterval || DepartureTimeInterval == -1))
			{
				TripTimeSum += pVehicle->m_TripTime;
				TravelCostSum += pVehicle->m_TollDollarCost ;

				DistanceSum+= pVehicle ->m_Distance;

				emission_data.TotalEnergy +=  pVehicle->Energy;
				emission_data.TotalCO2 +=  pVehicle->CO2;
				emission_data.TotalCO +=  pVehicle->CO;
				emission_data.TotalNOX +=  pVehicle->NOX;
				emission_data.TotalHC +=  pVehicle->HC;
				emission_data.TotalPM += pVehicle->PM;
				emission_data.TotalPM2_5 += pVehicle->PM2_5;

				VehicleCount++;

			}
		}
	}

	AvgTripTime = TripTimeSum/max(1,VehicleCount);  // unit: min
	AvgDistance = DistanceSum/max(1,VehicleCount);  // unit: mile
	AvgSpeed = AvgDistance/max(0.0001,AvgTripTime)*60;  // unit: mph
	AvgCost = TravelCostSum/max(1,VehicleCount);  // unit: mph

	emission_data.AvgEnergy = emission_data.TotalEnergy/max(1,VehicleCount);
	emission_data.AvgCO2 = emission_data.TotalCO2/max(1,VehicleCount);
	emission_data.AvgCO = emission_data.TotalCO/max(1,VehicleCount);
	emission_data.AvgNOX = emission_data.TotalNOX/max(1,VehicleCount);
	emission_data.AvgHC = emission_data.TotalHC/max(1,VehicleCount);
	emission_data.AvgPM = emission_data.TotalPM / max(1, VehicleCount);
	emission_data.AvgPM2_5 = emission_data.TotalPM2_5 / max(1, VehicleCount);

	return VehicleCount;
}


int g_OutputSimulationMOESummary(float& AvgTripTime, float& AvgDistance, float& AvgSpeed, float & AvgTollCost,  EmissionStatisticsData &emission_data, LinkMOEStatisticsData &link_data,
								 int DemandType,int VehicleType, int InformationClass, int from_zone_id, int to_zone_id,
								 int from_node_id, int mid_node_id	, int to_node_id	,	
								 int departure_starting_time,int departure_ending_time,int entrance_starting_time, int entrance_ending_time)
{
	float TripTimeSum = 0;
	float DistanceSum = 0;
	float CostSum = 0;
	int VehicleCount = 0;


	emission_data.Init ();
	link_data.Init ();

	if(from_node_id ==0)  // not link or path MOE
	{
		for (vector<DTAVehicle*>::iterator v = g_VehicleVector.begin(); v != g_VehicleVector.end();v++)
		{

			DTAVehicle* pVehicle = (*v);
			if(pVehicle->m_bComplete)  // vehicle completes the trips
			{
				if( (pVehicle->m_InformationType == InformationClass ||InformationClass <=0)
					&& (pVehicle->m_DemandType == DemandType ||DemandType<=0)
					&& (pVehicle->m_VehicleType  == VehicleType ||VehicleType <=0)
					&& (pVehicle->m_OriginZoneID    == from_zone_id ||from_zone_id <=0)
					&& (pVehicle->m_DestinationZoneID    == to_zone_id ||to_zone_id <=0)
					&& ((int)(pVehicle->m_DepartureTime) >= departure_starting_time && (int)(pVehicle->m_DepartureTime < departure_ending_time)|| (departure_starting_time	 == 0 && departure_ending_time ==0) || (departure_starting_time	 == 0 && departure_ending_time == 1440)))
				{


					TripTimeSum += pVehicle->m_TripTime;
					DistanceSum+= pVehicle ->m_Distance;
					CostSum += pVehicle ->m_TollDollarCost;
					emission_data.TotalEnergy +=  pVehicle->Energy;
					emission_data.TotalCO2 +=  pVehicle->CO2;
					emission_data.TotalCO +=  pVehicle->CO;
					emission_data.TotalNOX +=  pVehicle->NOX;
					emission_data.TotalHC +=  pVehicle->HC;
					emission_data.TotalPM += pVehicle->PM;
					emission_data.TotalPM2_5 += pVehicle->PM2_5;


					double Gasoline = pVehicle->Energy/1000/(121.7);  // convert energy from KJ to mega joules  then to gasline per gallon
					double MilesPerGallon = 0;

					if(Gasoline>0.0001)
						MilesPerGallon = pVehicle->m_Distance / max(0.0001,Gasoline);

					emission_data.TotalMilesPerGallon += MilesPerGallon;

					emission_data.TotalMiles += pVehicle->m_Distance;
					emission_data.TotalGasolineGallon += Gasoline;


					//					TRACE("veh id: %d,NOX: %f\n",pVehicle->m_AgentID ,pVehicle->NOX);

					VehicleCount++;

				}
			}
		}

		AvgTripTime = TripTimeSum/max(1,VehicleCount);  // unit: min
		AvgDistance = DistanceSum/max(1,VehicleCount);  // unit: mile
		AvgSpeed = AvgDistance/max(1,AvgTripTime)*60;  // unit: mph
		AvgTollCost = CostSum/max(1,VehicleCount);

		emission_data.AvgEnergy = emission_data.TotalEnergy/max(1,VehicleCount);
		emission_data.AvgCO2 = emission_data.TotalCO2/max(1,VehicleCount);
		emission_data.AvgCO = emission_data.TotalCO/max(1,VehicleCount);
		emission_data.AvgNOX = emission_data.TotalNOX/max(1,VehicleCount);
		emission_data.AvgHC = emission_data.TotalHC/max(1,VehicleCount);
		emission_data.AvgPM = emission_data.TotalPM / max(1, VehicleCount);
		emission_data.AvgPM2_5 = emission_data.TotalPM2_5 / max(1, VehicleCount);

		if(emission_data.AvgEnergy >0.00001)
		{
			emission_data.AvgMilesPerGallon = emission_data.TotalMiles /max(0.1,emission_data.TotalGasolineGallon );
		}

	}else
	{  

		if(mid_node_id== 0 )  // Link MOE
		{

			for(unsigned li = 0; li< g_LinkVector.size(); li++)
			{
				DTALink* pLink = g_LinkVector[li];

				if(pLink->m_FromNodeNumber == from_node_id && pLink->m_ToNodeNumber == to_node_id)
				{

					AvgTripTime = pLink->GetTravelTimeByMin(g_NumberOfIterations,entrance_starting_time, entrance_ending_time-entrance_starting_time,g_TrafficFlowModelFlag);

					AvgSpeed =		pLink->m_Length / max(0.00001,AvgTripTime) *60;  // unit: mph
					ASSERT(AvgSpeed <= pLink->m_SpeedLimit +1 );
					AvgDistance = pLink->m_Length;

					VehicleCount = pLink->GetTrafficVolumeByMin(g_NumberOfIterations,entrance_starting_time, entrance_ending_time-entrance_starting_time);

					if(VehicleCount==0)  // no flow reset
					{
						AvgTripTime= 0;
						AvgDistance = 0;
						AvgSpeed = 0;
					}


					emission_data.AvgEnergy = pLink->TotalEnergy /max(1,pLink->CFlowArrivalCount );
					emission_data.AvgCO2 =  pLink->TotalCO2/max(1,pLink->CFlowArrivalCount );
					emission_data.AvgCO =   pLink->TotalCO/max(1,pLink->CFlowArrivalCount );
					emission_data.AvgNOX =  pLink->TotalNOX/max(1,pLink->CFlowArrivalCount );
					emission_data.AvgHC =  pLink->TotalHC/max(1,pLink->CFlowArrivalCount );
					emission_data.AvgPM = pLink->TotalPM / max(1, pLink->CFlowArrivalCount);
					emission_data.AvgPM2_5 = pLink->TotalPM2_5 / max(1, pLink->CFlowArrivalCount);

					link_data.SOV_volume =  pLink->CFlowArrivalCount_DemandType [1];
					link_data.HOV_volume  =  pLink->CFlowArrivalCount_DemandType [2];
					link_data.Truck_volume  =  pLink->CFlowArrivalCount_DemandType [3];
					link_data.Intermodal_volume  =  pLink->CFlowArrivalCount_DemandType [4];

					break;
				} // for this link
			}
		}else  /////////////////////////////////// 3 point path
		{

			for (vector<DTAVehicle*>::iterator v = g_VehicleVector.begin(); v != g_VehicleVector.end();v++)
			{

				DTAVehicle* pVehicle = (*v);
				if(pVehicle->m_bComplete)  // vehicle completes the trips
				{
					if( (pVehicle->m_InformationType == InformationClass ||InformationClass <=0)
						&& (pVehicle->m_DemandType == DemandType ||DemandType<=0)
						&& (pVehicle->m_VehicleType  == VehicleType ||VehicleType <=0)
						&& (pVehicle->m_OriginZoneID    == from_zone_id ||from_zone_id <=0)
						&& (pVehicle->m_DestinationZoneID    == to_zone_id ||to_zone_id <=0)
						&& ((int)(pVehicle->m_DepartureTime) >= departure_starting_time && (int)(pVehicle->m_DepartureTime < departure_ending_time)|| (departure_starting_time	 == 0 && departure_ending_time ==0) || (departure_starting_time	 == 0 && departure_ending_time == 1440)))
					{
						//step 1: test three point condition


						bool pass_from_node_id = false;
						bool pass_mid_node_id = false;
						bool pass_to_node_id = false;
						float from_node_id_timestamp = 0;
						float end_node_id_timestamp = 0;

						int NodeID = g_LinkVector[pVehicle->m_LinkAry [0].LinkNo]->m_FromNodeID;  // first node
						int NodeName = g_NodeVector[NodeID].m_NodeNumber ;
						float total_path_distance = 0;

						EmissionStatisticsData sub_path_emission_data;

						if(NodeName == from_node_id)
						{
							pass_from_node_id = true;
							from_node_id_timestamp = pVehicle->m_DepartureTime ;
						}

						for(int j = 0; j< pVehicle->m_NodeSize-1; j++)
						{
							int LinkID = pVehicle->m_LinkAry [j].LinkNo;
							NodeID = g_LinkVector[LinkID]->m_ToNodeID;
							NodeName = g_NodeVector[NodeID].m_NodeNumber ;

							if(NodeName == from_node_id)
							{
								pass_from_node_id = true;

								if(j>0)
									from_node_id_timestamp = pVehicle->m_LinkAry [j-1].AbsArrivalTimeOnDSN;
								else
									from_node_id_timestamp = pVehicle->m_DepartureTime ;
							}

							if(pass_from_node_id && NodeName == mid_node_id)
							{
								pass_mid_node_id = true;
							}

							if(pass_from_node_id)  // start counting distance along the path
							{
								DTALink* pLink  = g_LinkVector[LinkID];
								total_path_distance += pLink->m_Length ;
								// be careful here. we do not have link based statistics for each vehicle, so we use link based avg statistics
								sub_path_emission_data.TotalEnergy += pLink->TotalEnergy /max(1,pLink->CFlowArrivalCount );
								sub_path_emission_data.TotalCO2 += pLink->TotalCO2 /max(1,pLink->CFlowArrivalCount );
								sub_path_emission_data.TotalCO += pLink->TotalCO /max(1,pLink->CFlowArrivalCount );
								sub_path_emission_data.TotalNOX += pLink->TotalNOX /max(1,pLink->CFlowArrivalCount );
								sub_path_emission_data.TotalHC += pLink->TotalHC /max(1,pLink->CFlowArrivalCount );
								sub_path_emission_data.TotalPM += pLink->TotalPM/ max(1, pLink->CFlowArrivalCount);
								sub_path_emission_data.TotalPM2_5 += pLink->TotalPM2_5 / max(1, pLink->CFlowArrivalCount);

								float Gasoline =0;

								if(pVehicle->m_VehicleType >=3) // trucks typically using diesel fuel
									Gasoline =  pVehicle->Energy/(146.3 *1000);
								else  // passenger vehicles
									Gasoline = pVehicle->Energy/(121.7*1000); // passenger cars using gasoline

								//1 gallon = 121.7 megajoules – LHV *
								//Diesel fuel // 1 gallon = 146.3 megajoules – HHV *

								//source:
								//http://ecotec-systems.com/Resources/FUEL_CONVERSION_WORK_SHEET.pdf
								//					Energy contents are expressed as either High (gross) Heating Value (HHV) or Lower (net) 
								//Heating Value (LHV). LHV is closest to the actual energy yield in most cases. HHV (including 
								//condensation of combustion products) is greater by between 5% (in the case of coal) and 
								//10% (for natural gas), depending mainly on the hydrogen content of the fuel. For most 
								//biomass feed-stocks this difference appears to be 6-7%.

								float MilesPerGallon = 0;

								if(Gasoline>0.0001)
									MilesPerGallon = pVehicle->m_Distance / max(0.0001,Gasoline);

								sub_path_emission_data.TotalMilesPerGallon += MilesPerGallon;

							}


							if(pass_mid_node_id && NodeName == to_node_id)
							{
								pass_to_node_id = true;
								ASSERT(j>0);
								end_node_id_timestamp = pVehicle->m_LinkAry [j].AbsArrivalTimeOnDSN;
								break;
							}


						}


						// step 2: check condition again for statitics testing

						if(pass_from_node_id && pass_mid_node_id && pass_to_node_id)
						{
							TripTimeSum += (end_node_id_timestamp - from_node_id_timestamp);
							DistanceSum+= total_path_distance;
							VehicleCount++;

							emission_data.TotalEnergy +=  sub_path_emission_data.TotalEnergy;
							emission_data.TotalCO2 +=  sub_path_emission_data.TotalCO2;
							emission_data.TotalCO +=  sub_path_emission_data.TotalCO;
							emission_data.TotalNOX +=  sub_path_emission_data.TotalNOX;
							emission_data.TotalHC +=  sub_path_emission_data.TotalHC;
							emission_data.TotalPM += sub_path_emission_data.TotalPM;
							emission_data.TotalPM2_5 += sub_path_emission_data.TotalPM2_5;
							emission_data.TotalMilesPerGallon += sub_path_emission_data.TotalMilesPerGallon;

						}

					} // condition on demand type... 
				} //condition on complete vehicles
			} // for each vehicle

			AvgTripTime = TripTimeSum/max(1,VehicleCount);  // unit: min
			AvgDistance = DistanceSum/max(1,VehicleCount);  // unit: mile
			AvgSpeed = AvgDistance/max(0.001,AvgTripTime)*60;  // unit: mph

			emission_data.AvgEnergy = emission_data.TotalEnergy/max(1,VehicleCount);
			emission_data.AvgCO2 = emission_data.TotalCO2/max(1,VehicleCount);
			emission_data.AvgCO = emission_data.TotalCO/max(1,VehicleCount);
			emission_data.AvgNOX = emission_data.TotalNOX/max(1,VehicleCount);
			emission_data.AvgHC = emission_data.TotalHC/max(1,VehicleCount);
			emission_data.AvgPM = emission_data.TotalPM / max(1, VehicleCount);
			emission_data.AvgPM2_5 = emission_data.TotalPM2_5 / max(1, VehicleCount);
			if (emission_data.AvgEnergy >0.00001)
			{
				emission_data.AvgMilesPerGallon = emission_data.TotalMiles /max(0.1,emission_data.TotalGasolineGallon );
			}
		}
	}
	return VehicleCount;
}


int g_TagAgents(float& AvgTripTime, float& AvgDistance, float& AvgSpeed, float & AvgTollCost, EmissionStatisticsData &emission_data, LinkMOEStatisticsData &link_data,
	int DemandType, int VehicleType, int InformationClass, int from_zone_id, int to_zone_id,
	int from_node_id, int mid_node_id, int to_node_id,
	int departure_starting_time, int departure_ending_time, int entrance_starting_time, int entrance_ending_time)
{
	int VehicleCount = 0;

	if (from_node_id == 0)  // not link or path MOE
	{
		for (vector<DTAVehicle*>::iterator v = g_VehicleVector.begin(); v != g_VehicleVector.end(); v++)
		{

			DTAVehicle* pVehicle = (*v);
			{
				if ((pVehicle->m_InformationType == InformationClass || InformationClass <= 0)
					&& (pVehicle->m_DemandType == DemandType || DemandType <= 0)
					&& (pVehicle->m_VehicleType == VehicleType || VehicleType <= 0)
					&& (pVehicle->m_OriginZoneID == from_zone_id || from_zone_id <= 0)
					&& (pVehicle->m_DestinationZoneID == to_zone_id || to_zone_id <= 0)
					&& ((int)(pVehicle->m_DepartureTime) >= departure_starting_time && (int)(pVehicle->m_DepartureTime < departure_ending_time) || (departure_starting_time == 0 && departure_ending_time == 0) || (departure_starting_time == 0 && departure_ending_time == 1440)))
				{

					pVehicle->m_bTag = true;  // tag all vehicles for this condition

				}
			}
		}


	}
	else
	{

		if (mid_node_id == 0)  // Link MOE
		{

			for (unsigned li = 0; li< g_LinkVector.size(); li++)
			{
				DTALink* pLink = g_LinkVector[li];

				if (pLink->m_FromNodeNumber == from_node_id && pLink->m_ToNodeNumber == to_node_id)
				{


					// scan through the list of vehicles on the link
					for (std::list<struc_vehicle_item>::iterator i = pLink->EntranceQueue.begin(); i != pLink->EntranceQueue.end(); ++i)
					{

						struc_vehicle_item vi = (*i);
						DTAVehicle* pVehicle = g_VehicleMap[vi.veh_id];

						if ((pVehicle->m_InformationType == InformationClass || InformationClass <= 0)
							&& (pVehicle->m_DemandType == DemandType || DemandType <= 0)
							&& (pVehicle->m_VehicleType == VehicleType || VehicleType <= 0)
							&& (pVehicle->m_OriginZoneID == from_zone_id || from_zone_id <= 0)
							&& (pVehicle->m_DestinationZoneID == to_zone_id || to_zone_id <= 0)
							&& ((int)(pVehicle->m_DepartureTime) >= departure_starting_time && (int)(pVehicle->m_DepartureTime < departure_ending_time) || (departure_starting_time == 0 && departure_ending_time == 0) || (departure_starting_time == 0 && departure_ending_time == 1440)))

						{
							pVehicle->m_bTag = true;  // tag all vehicles on this link
						}

					}

					break;
				} // for this link
			}
		}
		else  /////////////////////////////////// 3 point path
		{

			for (vector<DTAVehicle*>::iterator v = g_VehicleVector.begin(); v != g_VehicleVector.end(); v++)
			{

				DTAVehicle* pVehicle = (*v);
					if ((pVehicle->m_InformationType == InformationClass || InformationClass <= 0)
						&& (pVehicle->m_DemandType == DemandType || DemandType <= 0)
						&& (pVehicle->m_VehicleType == VehicleType || VehicleType <= 0)
						&& (pVehicle->m_OriginZoneID == from_zone_id || from_zone_id <= 0)
						&& (pVehicle->m_DestinationZoneID == to_zone_id || to_zone_id <= 0)
						&& ((int)(pVehicle->m_DepartureTime) >= departure_starting_time && (int)(pVehicle->m_DepartureTime < departure_ending_time) || (departure_starting_time == 0 && departure_ending_time == 0) || (departure_starting_time == 0 && departure_ending_time == 1440)))
					{
						//step 1: test three point condition


						bool pass_from_node_id = false;
						bool pass_mid_node_id = false;
						bool pass_to_node_id = false;
						float from_node_id_timestamp = 0;
						float end_node_id_timestamp = 0;

						int NodeID = g_LinkVector[pVehicle->m_LinkAry[0].LinkNo]->m_FromNodeID;  // first node
						int NodeName = g_NodeVector[NodeID].m_NodeNumber;
						float total_path_distance = 0;

						if (NodeName == from_node_id)
						{
							pass_from_node_id = true;
							from_node_id_timestamp = pVehicle->m_DepartureTime;
						}

						for (int j = 0; j< pVehicle->m_NodeSize - 1; j++)
						{
							int LinkID = pVehicle->m_LinkAry[j].LinkNo;
							NodeID = g_LinkVector[LinkID]->m_ToNodeID;
							NodeName = g_NodeVector[NodeID].m_NodeNumber;

							if (NodeName == from_node_id)
							{
								pass_from_node_id = true;

								if (j>0)
									from_node_id_timestamp = pVehicle->m_LinkAry[j - 1].AbsArrivalTimeOnDSN;
								else
									from_node_id_timestamp = pVehicle->m_DepartureTime;
							}

							if (pass_from_node_id && NodeName == mid_node_id)
							{
								pass_mid_node_id = true;
							}

							if (pass_from_node_id)  // start counting distance along the path
							{

							}


							if (pass_mid_node_id && NodeName == to_node_id)
							{
								pass_to_node_id = true;
								ASSERT(j>0);
								end_node_id_timestamp = pVehicle->m_LinkAry[j].AbsArrivalTimeOnDSN;
								break;
							}


						}


						// step 2: check condition again for statitics testing

						if (pass_from_node_id && pass_mid_node_id && pass_to_node_id)
						{
							pVehicle->m_bTag = true; 

						}

					} // condition on demand type... 
			} // for each vehicle

		}
	}
	return VehicleCount;
}

void g_GenerateSummaryStatisticsTable()
{

	int cl;

	CCSVParser parser_MOE_settings;
	if (!parser_MOE_settings.OpenCSVFile("optional_MOE_settings.csv",false))
	{
		cout << "File optional_MOE_settings.csv cannot be opened. Please check." << endl;
		return;
	}


	g_SummaryStatFile.Reset ();
	g_SummaryStatFile.SetRowTitle(true);

	int moe_group = 0;
	int prev_moe_group = -1;

	while(parser_MOE_settings.ReadRecord())
	{
		string moe_type, moe_category_label;


		parser_MOE_settings.GetValueByFieldName("moe_type",moe_type);
		parser_MOE_settings.GetValueByFieldName("moe_group",moe_group);
		parser_MOE_settings.GetValueByFieldName("moe_category_label",moe_category_label);

		cout << "  outputing simulation summary for MOE " << moe_group << " "<< moe_category_label <<endl;

		if( prev_moe_group !=  moe_group)  // new group
		{
			g_SummaryStatFile.Reset ();

			g_SummaryStatFile.WriteNewEndofLine ();
			g_SummaryStatFile.WriteNewEndofLine ();

			CString str_moe_group;
			str_moe_group.Format ("--MOE Group %d--",moe_group);
			g_SummaryStatFile.WriteTextString (str_moe_group);

			if(moe_type.compare("network_time_dependent") == 0)  // time-dependent 
			{
				g_SummaryStatFile.WriteTextString ("Detailed Data:,output_NetworkTDMOE.csv");

				OutputNetworkMOEData(g_SummaryStatFile.outFile);

				continue; // skip the other MOEs
			}


			if(moe_type.compare("link_critical") == 0)  // critical link list
			{

				g_SummaryStatFile.WriteTextString ("Detailed Data:,output_LinkMOE.csv");

				int cut_off_volume = 100;
				parser_MOE_settings.GetValueByFieldName("cut_off_volume",cut_off_volume);
				g_OutputLinkMOESummary(g_SummaryStatFile.outFile, cut_off_volume);

				continue; // skip the other MOEs
			}

			if(moe_type.compare("od_critical") == 0)  // critical OD list
			{

				g_SummaryStatFile.WriteTextString ("Detailed Data:,output_ODMOE.csv");

				int cut_off_volume = 50;
				parser_MOE_settings.GetValueByFieldName("cut_off_volume",cut_off_volume);

				if(cut_off_volume ==0)
					cut_off_volume = 1;  // set minimum cut off value as 1



				OutputODMOEData(g_SummaryStatFile.outFile,cut_off_volume,0);

			continue; // skip the other MOEs
			}


			g_SummaryStatFile.SetFieldName ("#_of_vehicles");
			g_SummaryStatFile.SetFieldName ("avg_distance");
			g_SummaryStatFile.SetFieldName ("avg_travel_time");
			g_SummaryStatFile.SetFieldName ("avg_speed");
			g_SummaryStatFile.SetFieldName ("avg_toll_cost");

			g_SummaryStatFile.SetFieldName("avg_energy_K_joule");
			g_SummaryStatFile.SetFieldName("avg_CO2_gram");
			g_SummaryStatFile.SetFieldName("avg_NOX_gram");
			g_SummaryStatFile.SetFieldName("avg_CO_gram");
			g_SummaryStatFile.SetFieldName("avg_HC_gram");
			g_SummaryStatFile.SetFieldName("avg_PM_gram");
			g_SummaryStatFile.SetFieldName("avg_PM2_5_gram");

			g_SummaryStatFile.SetFieldName("avg_energy_K_joule_per_mile");
			g_SummaryStatFile.SetFieldName("avg_CO2_gram_per_mile");
			g_SummaryStatFile.SetFieldName("avg_NOX_gram_per_mile");
			g_SummaryStatFile.SetFieldName("avg_CO_gram_per_mile");
			g_SummaryStatFile.SetFieldName("avg_HC_gram_per_mile");
			g_SummaryStatFile.SetFieldName("avg_PM_gram_per_mile");
			g_SummaryStatFile.SetFieldName("avg_PM2_5_gram_per_mile");

			g_SummaryStatFile.SetFieldName("avg_MilesPerGallon");

			if(moe_category_label.compare("link") == 0)  // Link MOE
			{

				g_SummaryStatFile.SetFieldName("SOV_volume");
				g_SummaryStatFile.SetFieldName("HOV_volume");
				g_SummaryStatFile.SetFieldName("Truck_volume");
				g_SummaryStatFile.SetFieldName("Intermodal_volume");

				g_SummaryStatFile.SetFieldName("number_of_crashes_per_year");
				g_SummaryStatFile.SetFieldName("number_of_fatal_and_injury_crashes_per_year");
				g_SummaryStatFile.SetFieldName("number_of_property_damage_only_crashes_per_year");

			}
		}

		if( prev_moe_group !=  moe_group)
		{
			g_SummaryStatFile.WriteHeader ();  // write header for a new group
			prev_moe_group = moe_group;
		}


		int demand_type =  0;
		int vehicle_type = 0;
		int information_type = 0;
		int from_node_id = 0;
		int mid_node_id	=0;
		int to_node_id	=0;
		int from_zone_id	=0;
		int to_zone_id	=0;
		int departure_starting_time	 = 0;
		int departure_ending_time= 1440;
		int entrance_starting_time	= 0;
		int entrance_ending_time = 1440;


		parser_MOE_settings.GetValueByFieldName("moe_type",moe_type);
		parser_MOE_settings.GetValueByFieldName("moe_category_label",moe_category_label);
		parser_MOE_settings.GetValueByFieldName("demand_type",demand_type);
		parser_MOE_settings.GetValueByFieldName("vehicle_type",vehicle_type);
		parser_MOE_settings.GetValueByFieldName("information_type",information_type);
		parser_MOE_settings.GetValueByFieldName("from_node_id",from_node_id);
		parser_MOE_settings.GetValueByFieldName("mid_node_id",mid_node_id);
		parser_MOE_settings.GetValueByFieldName("to_node_id",to_node_id);
		parser_MOE_settings.GetValueByFieldName("from_zone_id",from_zone_id);
		parser_MOE_settings.GetValueByFieldName("to_zone_id",to_zone_id);
		parser_MOE_settings.GetValueByFieldName("departure_starting_time",departure_starting_time);
		parser_MOE_settings.GetValueByFieldName("departure_ending_time",departure_ending_time);
		parser_MOE_settings.GetValueByFieldName("entrance_starting_time",entrance_starting_time);
		parser_MOE_settings.GetValueByFieldName("entrance_ending_time",entrance_ending_time);

		int Count=0; 
		float AvgTripTime = 0;
		float AvgDistance = 0;
		float AvgSpeed = 0;
		float AvgCost = 0;
		EmissionStatisticsData emission_data;
		LinkMOEStatisticsData  link_data;
		Count = g_OutputSimulationMOESummary(AvgTripTime,AvgDistance, AvgSpeed, AvgCost,  emission_data, link_data,
			demand_type,vehicle_type, information_type, from_zone_id,to_zone_id,
			from_node_id, mid_node_id, to_node_id,	
			departure_starting_time,departure_ending_time,entrance_starting_time,entrance_ending_time);

		float percentage = Count*100.0f/max(1,g_SimulationResult.number_of_vehicles);

		g_SummaryStatFile.SetValueByFieldName ("#_of_vehicles",Count );
		g_SummaryStatFile.SetValueByFieldName ("avg_travel_time",AvgTripTime);
		g_SummaryStatFile.SetValueByFieldName ("avg_distance",AvgDistance);
		g_SummaryStatFile.SetValueByFieldName ("avg_speed",AvgSpeed);
		g_SummaryStatFile.SetValueByFieldName ("avg_toll_cost",AvgCost);

		// prevent devided by zero error
		if(AvgDistance<0.01)
			AvgDistance = 0.01f;


		g_SummaryStatFile.SetValueByFieldName("avg_energy_K_joule",emission_data.AvgEnergy);
		g_SummaryStatFile.SetValueByFieldName("avg_CO2_gram",emission_data.AvgCO2);
		g_SummaryStatFile.SetValueByFieldName("avg_NOX_gram",emission_data.AvgNOX);
		g_SummaryStatFile.SetValueByFieldName("avg_CO_gram",emission_data.AvgCO);
		g_SummaryStatFile.SetValueByFieldName("avg_HC_gram",emission_data.AvgHC);
		g_SummaryStatFile.SetValueByFieldName("avg_PM_gram", emission_data.AvgPM);
		g_SummaryStatFile.SetValueByFieldName("avg_PM2_5_gram", emission_data.AvgPM2_5);

		float avg_energy_K_joule_per_mile = emission_data.AvgEnergy/AvgDistance ;
		g_SummaryStatFile.SetValueByFieldName("avg_energy_K_joule_per_mile",avg_energy_K_joule_per_mile);
		float avg_CO2_gram_per_mile = emission_data.AvgCO2/AvgDistance;
		g_SummaryStatFile.SetValueByFieldName("avg_CO2_gram_per_mile",avg_CO2_gram_per_mile);

		float avg_NOX_gram_per_mile = emission_data.AvgNOX/AvgDistance;
		g_SummaryStatFile.SetValueByFieldName("avg_NOX_gram_per_mile",avg_NOX_gram_per_mile);

		float avg_CO_gram_per_mile = emission_data.AvgCO/AvgDistance;
		g_SummaryStatFile.SetValueByFieldName("avg_CO_gram_per_mile",avg_CO_gram_per_mile);

		float avg_HC_gram_per_mile = emission_data.AvgHC/AvgDistance;
		g_SummaryStatFile.SetValueByFieldName("avg_HC_gram_per_mile",avg_HC_gram_per_mile);

		float avg_PM_gram_per_mile = emission_data.AvgPM / AvgDistance;
		g_SummaryStatFile.SetValueByFieldName("avg_PM_gram_per_mile", avg_PM_gram_per_mile);

		float avg_PM2_5_gram_per_mile = emission_data.AvgPM2_5 / AvgDistance;
		g_SummaryStatFile.SetValueByFieldName("avg_PM2_5_gram_per_mile", avg_PM2_5_gram_per_mile);

		g_SummaryStatFile.SetValueByFieldName("avg_MilesPerGallon", emission_data.AvgMilesPerGallon);



		if(moe_type.compare ("link")==0) // Link MOE
		{
			g_SummaryStatFile.SetValueByFieldName("SOV_volume",link_data.SOV_volume );
			g_SummaryStatFile.SetValueByFieldName("HOV_volume",link_data.HOV_volume );
			g_SummaryStatFile.SetValueByFieldName("Truck_volume",link_data.Truck_volume );
			g_SummaryStatFile.SetValueByFieldName("Intermodal_volume",link_data.Intermodal_volume );

			g_SummaryStatFile.SetValueByFieldName("number_of_crashes_per_year",link_data.number_of_crashes_per_year );
			g_SummaryStatFile.SetValueByFieldName("number_of_fatal_and_injury_crashes_per_year",link_data.number_of_fatal_and_injury_crashes_per_year);
			g_SummaryStatFile.SetValueByFieldName("number_of_property_damage_only_crashes_per_year",link_data.number_of_property_damage_only_crashes_per_year );

		}

		TRACE("%s\n",moe_category_label.c_str ());
		if(moe_category_label.find (",") !=  string::npos)
		{
			moe_category_label = '"' + moe_category_label + '"' ;
		}

		string str = moe_category_label + ",";

		g_SummaryStatFile.WriteTextLabel (str.c_str ());

		g_SummaryStatFile.WriteRecord ();

	}


	parser_MOE_settings.CloseCSVFile ();

}
void g_OutputSimulationStatistics(int Iteration)
{

	std::vector<int> MOE_time_interval_vector;
	MOE_time_interval_vector.push_back (60);
	MOE_time_interval_vector.push_back (30);
	MOE_time_interval_vector.push_back (15);
	MOE_time_interval_vector.push_back (5);

	for(int MOE_interval_index = 0; MOE_interval_index <MOE_time_interval_vector.size(); MOE_interval_index++)
	{

		int time_interval = MOE_time_interval_vector[MOE_interval_index];

		CString title_str;
		title_str.Format ("\n--Time Dependent Network MOE--,%d min interval", time_interval);
		g_SummaryStatFile.WriteTextString(title_str);
		g_SummaryStatFile.Reset();
		g_SummaryStatFile.SetFieldName("time_stamp_in_min");
		g_SummaryStatFile.SetFieldName("time_clock");
		g_SummaryStatFile.SetFieldName("cumulative_in_flow_count");
		g_SummaryStatFile.SetFieldName("cumulative_out_flow_count");
		g_SummaryStatFile.SetFieldName("number_vehicles_in_network");
		g_SummaryStatFile.SetFieldName("number_vehicles_entering_network");
		g_SummaryStatFile.SetFieldName("total_vehicle_mile_traveled");
		g_SummaryStatFile.SetFieldName("avg_trip_time (departure_time_based)");
		g_SummaryStatFile.SetFieldName("avg_trip_time_index = trip time/FFTT");
		g_SummaryStatFile.SetFieldName("avg_distance");
		g_SummaryStatFile.SetFieldName("avg_speed");

		g_SummaryStatFile.WriteHeader (false,false);

		int start_time  = (int)(g_DemandLoadingStartTimeInMin/time_interval)*time_interval;

		for(int time = start_time; time< min( g_NetworkMOEAry.size(), g_PlanningHorizon);time++)
		{

			if(time >=1)
			{
				// deal with interval without data
				if(g_NetworkMOEAry[time].CumulativeInFlow < g_NetworkMOEAry[time-1].CumulativeInFlow)
					g_NetworkMOEAry[time].CumulativeInFlow = g_NetworkMOEAry[time-1].CumulativeInFlow;

				if(g_NetworkMOEAry[time].CumulativeOutFlow < g_NetworkMOEAry[time-1].CumulativeOutFlow)
					g_NetworkMOEAry[time].CumulativeOutFlow = g_NetworkMOEAry[time-1].CumulativeOutFlow;
			}


			if(time%time_interval==0)
			{
				g_SummaryStatFile.SetValueByFieldName ("time_stamp_in_min", time);
				std::string time_str = GetTimeClockString(time);
				g_SummaryStatFile.SetValueByFieldName ("time_clock", time_str);
				g_SummaryStatFile.SetValueByFieldName ("cumulative_in_flow_count", g_NetworkMOEAry[time].CumulativeInFlow);
				g_SummaryStatFile.SetValueByFieldName ("cumulative_out_flow_count", g_NetworkMOEAry[time].CumulativeOutFlow);

				int previous_time_interval = 0;
				if (time > start_time)
					previous_time_interval = time - time_interval;

				int count_previous_time_interval = g_NetworkMOEAry[previous_time_interval].CumulativeInFlow;

				int number_vehicles_entering_network = g_NetworkMOEAry[time].CumulativeInFlow - count_previous_time_interval;

				g_SummaryStatFile.SetValueByFieldName("number_vehicles_entering_network", number_vehicles_entering_network);
				
				int number_vehicles_in_network = g_NetworkMOEAry[time].CumulativeInFlow - g_NetworkMOEAry[time].CumulativeOutFlow;
				g_SummaryStatFile.SetValueByFieldName ("number_vehicles_in_network", number_vehicles_in_network);

				int arrival_flow = 0;
				float total_trip_time = 0;
				float total_free_flow_trip_time = 0;
				float total_distance = 0;
				float total_waiting_time_at_origin = 0;

				for (int tt = time; tt< min (g_NetworkMOEAry.size(), time + time_interval); tt++)
				{
					arrival_flow+= g_NetworkMOEAry[tt].Flow_in_a_min;
					total_trip_time+= g_NetworkMOEAry[tt].AbsArrivalTimeOnDSN_in_a_min;
					total_free_flow_trip_time += g_NetworkMOEAry[tt].TotalFreeFlowTravelTime;
					total_distance += g_NetworkMOEAry[tt].TotalDistance;
					total_waiting_time_at_origin += g_NetworkMOEAry[tt].TotalBufferWaitingTime;

				}

				float avg_trip_time = total_trip_time/max(1,arrival_flow);
				float avg_trip_time_index = total_trip_time/max(0.001,total_free_flow_trip_time);

				float avg_distance = total_distance/max(1,arrival_flow);
				float avg_speed = total_distance/max(1,total_trip_time)*60;

				g_SummaryStatFile.SetValueByFieldName ("total_in_flow", arrival_flow);
				g_SummaryStatFile.SetValueByFieldName ("avg_trip_time (departure_time_based)", avg_trip_time);
				g_SummaryStatFile.SetValueByFieldName ("avg_trip_time_index = trip time/FFTT", avg_trip_time_index);

				g_SummaryStatFile.SetValueByFieldName ("total_vehicle_mile_traveled", total_distance);
				g_SummaryStatFile.SetValueByFieldName("avg_distance",avg_distance);
				g_SummaryStatFile.SetValueByFieldName("avg_speed",avg_speed);

				g_SummaryStatFile.WriteRecord ();
			}


			if((g_NetworkMOEAry[time].CumulativeInFlow - g_NetworkMOEAry[time].CumulativeOutFlow) == 0)
				break; // stop, as all vehicles leave. 
		}

	}
	g_SummaryStatFile.WriteTextString("Unit of output:");
	g_SummaryStatFile.WriteTextString("travel time=,min");
	g_SummaryStatFile.WriteTextString("travel Cost=,dollar");
	g_SummaryStatFile.WriteTextString("distance=,miles");
	g_SummaryStatFile.WriteTextString("speed=,mph");
	g_SummaryStatFile.WriteTextString("energy=,1000 joule");
	g_SummaryStatFile.WriteTextString("CO2=,gram");

	g_GenerateSummaryStatisticsTable();

	int NumberOfLinks = 9999999;

	ofstream LinkMOESummaryFile;

	LinkMOESummaryFile.open ("output_LinkMOE.csv", ios::out);
	if (LinkMOESummaryFile.is_open())
	{
		g_OutputLinkMOESummary(LinkMOESummaryFile);  // output assignment results anyway
		
		LinkMOESummaryFile.close();



	}else
	{
		cout << "Error: File output_LinkMOE.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();

	}

	//g_OutputLinkOutCapacitySummary();


	//	g_OutputSummaryKML(MOE_crashes);
	//	g_OutputSummaryKML(MOE_CO2);
	//	g_OutputSummaryKML(MOE_total_energy);

	EmissionStatisticsData emission_data;
	int Count=0; 
	float AvgTripTime, AvgDistance, AvgSpeed, AvgCost;
	g_LogFile << "--- MOE for vehicles completing trips ---" << endl;

	Count = g_OutputSimulationSummary(AvgTripTime, AvgDistance, AvgSpeed, AvgCost, emission_data, -1, -1, -1,-1);
	g_LogFile << " # of Vehicles = " << Count << " AvgTripTime = " << AvgTripTime << " (min), AvgDistance = " << AvgDistance << " , AvgSpeed =  " << AvgSpeed << " " <<  endl;

	g_LogFile << "--- MOE for Each Information Class ---" << endl;

	Count = g_OutputSimulationSummary(AvgTripTime, AvgDistance, AvgSpeed, AvgCost, emission_data, 1, -1, -1,-1);
	if(Count>0)
		g_LogFile << "Hist Knowledge (VMS non-responsive): # of Vehicles = " << Count << " ,AvgTripTime = " << AvgTripTime << " (min), AvgDistance = " << AvgDistance << " , AvgSpeed =  " << AvgSpeed << " " << endl;

	Count = g_OutputSimulationSummary(AvgTripTime, AvgDistance, AvgSpeed, AvgCost, emission_data, 2, -1, -1,-1);
	if(Count>0)
		g_LogFile << "Pre-trip Info                      : # of Vehicles = " << Count << " ,AvgTripTime = " << AvgTripTime << " (min), AvgDistance = " << AvgDistance << " , AvgSpeed =  " << AvgSpeed << " " <<  endl;

	Count = g_OutputSimulationSummary(AvgTripTime, AvgDistance, AvgSpeed, AvgCost, emission_data, 3, -1, -1,-1);
	if(Count>0)
		g_LogFile << "En-route Info                      : # of Vehicles = " << Count << " ,AvgTripTime = " << AvgTripTime << " (min), AvgDistance = " << AvgDistance << " , AvgSpeed =  " << AvgSpeed << " " << endl;

	Count = g_OutputSimulationSummary(AvgTripTime, AvgDistance, AvgSpeed, AvgCost,  emission_data, 4, -1, -1,-1);
	if(Count>0)
		g_LogFile << "Hist Knowledge (VMS responsive)    : # of Vehicles = " << Count << " ,AvgTripTime = " << AvgTripTime << " (min), AvgDistance = " << AvgDistance << " , AvgSpeed =  " << AvgSpeed << " " <<  endl;

	g_LogFile << endl  << "--- MOE for Each Vehicle Type ---" << endl;

	Count = g_OutputSimulationSummary(AvgTripTime, AvgDistance, AvgSpeed, AvgCost,  emission_data, -1, 1, -1,-1);
	if(Count>0)
		g_LogFile << "SOV Passenger Car: # of Vehicles = " << Count << " ,AvgTripTime = " << AvgTripTime << " (min), AvgDistance = " << AvgDistance << " , AvgSpeed =  " << AvgSpeed << " " <<  endl;

	Count = g_OutputSimulationSummary(AvgTripTime, AvgDistance, AvgSpeed, AvgCost,  emission_data, -1, 2, -1,-1);
	if(Count>0)
		g_LogFile << "HOV Passenger Car: # of Vehicles = " << Count << " ,AvgTripTime = " << AvgTripTime << " (min), AvgDistance = " << AvgDistance << " , AvgSpeed =  " << AvgSpeed << " " <<  endl;

	Count = g_OutputSimulationSummary(AvgTripTime, AvgDistance, AvgSpeed, AvgCost,  emission_data, -1, 3, -1,-1);
	if(Count>0)
		g_LogFile << "Truck            : # of Vehicles = " << Count << " ,AvgTripTime = " << AvgTripTime << " (min), AvgDistance = " << AvgDistance << " , AvgSpeed =  " << AvgSpeed << " " <<  endl;

	g_LogFile << endl << "--- MOE for Each Departure Time Interval ---" << endl;

	for (int departure_time_index = 0; departure_time_index < g_NumberOfSPCalculationPeriods; departure_time_index++)
	{

		int departure_time = g_AssignmentIntervalStartTimeInMin[departure_time_index];

		Count = g_OutputSimulationSummary(AvgTripTime, AvgDistance, AvgSpeed, AvgCost, emission_data, -1, -1, -1, departure_time_index);
		if(Count>0)
			g_LogFile << "Time:" << departure_time << " min, # of Vehicles = " << Count << " ,AvgTripTime = " << AvgTripTime << " (min), AvgDistance = " << AvgDistance << " , AvgSpeed =  " << AvgSpeed << " " <<  endl;

	}
	g_LogFile << endl;

	//// output general link statistics
	//std::set<DTALink*>::iterator iterLink;
	//g_LogFile << "--- Link MOE ---" << endl;

	//for(unsigned li = 0; li< min(NumberOfLinks,g_LinkVector.size()); li++)
	//{
	//	float Capacity = g_LinkVector[li]->m_LaneCapacity * g_LinkVector[li]->m_OutflowNumLanes;
	//	g_LogFile << "Link: " <<  g_NodeVector[g_LinkVector[li]->m_FromNodeID].m_NodeNumber  << " -> " << g_NodeVector[g_LinkVector[li]->m_ToNodeID].m_NodeNumber << 
	//		", Link Capacity: " << Capacity <<
	//		", Inflow: " << g_LinkVector[li]->CFlowArrivalCount <<
	//		//				" Outflow: " << g_LinkVector[li]->CFlowDepartureCount <<
	//		", VOC Ratio: " << g_LinkVector[li]->CFlowDepartureCount /max(1,Capacity)  << endl;
	//}

	//g_LogFile << g_GetAppRunningTime()  <<" ---End of Link MOE ---" << endl;

}


void g_OutputLinkMOESummary(ofstream &LinkMOESummaryFile, int cut_off_volume)
{
	LinkMOESummaryFile.precision(4) ;
	LinkMOESummaryFile.setf(ios::fixed);

	LinkMOESummaryFile << "from_node_id,to_node_id,type_code,link_type,link_length,start_time_in_min,end_time_in_min,total_link_volume,lane_capacity_in_vhc_per_hour,link_capacity_in_vhc_per_hour,volume_over_capacity_ratio,";
	LinkMOESummaryFile << "avg_waiting_time_on_loading_buffer_(min), speed_limit,speed,free-flow_travel_time,travel_time,percentage_of_speed_limit, level_of_service, sensor_data_flag, sensor_link_volume, ";
	LinkMOESummaryFile << "simulated_link_volume, measurement_error, abs_measurement_error_percentage, simulated_AADT, TotalEnergy_(J / hr), CO2_(g / hr), NOX_(g / hr), CO_(g / hr), HC_(g / hr, PM_(g / hr), PM_2_5(g / hr),";
	LinkMOESummaryFile << "sov_volume, hov_volume, truck_volume, intermodal_volume,";

	for (int i = 0; i < g_VehicleTypeVector.size(); i++)
	{

		LinkMOESummaryFile << g_VehicleTypeVector[i].vehicle_type_name.c_str() << ",";
	}

	LinkMOESummaryFile << "geometry, ";

	//DTASafetyPredictionModel SafePredictionModel;
	//SafePredictionModel.UpdateCrashRateForAllLinks();

	LinkMOESummaryFile << "from_node_id,to_node_id,";
	int d;

	//hour by hour
	int h;
	//volume
	for (h = 0; h <= 23; h++)
	{
		LinkMOESummaryFile << "vol_h" << h + 1 << ",";
	}

	//speed
	for (h = 0; h <= 23; h++)
	{
		LinkMOESummaryFile << "speed_h" << h + 1 << ",";
	}

	// day by day

	//volume
	for(d=0; d <=g_NumberOfIterations; d++)
	{
		LinkMOESummaryFile << "vol_d" << d+1 << ",";
	}

	//speed
	for(d=0; d <=g_NumberOfIterations; d++)
	{
		LinkMOESummaryFile << "spd_d" << d+1 << ",";
	}
	LinkMOESummaryFile << endl;

	std::set<DTALink*>::iterator iterLink;

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{

		DTALink* pLink = g_LinkVector[li];

		double average_travel_time = pLink->GetTravelTimeByMin(g_NumberOfIterations,0, pLink->m_SimulationHorizon,g_TrafficFlowModelFlag);
		double speed = pLink->m_Length / max(0.00001,average_travel_time) *60;  // unit: mph
		
		double capacity_simulation_horizon = pLink->m_LaneCapacity * pLink->m_OutflowNumLanes;
		
		if(g_NodeVector[pLink->m_FromNodeID].m_NodeNumber  == 30 && g_NodeVector[pLink->m_ToNodeID].m_NodeNumber  == 31)
		{
		
		TRACE("");
		}


		double voc_ratio = pLink->CFlowArrivalCount /max(1,(g_DemandLoadingEndTimeInMin - g_DemandLoadingStartTimeInMin )/60.0) /  max(0.1,capacity_simulation_horizon);
		int percentage_of_speed_limit = int(speed/max(0.1,pLink->m_SpeedLimit)*100+0.5);

		if(pLink->CFlowArrivalCount >= cut_off_volume)
		{
			LinkMOESummaryFile << g_NodeVector[pLink->m_FromNodeID].m_NodeNumber  << "," ;
			LinkMOESummaryFile << g_NodeVector[pLink->m_ToNodeID].m_NodeNumber << "," ;


			LinkMOESummaryFile << g_LinkTypeMap[pLink->m_link_type].type_code.c_str ()<< "," ;

			LinkMOESummaryFile << g_LinkTypeMap[pLink->m_link_type].link_type_name .c_str () << ",";
			LinkMOESummaryFile << pLink->m_Length << ",";

			LinkMOESummaryFile << g_DemandLoadingStartTimeInMin << "," ;
			LinkMOESummaryFile << g_DemandLoadingEndTimeInMin << "," ;
			LinkMOESummaryFile << pLink->CFlowArrivalCount << "," ; // total hourly arrival flow 
			LinkMOESummaryFile << pLink->m_LaneCapacity << "," ;
			LinkMOESummaryFile << capacity_simulation_horizon << ",";
			LinkMOESummaryFile << voc_ratio << "," ;

			pLink->m_LoadingBufferWaitingTime = max(0, pLink->m_LoadingBufferWaitingTime);

			LinkMOESummaryFile << pLink->m_LoadingBufferWaitingTime / max(1, pLink->CFlowArrivalCount) << "," ;


			LinkMOESummaryFile << pLink->m_SpeedLimit << "," ;
			LinkMOESummaryFile << speed << "," ;
			LinkMOESummaryFile << pLink->m_FreeFlowTravelTime << ",";
			
			LinkMOESummaryFile << average_travel_time << ",";
			LinkMOESummaryFile << percentage_of_speed_limit << "," ;
			LinkMOESummaryFile << g_GetLevelOfService(percentage_of_speed_limit) << "," ;
			LinkMOESummaryFile << pLink->m_bSensorData << "," ;
			LinkMOESummaryFile << pLink->m_ObservedFlowVolume << "," ;

			float error_percentage;

			if( pLink->m_bSensorData)
				error_percentage = (pLink->CFlowArrivalCount - pLink->m_ObservedFlowVolume) / max(1,pLink->m_ObservedFlowVolume) *100;
			else
				error_percentage  = 0;

			float error;
			if( pLink->m_bSensorData)
				error = (pLink->CFlowArrivalCount - pLink->m_ObservedFlowVolume);
			else
				error  = 0;
			LinkMOESummaryFile << pLink->CFlowArrivalCount << "," ; // total hourly arrival flow 
			LinkMOESummaryFile << error << "," ;
			LinkMOESummaryFile << fabs(error_percentage) << "," ;

			LinkMOESummaryFile << pLink->m_AADT << "," ;

			LinkMOESummaryFile << pLink->TotalEnergy  << "," ;
			LinkMOESummaryFile << pLink->TotalCO2  << "," ;
			LinkMOESummaryFile << pLink->TotalNOX  << "," ;
			LinkMOESummaryFile << pLink->TotalCO  << "," ;
			LinkMOESummaryFile << pLink->TotalHC  << "," ;
			LinkMOESummaryFile << pLink->TotalPM << ",";
			LinkMOESummaryFile << pLink->TotalPM2_5 << ",";

			LinkMOESummaryFile << pLink->CFlowArrivalCount_DemandType[1]  << "," ;
			LinkMOESummaryFile << pLink->CFlowArrivalCount_DemandType[2]  << "," ;
			LinkMOESummaryFile << pLink->CFlowArrivalCount_DemandType[3]  << "," ;
			LinkMOESummaryFile << pLink->CFlowArrivalCount_DemandType[4]  << "," ;


			for (int i = 0; i < g_VehicleTypeVector.size(); i++)
			{
				int vehicle_type = g_VehicleTypeVector[i].vehicle_type; 
				LinkMOESummaryFile << pLink->CFlowArrivalCount_VehicleType[vehicle_type] << ",";
			}
			// geometry
			LinkMOESummaryFile << "\"" << pLink->m_original_geometry_string << "\"" << ",";

			LinkMOESummaryFile << g_NodeVector[pLink->m_FromNodeID].m_NodeNumber  << "," ;
			LinkMOESummaryFile << g_NodeVector[pLink->m_ToNodeID].m_NodeNumber << "," ;


				//volume
			for (h = 0; h <= 23; h++)
				{
				int StartTime = h * 60;
				int EndTime = (h + 1) * 60;

					float count = 0;
					if (EndTime <= pLink->m_LinkMOEAry.size())
					{
						count = pLink->m_LinkMOEAry[EndTime].CumulativeDepartureCount - pLink->m_LinkMOEAry[StartTime].CumulativeDepartureCount;

						if (count < 0)
							count = 0;
					}
					
					LinkMOESummaryFile << count << ",";
				}

				//speed
			for (h = 0; h <= 23; h++)
				{
				int StartTime = h * 60;
				int EndTime = (h + 1) * 60; \


				float travel_time_in_hour = pLink->GetTravelTimeByMin(-1, StartTime, 60, g_TrafficFlowModelFlag)/60.0;

				float speed = pLink->m_Length / max(0.0001, travel_time_in_hour);

				LinkMOESummaryFile << speed << ",";
			}

		////	if(0)  // commend out day to day 
		//	{
		//	//volume
		//	for(d=0; d < pLink->m_Day2DayLinkMOEVector.size(); d++)
		//	{
		//		LinkMOESummaryFile << pLink->m_Day2DayLinkMOEVector [d].TotalFlowCount << ",";
		//	}

		//	//speed
		//	for(d=0; d < pLink->m_Day2DayLinkMOEVector.size(); d++)
		//	{
		//		LinkMOESummaryFile << pLink->m_Day2DayLinkMOEVector [d].AvgSpeed  << ",";
		//	}
		//	}

			LinkMOESummaryFile << endl;	

		}
	}


}

void g_OutputLinkOutCapacitySummary()
{
	ofstream LinkOutCapcaityFile;

	LinkOutCapcaityFile.open ("debug_LinkCapacity.csv", ios::out);

	cout << "outputing file debug_LinkCapacity.csv ..." << endl;
	LinkOutCapcaityFile.precision(1) ;
	LinkOutCapcaityFile.setf(ios::fixed);

	std::set<DTALink*>::iterator iterLink;
	bool bTitlePrintOut = false;

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{

		unsigned int t;

		DTALink* pLink = g_LinkVector[li];

		if(pLink->m_OutCapacityVector.size() > 0 )
		{
			if(!bTitlePrintOut)
			{
				LinkOutCapcaityFile << "time_stamp_in_min" << "," ;

				for(t = 0; t < pLink->m_OutCapacityVector.size(); t++)
				{
					LinkOutCapcaityFile << pLink->m_OutCapacityVector[t].time_stamp_in_min << "," ;
				}
				bTitlePrintOut = true;
				LinkOutCapcaityFile << endl;	
			}

			LinkOutCapcaityFile << "link outflow capacity " << pLink->m_FromNodeNumber << "->" << pLink->m_ToNodeNumber << ",";
			for( t = 0; t < pLink->m_OutCapacityVector.size(); t++)
			{
				LinkOutCapcaityFile << pLink->m_OutCapacityVector[t].out_capacity_in_vehicle_number << "," ;
			}
			LinkOutCapcaityFile << endl;	

			LinkOutCapcaityFile << "link queue " << pLink->m_FromNodeNumber << "->" << pLink->m_ToNodeNumber << ",";
			for( t = 0; t < pLink->m_OutCapacityVector.size(); t++)
			{
				LinkOutCapcaityFile << pLink->m_OutCapacityVector[t].link_queue_size  << "," ;
			}
			LinkOutCapcaityFile << endl;	



			LinkOutCapcaityFile << "link left outflow capacity " << pLink->m_FromNodeNumber << "->" << pLink->m_ToNodeNumber << ",";
			for( t = 0; t < pLink->m_OutCapacityVector.size(); t++)
			{
				LinkOutCapcaityFile << pLink->m_OutCapacityVector[t].out_left_capacity_in_vehicle_number << "," ;
			}
			LinkOutCapcaityFile << endl;	

			LinkOutCapcaityFile << "link left queue " << pLink->m_FromNodeNumber << "->" << pLink->m_ToNodeNumber << ",";
			for( t = 0; t < pLink->m_OutCapacityVector.size(); t++)
			{
				LinkOutCapcaityFile << pLink->m_OutCapacityVector[t].link_left_queue_size  << "," ;
			}
			LinkOutCapcaityFile << endl;	

			LinkOutCapcaityFile << "link blocking count " << pLink->m_FromNodeNumber << "->" << pLink->m_ToNodeNumber << ",";
			for( t = 0; t < pLink->m_OutCapacityVector.size(); t++)
			{
				LinkOutCapcaityFile << pLink->m_OutCapacityVector[t].blocking_count   << "," ;
			}
			LinkOutCapcaityFile << endl;	

			LinkOutCapcaityFile << "link blocking node number " << pLink->m_FromNodeNumber << "->" << pLink->m_ToNodeNumber << ",";
			for( t = 0; t < pLink->m_OutCapacityVector.size(); t++)
			{
				LinkOutCapcaityFile << pLink->m_OutCapacityVector[t].blocking_node_number   << "," ;
			}
			LinkOutCapcaityFile << endl;	
		}
	}

	LinkOutCapcaityFile.close();
}

void g_Output2WayLinkMOESummary(ofstream &LinkMOESummaryFile, int cut_off_volume)
{
	LinkMOESummaryFile.precision(4) ;
	LinkMOESummaryFile.setf(ios::fixed);

	LinkMOESummaryFile << "link_id,from_node_id,to_node_id,type_code,start_time_in_min,end_time_in_min,AB_total_link_volume,BA_total_link_volume" << endl;

	std::set<DTALink*>::iterator iterLink;

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{

		DTALink* pLink = g_LinkVector[li];

		double AB_flow = pLink->CFlowArrivalCount;

		double BA_flow = 0;

		string BA_link_string = GetLinkStringID( pLink->m_ToNodeNumber,pLink->m_FromNodeNumber);
		if(g_LinkMap.find(BA_link_string)!= g_LinkMap.end())
		{
			DTALink* pLinkBA = g_LinkMap[BA_link_string];

			BA_flow = pLinkBA->CFlowArrivalCount;

		}

		pLink->CFlowArrivalCount;

		double average_travel_time = pLink->GetTravelTimeByMin(g_NumberOfIterations,0, pLink->m_SimulationHorizon,g_TrafficFlowModelFlag);
		double speed = pLink->m_Length / max(0.00001,average_travel_time) *60;  // unit: mph
		double capacity_simulation_horizon = pLink->m_LaneCapacity * pLink->m_OutflowNumLanes;
		double voc_ratio = pLink->CFlowArrivalCount / max(0.1,capacity_simulation_horizon);
		int percentage_of_speed_limit = int(speed/max(0.1,pLink->m_SpeedLimit)*100+0.5);

		if(pLink->m_link_code ==1)  // AB
		{
			LinkMOESummaryFile << pLink->m_OrgLinkID    << "," ;
			LinkMOESummaryFile << g_NodeVector[pLink->m_FromNodeID].m_NodeNumber  << "," ;
			LinkMOESummaryFile << g_NodeVector[pLink->m_ToNodeID].m_NodeNumber << "," ;
			LinkMOESummaryFile << g_LinkTypeMap[pLink->m_link_type].type_code.c_str ()<< "," ;
			LinkMOESummaryFile << g_DemandLoadingStartTimeInMin << "," ;
			LinkMOESummaryFile << g_DemandLoadingEndTimeInMin << "," ;
			LinkMOESummaryFile << pLink->CFlowArrivalCount << "," ; // total hourly arrival flow 
			LinkMOESummaryFile << BA_flow << "," ; // total hourly arrival flow 
			LinkMOESummaryFile << endl;

		}
	}

}

char g_get_vehicle_complete_flag(bool flag)
{
	if (flag == true)
		return 'c';
	else
		return 'i';

}

void OutputVehicleTrajectoryData(std::string fname_agent, std::string  fname_trip, int Iteration, bool bStartWithEmpty, bool bIncremental)
{

	FILE* st_agent = NULL;
	FILE* st_agent_compact = NULL;

	FILE* st_struct = NULL;
	FILE* st_path_link_sequence = NULL;


	fopen_s(&st_agent,fname_agent.c_str(),"w");
	fopen_s(&st_agent_compact, fname_trip.c_str(), "w");

	if(st_agent_compact==NULL)
	{
		cout << "File " << fname_trip << " cannot be opened." <<endl;
		g_ProgramStop();

	}

	int err = 0;
	if (g_VehicleLoadingMode == vehicle_binary_file_mode )
	{
		err = fopen_s(&st_struct, "agent_scenario.bin", "wb");
		if (err > 0)
		{
			cout << "The file 'agent_scenario.bin' was not opened\n";
			g_ProgramStop();
		}
	}
	else
	{
		err = fopen_s(&st_struct, "agent.bin", "wb");

		if (err > 0)
	{
			cout << "The file 'agent.bin' was not opened\n";

			g_ProgramStop();
	}

		DeleteFile("agent_scenario.bin");  // remove agent_scenario.bin as the final result is agent.bin

	}


	int output_path_sequence_flag = 1;// g_GetPrivateProfileInt("output", "path_sequence_flag", 1, g_DTASettingFileName);

	if(st_agent!=NULL)
	{
		std::map<int, DTAVehicle*>::iterator iterVM;
		int VehicleCount_withPhysicalPath = 0;
		// output statistics
		fprintf(st_agent, "agent_id,tour_id,dependency_agent_id,duration_in_min,following_agent_id,from_zone_id, to_zone_id, from_origin_node_id, to_destination_node_id, departure_time_in_min, arrival_time_in_min, complete_flag, travel_time_in_min, demand_type, vehicle_type, PCE, information_type, value_of_time, toll_cost, distance, TotalEnergy_(KJ), CO2_(g), NOX_(g), CO_(g), HC_(g), PM_(g), PM_2.5(g),vehicle_age,number_of_nodes,path_node_sequence,org_path_node_sequence,path_time_sequence,origin_node_x,origin_node_y,destination_node_x,destination_node_y,\n");
		fprintf(st_agent_compact, "agent_id,tour_id,from_zone_id,to_zone_id,from_origin_node_id,to_destination_node_id,departure_time_in_min,end_time_in_min,travel_time_in_min,demand_type,information_type,value_of_time,vehicle_type,vehicle_age,distance\n");

	for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{

			DTAVehicle* pVehicle = iterVM->second;

			if(pVehicle->m_AgentID == 2)
				TRACE("");


			if (pVehicle->m_NodeSize >= 2)  // with physical path in the network
			{

				
				if(bIncremental && pVehicle->m_bComplete && pVehicle->b_already_output_flag)  //skip output this vehicle, only under incremental mode, and this vehicle has completed its trip and being output previously in a file
					continue;


				int UpstreamNodeID = 0;
				int DownstreamNodeID = 0;

				int LinkID_0 = pVehicle->m_LinkAry [0].LinkNo;
				UpstreamNodeID= g_LinkVector[LinkID_0]->m_FromNodeID;
				DownstreamNodeID = g_LinkVector[LinkID_0]->m_ToNodeID;

				float TripTime = 0;

				if(pVehicle->m_bComplete)
					TripTime = max(0,pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime);


				float m_gap = 0;
				fprintf(st_agent, "%d,%d,%d,%.0f,%d,%d,%d,%d,%d,%4.2f,%4.2f,%c,%4.2f,%d,%d,%4.2f,%d,%4.2f,%4.3f,%4.3f,%f,%f,%f,%f,%f,%f,%f,%d,%d,",
					pVehicle->m_AgentID,
					pVehicle->m_ExternalTourID,
					pVehicle->m_dependency_agent_id,
					pVehicle->m_duration_in_min,
					pVehicle->m_following_agent_id,
					pVehicle->m_OriginZoneID,
					pVehicle->m_DestinationZoneID,
					g_NodeVector[pVehicle->m_OriginNodeID].m_NodeNumber,
					g_NodeVector[pVehicle->m_DestinationNodeID].m_NodeNumber,
					pVehicle->m_DepartureTime,
					pVehicle->m_ArrivalTime,
					g_get_vehicle_complete_flag(pVehicle->m_bComplete),
					TripTime,
					pVehicle->m_DemandType,
					pVehicle->m_VehicleType,
					pVehicle->m_PCE,
					pVehicle->m_InformationType, 
					pVehicle->m_VOT ,
					pVehicle->m_TollDollarCost, 
					pVehicle->m_Distance, 
					pVehicle->Energy ,
					pVehicle->CO2, pVehicle->NOX, pVehicle->CO, pVehicle->HC, pVehicle->PM, pVehicle->PM2_5,
					
					pVehicle->m_Age,
					pVehicle->m_NodeSize);

				fprintf(st_agent_compact,"%d,%d,%d,%d,%d,%d,%4.2f,%4.2f,%4.2f,%d,%d,%4.2f,%d,%d,%4.2f,",
					pVehicle->m_AgentID , pVehicle->m_AgentID  , pVehicle->m_OriginZoneID , pVehicle->m_DestinationZoneID,
					g_NodeVector[pVehicle->m_OriginNodeID].m_NodeNumber ,
					g_NodeVector[pVehicle->m_DestinationNodeID].m_NodeNumber ,
					pVehicle->m_DepartureTime, 

					pVehicle->m_ArrivalTime ,
					pVehicle->m_ArrivalTime - pVehicle->m_DepartureTime,
					pVehicle->m_DemandType, 
					pVehicle->m_InformationType, 
					pVehicle->m_VOT ,pVehicle->m_VehicleType, pVehicle->m_Age,
					pVehicle->m_Distance );




				struct_VehicleInfo_Header header;
				header.day_no = Iteration ; // Iteration starts from 0
				header.vehicle_id = pVehicle->m_AgentID;
				header.from_zone_id = pVehicle->m_OriginZoneID;
				header. to_zone_id = pVehicle->m_DestinationZoneID;
				header. departure_time = pVehicle->m_DepartureTime;
				header. arrival_time = pVehicle->m_ArrivalTime;
				header. complete_flag = pVehicle->m_bComplete;
				header. trip_time = TripTime;
				header. demand_type = pVehicle->m_DemandType;
				header.agent_type = pVehicle->m_DemandType;
				header. vehicle_type =pVehicle->m_VehicleType;
				header. information_type = pVehicle->m_InformationType;
				header. value_of_time =pVehicle->m_VOT;
				header. toll_cost_in_dollar = pVehicle->m_TollDollarCost;
				header. PM = pVehicle->PM;
				header. distance_ = pVehicle->m_Distance;
				header. Energy = pVehicle->Energy;
				header.CO2 = pVehicle->CO2;
				header.NOX = pVehicle->NOX;
				header.CO =  pVehicle->CO;
				header.HC = pVehicle->HC;
				header. number_of_nodes = pVehicle->m_NodeSize;
				header. age = pVehicle->m_Age ;
				header.number_of_VMS_response_links = 0;
				header.version_no = 1;
				header.PM2_5  = pVehicle->PM2_5;

				struct_VehicleInfo_Header InfoHeaderAsVehicleInput;
				InfoHeaderAsVehicleInput.vehicle_id = pVehicle->m_AgentID;
				InfoHeaderAsVehicleInput.from_zone_id = pVehicle->m_OriginZoneID;
				InfoHeaderAsVehicleInput. to_zone_id = pVehicle->m_DestinationZoneID;
				InfoHeaderAsVehicleInput. departure_time = pVehicle->m_DepartureTime;
				InfoHeaderAsVehicleInput. demand_type = pVehicle->m_DemandType;
				InfoHeaderAsVehicleInput.agent_type = pVehicle->m_DemandType;
				InfoHeaderAsVehicleInput. vehicle_type =pVehicle->m_VehicleType;
				InfoHeaderAsVehicleInput. information_type = pVehicle->m_InformationType;
				InfoHeaderAsVehicleInput. value_of_time =pVehicle->m_VOT;


				fwrite(&header, sizeof(struct_VehicleInfo_Header), 1, st_struct);

				//

				// path_node_sequence 
				int j = 0;

				if(g_LinkVector[pVehicle->m_LinkAry [0].LinkNo]==NULL)
				{

					cout << "Error: vehicle" << pVehicle->m_AgentID << "at LinkID"<< pVehicle->m_LinkAry [0].LinkNo << endl;
					cin.get();  // pause

				}

				int NodeID = g_LinkVector[pVehicle->m_LinkAry [0].LinkNo]->m_FromNodeID;  // first node
				int NodeName = g_NodeVector[NodeID].m_NodeNumber ;


				

				if (output_path_sequence_flag == 1)
				{
					fprintf(st_agent, "%d;", NodeName);
				
					for(j = 0; j< pVehicle->m_NodeSize-1; j++)  // for all nodes
					{
						int LinkID = pVehicle->m_LinkAry [j].LinkNo;
						int NodeID = g_LinkVector[LinkID]->m_ToNodeID;
						int NodeName = g_NodeVector[NodeID].m_NodeNumber ;
						fprintf(st_agent, "%d;", NodeName);
					}
				}

				fprintf(st_agent, ",");

				if (pVehicle->m_OriginalLinkAry != NULL)  // with path switching
				{

					fprintf(st_agent, "%d;", NodeName);

					for (j = 0; j< pVehicle->m_OrgNodeSize - 1; j++)  // for all nodes
					{
						int LinkID = pVehicle->m_OriginalLinkAry[j].LinkNo;
						int NodeID = g_LinkVector[LinkID]->m_ToNodeID;
						int NodeName = g_NodeVector[NodeID].m_NodeNumber;
						fprintf(st_agent, "%d;", NodeName);
					}

				}
				

					fprintf(st_agent, ",");

				// path timestamp sequence
					if (output_path_sequence_flag == 1)
					{
					
							fprintf(st_agent, "%4.2f;",pVehicle->m_LeavingTimeFromLoadingBuffer) ;

							for(j = 0; j< pVehicle->m_NodeSize-1; j++)  // for all nodes
							{
								fprintf(st_agent, "%4.2f;",pVehicle->m_LinkAry [j].AbsArrivalTimeOnDSN) ;
							}

					}
					fprintf(st_agent, ",");

				// time and node sequence

				// link travel time sequence

		
				j = 0;
				if(g_LinkVector[pVehicle->m_LinkAry [0].LinkNo]==NULL)
				{

					cout << "Error: vehicle" << pVehicle->m_AgentID << "at LinkID"<< pVehicle->m_LinkAry [0].LinkNo << endl;
					cin.get();  // pause

				}

				NodeID = g_LinkVector[pVehicle->m_LinkAry [0].LinkNo]->m_FromNodeID;  // first node
				NodeName = g_NodeVector[NodeID].m_NodeNumber ;


				//				float link_entering_time = pVehicle->m_DepartureTime;
				float link_entering_time = pVehicle->m_LeavingTimeFromLoadingBuffer;

	
				struct_Vehicle_Node node_element;
				node_element. NodeName = NodeName;
				node_element. AbsArrivalTimeOnDSN = pVehicle->m_DepartureTime;
				fwrite(&node_element, sizeof(node_element), 1, st_struct);

				float LinkWaitingTime = 0;
				for(j = 0; j< pVehicle->m_NodeSize-1; j++)  // for all nodes
				{
					int LinkID = pVehicle->m_LinkAry [j].LinkNo;

					if(g_LinkVector[LinkID]->CapacityReductionVector.size() >=1)
						pVehicle->m_bImpacted = true;

					if(pVehicle->m_bImpacted)
					{
					g_LinkVector[LinkID]->CFlowImpactedCount ++;  // count vehicles being impacted
					}





					int NodeID = g_LinkVector[LinkID]->m_ToNodeID;
					int NodeName = g_NodeVector[NodeID].m_NodeNumber ;
					float LinkTravelTime = 0;
					float Emissions = 0;

					if(NodeName==7340)
						TRACE("");

					if(j==0) // origin node
					{
						//						link_entering_time =  pVehicle->m_DepartureTime;
						link_entering_time = pVehicle->m_LeavingTimeFromLoadingBuffer;
					}
					else
					{
						link_entering_time = pVehicle->m_LinkAry [j-1].AbsArrivalTimeOnDSN;
					}

					LinkTravelTime = (pVehicle->m_LinkAry [j].AbsArrivalTimeOnDSN) - link_entering_time;
					LinkWaitingTime = pVehicle->m_LinkAry [j].AbsArrivalTimeOnDSN - link_entering_time - g_LinkVector[LinkID]->m_FreeFlowTravelTime ;

					int FromNodeID = g_LinkVector[LinkID]->m_FromNodeID;
					int FromNodeNumber = g_NodeVector[FromNodeID].m_NodeNumber ;

					int ToNodeID = g_LinkVector[LinkID]->m_ToNodeID;
					int ToNodeNumber = g_NodeVector[ToNodeID].m_NodeNumber ;

					if(j < pVehicle->m_NodeSize-2 &&  pVehicle->m_bComplete ) // we have not reached the destination yest
					{

						int NextLinkID = pVehicle->m_LinkAry [j+1].LinkNo;
						int NextNodeID = g_LinkVector[NextLinkID]->m_ToNodeID;
						int DestNodeNumber = g_NodeVector[NextNodeID].m_NodeNumber ;

						// construct movement id
						string movement_id = GetMovementStringID(FromNodeNumber, NodeName,DestNodeNumber);
						if(g_NodeVector[ToNodeID].m_MovementMap.find(movement_id) != g_NodeVector[ToNodeID].m_MovementMap.end()) // the capacity for this movement has been defined
						{

							g_NodeVector[ToNodeID].m_MovementMap[movement_id].total_vehicle_count++;
							g_NodeVector[ToNodeID].m_MovementMap[movement_id].total_vehicle_delay += LinkWaitingTime;

							TRACE("movement: %d, %f, link travel time %f\n", g_NodeVector[ToNodeID].m_MovementMap[movement_id].total_vehicle_count,g_NodeVector[ToNodeID].m_MovementMap[movement_id].total_vehicle_delay,LinkWaitingTime);

						}


					}

					

					node_element. NodeName = NodeName;
					node_element. AbsArrivalTimeOnDSN = pVehicle->m_LinkAry [j].AbsArrivalTimeOnDSN;
					fwrite(&node_element, sizeof(node_element), 1, st_struct);


				} //for all nodes in path


				//
				if (pVehicle->m_NodeSize >= 2)
				{
					NodeID = g_LinkVector[pVehicle->m_LinkAry[0].LinkNo]->m_FromNodeID;  // first node
					fprintf(st_agent, "%f,%f,", g_NodeVector[NodeID].m_pt.x, g_NodeVector[NodeID].m_pt.y);
					NodeID = g_LinkVector[pVehicle->m_LinkAry[pVehicle->m_NodeSize-2].LinkNo]->m_ToNodeID;  // last node
					fprintf(st_agent, "%f,%f,", g_NodeVector[NodeID].m_pt.x, g_NodeVector[NodeID].m_pt.y);
			
				}

				fprintf(st_agent,"\n");
					fprintf(st_agent_compact,"\n");


				// mark this vehicle has output its trip data
					pVehicle->b_already_output_flag = true;



			}else
			{// without physical path in the network
				float TripTime = 0;
               
				fprintf(st_agent,"%d,%d,%d,%.0f,%d,%d,%d,%d,%d,%4.2f,%4.2f,%c,%4.2f,%d,%d,%d,%4.2f,%4.3f,%4.3f,%f,%f,%f,%f,%f,%d\n",
				pVehicle->m_AgentID, 
				pVehicle->m_ExternalTourID, 
				
				pVehicle->m_dependency_agent_id,
				pVehicle->m_duration_in_min,
				pVehicle->m_following_agent_id,

				
				pVehicle->m_OriginZoneID, pVehicle->m_DestinationZoneID,

					g_NodeVector[pVehicle->m_OriginNodeID].m_NodeNumber ,
					g_NodeVector[pVehicle->m_DestinationNodeID].m_NodeNumber ,

					pVehicle->m_DepartureTime, pVehicle->m_ArrivalTime , 
					g_get_vehicle_complete_flag(pVehicle->m_bComplete),

					TripTime,			
					pVehicle->m_DemandType, 
					pVehicle->m_VehicleType,
					pVehicle->m_InformationType, 
					pVehicle->m_VOT , pVehicle->m_TollDollarCost, pVehicle->m_Distance, 
					pVehicle->Energy ,pVehicle->CO2, pVehicle->NOX, pVehicle->CO, pVehicle->HC,
					pVehicle->m_NodeSize);

				// write everything out, without path sequence
			}
			//if(pVehicle->m_bLoaded == false) 
			//{
			//	cout << "Warning: Not loaded vehicle " << pVehicle->m_AgentID << " from zone " << 
			//		pVehicle->m_OriginZoneID << " to zone " << pVehicle->m_DestinationZoneID << " departing at"
			//		<< pVehicle->m_DepartureTime << " demand type = " << (int)(pVehicle->m_DemandType) << " Node Size in path = " <<  pVehicle->m_NodeSize << endl;
			//}
		} // for all paths

		// not loaded in simulation


		fclose(st_agent);
		fclose(st_agent_compact);

		fclose(st_struct);


	}else
	{
		cout << "Error: File output_agent.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		getchar();
	}

}

void g_ReadAgentTagSettings()
{


	CCSVParser parser_MOE_settings;
	if (parser_MOE_settings.OpenCSVFile("optional_MOE_settings.csv", false))
	{
		int tag_count = 0;
		while (parser_MOE_settings.ReadRecord())
		{
			string moe_type, moe_category_label;

			int demand_type = 0;
			int vehicle_type = 0;
			int information_type = 0;
			int from_node_id = 0;
			int mid_node_id = 0;
			int to_node_id = 0;
			int origin_zone_id = 0;
			int destination_zone_id = 0;
			int departure_starting_time = 0;
			int departure_ending_time = 1440;
			int entrance_starting_time = 0;
			int entrance_ending_time = 1440;


			parser_MOE_settings.GetValueByFieldName("moe_type", moe_type);
			parser_MOE_settings.GetValueByFieldName("moe_category_label", moe_category_label);

			parser_MOE_settings.GetValueByFieldName("demand_type", demand_type);
			parser_MOE_settings.GetValueByFieldName("vehicle_type", vehicle_type);
			parser_MOE_settings.GetValueByFieldName("information_type", information_type);
			parser_MOE_settings.GetValueByFieldName("from_node_id", from_node_id);
			parser_MOE_settings.GetValueByFieldName("mid_node_id", mid_node_id);
			parser_MOE_settings.GetValueByFieldName("to_node_id", to_node_id);
			parser_MOE_settings.GetValueByFieldName("origin_zone_id", origin_zone_id);
			parser_MOE_settings.GetValueByFieldName("destination_zone_id", destination_zone_id);
			parser_MOE_settings.GetValueByFieldName("departure_starting_time_in_min", departure_starting_time);
			parser_MOE_settings.GetValueByFieldName("departure_ending_time_in_min", departure_ending_time);
			parser_MOE_settings.GetValueByFieldName("entrance_starting_time_in_min", entrance_starting_time);
			parser_MOE_settings.GetValueByFieldName("entrance_ending_time_in_min", entrance_ending_time);


			int Count = 0;
			float AvgTripTime, AvgDistance, AvgSpeed, AvgCost;
			EmissionStatisticsData emission_data;
			LinkMOEStatisticsData  link_data;
			Count = g_TagAgents(AvgTripTime, AvgDistance, AvgSpeed, AvgCost, emission_data, link_data,
				demand_type, vehicle_type, information_type, origin_zone_id, destination_zone_id,
				from_node_id, mid_node_id, to_node_id,
				departure_starting_time, departure_ending_time, entrance_starting_time, entrance_ending_time);

			tag_count += Count;
		}


		parser_MOE_settings.CloseCSVFile();

	}else
	{
		cout << "Error: input_tag_settings.csv cannot be openned. "<< endl << "Please check!";
		g_ProgramStop();

	}
}

bool OutputTripFile(char fname_trip[_MAX_PATH],int output_mode)
{
	FILE* st_agent_compact = NULL;
	fopen_s(&st_agent_compact,fname_trip,"w");

	if(st_agent_compact==NULL)
	{
		cout << "File " << fname_trip << " cannot be opened." <<endl;
		return false;
	}
		std::map<int, DTAVehicle*>::iterator iterVM;
		int VehicleCount_withPhysicalPath = 0;

		//// count # of vehicles to be output

		//int count = 0;
		//for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		//{

		//	DTAVehicle* pVehicle = iterVM->second;
		//	if(pVehicle->m_NodeSize >= 2)  // with physical path in the network
		//	{


		//		if(bIncremental && (pVehicle->m_bComplete == false ||  
		//			(pVehicle->m_bComplete == true && pVehicle->b_already_output_flag == true))) 
		//			continue;
		//	count++;

		//	}

		//}

		//if(count ==0)  // no data to output
		//	return true; 
		int output_agent_count = 0;

		
		if(st_agent_compact!=NULL)
	{

			g_LogFile << " output agent data to file " << fname_trip << endl;



		// output statistics
		fprintf(st_agent_compact, "agent_id,tour_id,complete_flag,from_zone_id,to_zone_id,from_origin_node_id,to_destination_node_id,departure_time_in_min,end_time_in_min,travel_time_in_min,demand_type,information_type,value_of_time,vehicle_type,vehicle_age,distance,current_link,current_path_node_sequence,alt_path_node_sequence,current_travel_time_to_dest,alt_travel_time_to_dest\n");


		for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{

			DTAVehicle* pVehicle = iterVM->second;

			if(pVehicle->m_AgentID == 2)
				TRACE("");


			if (pVehicle->m_NodeSize >= 2 )  // with physical path in the network
			{
				if (output_mode == 0)  // complete
				{
				
					if (pVehicle->b_already_output_flag == false)
					{
						if (pVehicle->m_bComplete != true)
							continue;
						else
							output_agent_count++;

						pVehicle->b_already_output_flag = true;
					}
					else  // already output this trip-complete agents
					{
						continue;  // skip outputing 
					}
				}

				if (output_mode == 1)  // tag
				{

					if (pVehicle->m_bTag != true || pVehicle->m_bComplete == true) // if this vehicle is not tagged, or this tagged vehicle completes the trip, skip this vehicle in the output
						continue;
					else
						output_agent_count++;

				}


				int UpstreamNodeID = 0;
				int DownstreamNodeID = 0;

				int LinkID_0 = pVehicle->m_LinkAry [0].LinkNo;
				UpstreamNodeID= g_LinkVector[LinkID_0]->m_FromNodeID;
				DownstreamNodeID = g_LinkVector[LinkID_0]->m_ToNodeID;

				float TripTime = 0;

				if(pVehicle->m_bComplete)
					TripTime = pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime;

				int upstream_node = 0;
				int downstream_node = 0;

				if (pVehicle->m_bComplete == false)
				{

				int current_link_no = pVehicle->m_LinkAry[pVehicle->m_SimLinkSequenceNo].LinkNo;
				upstream_node = g_LinkVector[current_link_no]->m_FromNodeNumber;
				downstream_node = g_LinkVector[current_link_no]->m_ToNodeNumber;
				}
				

				float m_gap = 0;

				float travel_time = 0;

				if (pVehicle->m_bComplete == true)
					travel_time = pVehicle->m_ArrivalTime - pVehicle->m_DepartureTime;
				else
					travel_time = -1;  // not available 

				fprintf(st_agent_compact, "%d,%d,%d,%d,%d,%d,%d,%4.2f,%4.2f,%4.2f,%d,%d,%4.2f,%d,%d,%4.2f,%d;%d,",
					pVehicle->m_AgentID,
					pVehicle->m_ExternalTourID,
					pVehicle->m_bComplete,
					pVehicle->m_OriginZoneID,
					pVehicle->m_DestinationZoneID,
					g_NodeVector[pVehicle->m_OriginNodeID].m_NodeNumber,
					g_NodeVector[pVehicle->m_DestinationNodeID].m_NodeNumber,
					pVehicle->m_DepartureTime,

					pVehicle->m_ArrivalTime,
					travel_time,
					pVehicle->m_DemandType, 
					pVehicle->m_InformationType,
					pVehicle->m_VOT, 
					pVehicle->m_VehicleType, 
					pVehicle->m_Age,
					pVehicle->m_Distance,
					upstream_node,
					downstream_node
					);


				// path_node_sequence 
				int j = 0;

				if (g_LinkVector[pVehicle->m_LinkAry[0].LinkNo] == NULL)
				{

					cout << "Error: vehicle" << pVehicle->m_AgentID << "at LinkID" << pVehicle->m_LinkAry[0].LinkNo << endl;
					cin.get();  // pause

				}

				int NodeID = g_LinkVector[pVehicle->m_LinkAry[0].LinkNo]->m_FromNodeID;  // first node
				int NodeName = g_NodeVector[NodeID].m_NodeNumber;


				fprintf(st_agent_compact, "%d;", NodeName);
				for (j = 0; j< pVehicle->m_NodeSize - 1; j++)  // for all nodes
				{
					int LinkID = pVehicle->m_LinkAry[j].LinkNo;
					int NodeID = g_LinkVector[LinkID]->m_ToNodeID;
					int NodeName = g_NodeVector[NodeID].m_NodeNumber;
					fprintf(st_agent_compact, "%d;", NodeName);
				}
				fprintf(st_agent_compact, ",");


				// output alternative path information for tagged agents
//************************************************************************************************************************************///
				int PathLinkList[MAX_NODE_SIZE_IN_A_PATH] = { -1 };   // the new path to be copied back
				int CurrentPathLinkList[MAX_NODE_SIZE_IN_A_PATH] = { -1 };
				int TempPathLinkList[MAX_NODE_SIZE_IN_A_PATH] = { -1 };

				std::vector<int> UpdatedLinkList;
				float AbsArrivalTimeOnDSNVector[MAX_NODE_SIZE_IN_A_PATH] = { 0 };
				int PathNodeSize;
				int TempNodeSize;

				int StartingNodeID = pVehicle->m_OriginNodeID;

				float TotalCost = 0;
				bool bGeneralizedCostFlag = false;
				bool bDebugFlag = false;
				int link_count = 0;
				int CurrentLinkID = 0;
				int node_id;
				if (pVehicle->m_NodeSize >= 1) // with path
				{

					// if this is a pre-trip vehicle, and he has not obtained real-time information yet


					if (pVehicle->m_SimLinkSequenceNo < pVehicle->m_NodeSize - 1)  // has not reached the last link yet
					{
					
					CurrentLinkID = pVehicle->m_LinkAry[pVehicle->m_SimLinkSequenceNo].LinkNo;
					StartingNodeID = g_LinkVector[CurrentLinkID]->m_ToNodeID;

					if (StartingNodeID != pVehicle->m_DestinationNodeID)  // you will reach the destination (on the last link).
					{
					
				

					// copy existing link number (and remaining links)

					for (int i = 0; i< pVehicle->m_NodeSize - 1; i++)
					{
						CurrentPathLinkList[i] = pVehicle->m_LinkAry[i].LinkNo;
						AbsArrivalTimeOnDSNVector[i] = pVehicle->m_LinkAry[i].AbsArrivalTimeOnDSN;
					}


					link_count = pVehicle->m_SimLinkSequenceNo;


					for (int i = 0; i <= pVehicle->m_SimLinkSequenceNo; i++)
					{
						UpdatedLinkList.push_back(CurrentPathLinkList[i]);  // first step

					}


					//if(is.Type == 2) // detour VMS
					//{
					//for(int ll = 0; ll < is.DetourLinkSize ; ll++)
					//{
					//PathLinkList[l++] = is.DetourLinkArray [ll];
					//CurrentTime+= g_LinkVector[is.DetourLinkArray [ll]]->m_FreeFlowTravelTime;  // add travel time along the path to get the starting time at the end of subpath
					//}

					//CurrentNodeID = g_LinkVector[is.DetourLinkArray [is.DetourLinkSize -1]]->m_ToNodeID;

					//}



				//calculate the shortest path based on prevailing travel time
				float perception_error_for_alternative_path_generation = 0.5;
				int ProcessID = 0;
				int departure_time = pVehicle->m_DepartureTime;
				PathNodeSize = g_PrevailingTimeNetwork_MP[ProcessID].FindBestPathWithVOTForSingleAgent(pVehicle->m_OriginZoneID,
					StartingNodeID, 
					departure_time,
					pVehicle->m_DestinationZoneID, 
					pVehicle->m_DestinationNodeID,
					pVehicle->m_DemandType, 
					pVehicle->m_VOT, 
					PathLinkList, 
					TotalCost, 
					bGeneralizedCostFlag,
					bDebugFlag,
					perception_error_for_alternative_path_generation);

				//count is the current node sequence number
				int bAlternativeFlag = 0;
				for (int i = 0; i<PathNodeSize - 1; i++)
				{
					link_count += 1;  // add link no from the new path starting from the current position 

					if (CurrentPathLinkList[link_count] != PathLinkList[i])
					{
						bAlternativeFlag = 1;

						_proxy_ABM_log(0, " writing tagged agent file %s with tagged agent %d with alternative path\n", fname_trip, pVehicle->m_AgentID);

						break;
					}
				}

				pVehicle->m_alt_path_node_sequence.clear();

				if(bAlternativeFlag == 1)
				{
					for (int i = 0; i<PathNodeSize - 1; i++)
					{
						node_id = g_LinkVector[PathLinkList[i]]->m_ToNodeID;

						fprintf(st_agent_compact, "%d;", g_NodeVector[node_id].m_NodeNumber);

						pVehicle->m_alt_path_node_sequence.push_back(g_NodeVector[node_id].m_NodeNumber);

						_proxy_ABM_log(0, "node %d\n", g_NodeVector[node_id].m_NodeNumber);

					}

				}
					float current_travel_time = 0;
					for (int i = pVehicle->m_SimLinkSequenceNo; i< pVehicle->m_NodeSize - 1; i++)
					{
						current_travel_time += g_LinkVector[pVehicle->m_LinkAry[i].LinkNo]->m_prevailing_travel_time;
					}
					int current_zone_id = 0;

					fprintf(st_agent_compact, ",%3.1f,", current_travel_time);

					if (bAlternativeFlag == 1 && pVehicle->m_alt_path_node_sequence.size() >=1)
					{
						float alt_travel_time = 0;
						for (int i = 0; i < PathNodeSize - 1; i++)
						{
							alt_travel_time += g_LinkVector[PathLinkList[i]]->m_prevailing_travel_time;
						}
						_proxy_ABM_log(0, "current travel time to destination = %f (min); alt travel time to destination = %f (min); \n",
							current_travel_time, alt_travel_time);



						fprintf(st_agent_compact, "%3.1f,", alt_travel_time);
					}

					_proxy_ABM_log(0, "\n");
				}
				fprintf(st_agent_compact, ",");
				}

					}
				}
				fprintf(st_agent_compact,"\n");

				
		} // for all paths
		if (output_mode == 1)
			_proxy_ABM_log(0, " writing tagged agent file %s with %d tagged agents\n", fname_trip, output_agent_count);
		if (output_mode == 0)
			_proxy_ABM_log(0, " writing complete agent file %s with %d agents completing trips.\n", fname_trip,  output_agent_count);

		fclose(st_agent_compact);

		return true;
	}else
	{
	    return false;
	}
}
struct ChartData
{
public:
	double x;
	double y;
	CString str;

	ChartData()
	{
	x= 0;
	y = 0;

	
	}

};
void g_DrawPieChart(FILE* st, CString Name, double longitude, double latitude, double size_ratio,std::vector<ChartData> DataVector, int style)
{
fprintf(st,"<Placemark id=\"%s\">\n",Name);

fprintf(st,"<name>%s</name>", Name);


switch(style)
{
case 1:
fprintf(st,"<description><![CDATA[<img src='http://chart.apis.google.com/chart?cht=p&amp;chd=t:");
case 2:
fprintf(st,"<description><![CDATA[<img src='http://chart.apis.google.com/chart?cht=p3&amp;chd=t:");

}

for(int i= 0; i< DataVector.size (); i++)
{
	fprintf(st,"%f",DataVector[i].x);

	if(i!= DataVector.size ()-1)
		fprintf(st,",");
}
		
int size = 300*size_ratio;

switch(style)
{
case 1:
fprintf(st,"&amp;chs=500x150&amp;chf=bg,s,ffffff00&amp;chco=FF7800&amp;chl=");break;
case 2:
fprintf(st,"chs=%dx150&amp;chf=bg,s,ffffff00&amp;chco=FF7800&amp;chl=", size);break;
}


for(int i= 0; i< DataVector.size (); i++)
{
	DataVector[i].str.Replace (" ", "%20");
	fprintf(st,"%s",DataVector[i].str );

	if(i!= DataVector.size ()-1)
		fprintf(st,"%%7C");
}

fprintf(st,"'>]]></description><LookAt>");
fprintf(st,"<longitude>%f</longitude>\n<latitude>%f</latitude>\n",longitude,latitude);
fprintf(st,"<altitude>0</altitude>\n<range>3200000</range>\n<tilt>0</tilt>\
			<heading>0</heading>\
		</LookAt>\
		<Style>\
			<IconStyle>\
				<scale>2.17107</scale>\
				<Icon>");

fprintf(st,"<href>http://chart.apis.google.com/chart?cht=p&amp;chd=t:");


for(int i= 0; i< DataVector.size (); i++)
{
	fprintf(st,"%f",DataVector[i].x);

	if(i!= DataVector.size ()-1)
		fprintf(st,",");
}

size = 200*size_ratio;

fprintf(st,"&amp;chs=%dx%d&amp;chf=bg,s,ffffff00&amp;chco=FF7800</href>\
				</Icon>\
			</IconStyle>\
			</Style>\
		<Point>\n",size,size);

fprintf(st,"<coordinates>%f,%f,0</coordinates>\n", longitude,latitude);
fprintf(st,"</Point>\n</Placemark>");
}

void g_PrintSchemaSimpleField(FILE* st, CString name, CString type)
{
	fprintf(st,"<SimpleField name=\"%s\" type=\"%s\"></SimpleField>\n",name,type);

}

void g_PrintSchemaSimpleData(FILE* st, CString name, float value)
{
	fprintf(st,"<SimpleData name=\"%s\">%5.2f</SimpleData>\n",name,value);
}

void g_PrintSchemaSimpleData(FILE* st, CString name, CString value)
{
	fprintf(st,"<SimpleData name=\"%s\">%s</SimpleData>\n",name,value);
}

void g_PrintSchemaSimpleData(FILE* st, CString name, int value)
{
	fprintf(st,"<SimpleData name=\"%s\">%d</SimpleData>\n",name,value);
}

void g_ExportLink3DLayerToKMLFiles(CString file_name, CString GISTypeString, int ColorCode, bool no_curve_flag, float height_ratio)
{

	DeleteFile(file_name);
	// m_LinkBandWidthMode =  LBW_number_of_lanes;
	FILE* st;
	fopen_s(&st,file_name,"w");
	GDPoint BandPoint[2000];
	if(st!=NULL)
	{
		fprintf(st,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		fprintf(st,"<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
		fprintf(st,"<Document>\n");
		fprintf(st,"<name>Traffic Simulation Data</name>\n");

		// blue style
		fprintf(st,"<Style id=\"green\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d00ff00</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// red style

		fprintf(st,"<Style id=\"red\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d0000ff</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// blue style

		fprintf(st,"<Style id=\"blue\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7dff0000</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// yellow style

		fprintf(st,"<Style id=\"yellow\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d00ffff</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

	COLORREF m_colorLOS[MAX_LOS_SIZE];
	CString LOS_str[MAX_LOS_SIZE];
	m_colorLOS[0] = RGB(190,190,190);
	m_colorLOS[1] = RGB(0,255,0);
	m_colorLOS[2] = RGB(255,250,117);
	m_colorLOS[3] = RGB(255,250,0);
	m_colorLOS[4] = RGB(255,216,0);
	m_colorLOS[5] = RGB(255,153,0);
	m_colorLOS[6] = RGB(255,0,0);

	double m_LOSBound[40][MAX_LOS_SIZE];
	m_LOSBound[MOE_speed][1] = 100;
	m_LOSBound[MOE_speed][2] = 80;
	m_LOSBound[MOE_speed][3] = 65;
	m_LOSBound[MOE_speed][4] = 50;
	m_LOSBound[MOE_speed][5] = 40;
	m_LOSBound[MOE_speed][6] = 33;
	m_LOSBound[MOE_speed][7] = 0;

	int Transparency = 70;
   for(int i = 1; i<MAX_LOS_SIZE-1; i++)
	   {
			char str[10];
			int transparency = 255- Transparency /100.0*255;
			wsprintf(str, "%02x%06x",  transparency, m_colorLOS[i]);

		LOS_str[i].Format ("color%d", i);
		fprintf(st,"<Style id=\"%s\">\n", LOS_str[i]);
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>%s</color>\n",str );
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");
		}

       fprintf(st,"<Style id=\"no_control_node\">\n<BalloonStyle>\n<displayMode>hide</displayMode>\n</BalloonStyle>\n</Style>");
       fprintf(st,"<Style id=\"signal_node\">\
      <IconStyle>\
        <Icon>\
          <href>http://maps.google.com/mapfiles/kml/paddle/red-stars.png</href>\
        </Icon>\
      </IconStyle>\
    </Style>");
/////////////////////////// Node layer
fprintf(st,"<Folder><name>Node Layer</name> \n");
fprintf(st,"\t\t<visibility>0</visibility>\n");

fprintf(st,"<Schema name=\"node\" id=\"node\">\n");
g_PrintSchemaSimpleField(st,"Description" , "string");
g_PrintSchemaSimpleField(st,"Name" , "string");
g_PrintSchemaSimpleField(st,"node_id" , "int");
g_PrintSchemaSimpleField(st,"control_type" , "string");
fprintf(st,"</Schema>\n");

for(unsigned i = 0; i< g_NodeVector.size(); i++)
{
		fprintf(st,"<Placemark id=\"Node%d\">\n",g_NodeVector[i].m_NodeNumber );
			
				if(	g_NodeVector[i].m_ControlType != g_settings.pretimed_signal_control_type_code &&
					g_NodeVector[i].m_ControlType != g_settings.actuated_signal_control_type_code )
				{		
		fprintf(st,"\t\t<styleUrl>#no_control_node</styleUrl>\n");
				}else
				{
		fprintf(st,"\t\t<styleUrl>#signal_node</styleUrl>\n");
		fprintf(st,"<description><p>\
    			<Snippet maxLines=\"0\"></Snippet>\
          <a href=\"http://www.civil.utah.edu/~zhou/QEM_SIG.xls\">Download QEM spreadsheet</a>\<![CDATA[<img src='QEM.png'>]]></p></description>");

				}

		fprintf(st,"<name>Node %d</name>\n", g_NodeVector[i].m_NodeNumber);
	fprintf(st,"<ExtendedData><SchemaData schemaUrl=\"#node\">\n");


		g_PrintSchemaSimpleData(st,"node_id",g_NodeVector[i].m_NodeNumber);
		g_PrintSchemaSimpleData(st,"control_type", g_NodeControlTypeMap[g_NodeVector[i].m_ControlType].c_str ());
	
	fprintf(st,"</SchemaData></ExtendedData>\n");


	fprintf(st,"<Point><coordinates>%f,%f</coordinates></Point>\n</Placemark>\n", g_NodeVector[i].m_pt .x , g_NodeVector[i].m_pt .y);

   //     	fprintf(st,"\t\t<visibility>0</visibility>\n");
			//fprintf(st,"\t\t<styleUrl>#%s</styleUrl>\n","blue");

			//fprintf(st,"\t\t<Polygon>\n");
			//fprintf(st,"\t\t<extrude>1</extrude>\n");
			//fprintf(st,"\t\t<altitudeMode>relativeToGround</altitudeMode>\n");
			//fprintf(st,"\t\t<outerBoundaryIs>\n");
			//fprintf(st,"\t\t<LinearRing>\n");
			//fprintf(st,"\t\t<coordinates>\n");
			//float height  =  10;

	//		float offset = g_UnitOfMileOrKM/5280*50;  // 20 feet per node

	//		std::vector<GDPoint>  n_ShapePoints;
	//		GDPoint pt;
	//		pt.x =  g_NodeVector[i].m_pt .x - offset;
	//		pt.y =  g_NodeVector[i].m_pt .y - offset;
	//		n_ShapePoints.push_back (pt);

	//		pt.x =  g_NodeVector[i].m_pt .x - offset;
	//		pt.y =  g_NodeVector[i].m_pt .y + offset;
	//		n_ShapePoints.push_back (pt);

	//		pt.x =  g_NodeVector[i].m_pt .x + offset;
	//		pt.y =  g_NodeVector[i].m_pt .y + offset;
	//		n_ShapePoints.push_back (pt);

	//		pt.x =  g_NodeVector[i].m_pt .x + offset;
	//		pt.y =  g_NodeVector[i].m_pt .y - offset;
	//		n_ShapePoints.push_back (pt);

	//		float height = 7;
	//fprintf(st,"<LineString><coordinates>");
	//		for(unsigned int si = 0; si< n_ShapePoints.size(); si++)
	//		{

	//			fprintf(st,"%f,%f,%f ", n_ShapePoints[si].x, n_ShapePoints[si].y,height);
	//		}
	//
	//	fprintf(st,"</coordinates></LineString>\n</Placemark>\n");



			//fprintf(st,"\t\t</coordinates>\n");
			//fprintf(st,"\t\t</LinearRing>\n");
			//fprintf(st,"\t\t</outerBoundaryIs>\n");

			//fprintf(st,"\t\t</Polygon>\n");
		//	fprintf(st,"\t</Placemark>\n");

}

fprintf(st,"</Folder>\n");


///////////////////////////// Signal layer
//fprintf(st,"<Folder><name>Signal Layer</name> \n");
//fprintf(st,"\t\t<visibility>0</visibility>\n");
//
//fprintf(st,"<Schema name=\"signal\" id=\"signal\">\n");
//g_PrintSchemaSimpleField(st,"Description" , "string");
//g_PrintSchemaSimpleField(st,"Name" , "string");
//fprintf(st,"</Schema>\n");
//
//for(unsigned i = 0; i< g_NodeVector.size(); i++)
//{
//			
//				if(	g_NodeVector[i].m_ControlType != g_settings.pretimed_signal_control_type_code &&
//					g_NodeVector[i].m_ControlType != g_settings.actuated_signal_control_type_code )
//				{	
//					continue;
//				}	
//				
//				
//	fprintf(st,"<Placemark id=\"Signal%d\">\n",g_NodeVector[i].m_NodeNumber );
//
//
//	fprintf(st,"<name>Signal %d</name>\n", g_NodeVector[i].m_NodeNumber);
//	fprintf(st,"<ExtendedData><SchemaData schemaUrl=\"#node\">\n");
//	g_PrintSchemaSimpleData(st,"node_id",g_NodeVector[i].m_NodeNumber);
//
//	fprintf(st,"</SchemaData></ExtendedData>\n");
//
//	fprintf(st,"<Point><coordinates>%f,%f</coordinates></Point>\n</Placemark>\n", g_NodeVector[i].m_pt .x , g_NodeVector[i].m_pt .y);
//}
//
//fprintf(st,"</Folder>\n");

/////////////////////////// Link layer
fprintf(st,"<Folder><name>Link Layer</name> \n");
fprintf(st,"<Schema name=\"link\" id=\"link\">\n");
g_PrintSchemaSimpleField(st,"Description" , "string");
g_PrintSchemaSimpleField(st,"Name" , "string");
g_PrintSchemaSimpleField(st,"from_node_id" , "int");
g_PrintSchemaSimpleField(st,"to_node_id" , "int");
g_PrintSchemaSimpleField(st,"link_id" , "int");
g_PrintSchemaSimpleField(st,"link_length" , "float");
g_PrintSchemaSimpleField(st,"number_of_lanes" , "int");
g_PrintSchemaSimpleField(st,"speed_limit_mph" , "float");
g_PrintSchemaSimpleField(st,"capacity_per_lane_per_hour" , "float");
g_PrintSchemaSimpleField(st,"capacity_per_link_per_hour" , "float");
g_PrintSchemaSimpleField(st,"free_flow_travel_time_(min)" , "float");

fprintf(st,"</Schema>\n");

for(unsigned i = 0; i< g_LinkVector.size(); i++)
{
		fprintf(st,"<Placemark id=\"Link%d\">\n",g_LinkVector[i]->m_LinkNo +1 );
		fprintf(st,"<name>Link %s</name>\n", g_LinkVector[i]->m_Name.c_str ());
		fprintf(st,"<ExtendedData><SchemaData schemaUrl=\"#link\">\n");


		g_PrintSchemaSimpleData(st,"from_node_id", g_LinkVector[i]->m_FromNodeNumber );
		g_PrintSchemaSimpleData(st,"to_node_id", g_LinkVector[i]->m_ToNodeNumber );
		g_PrintSchemaSimpleData(st,"link_id",  g_LinkVector[i]->m_LinkNo+1);
		g_PrintSchemaSimpleData(st,"link_length_",  g_LinkVector[i]->m_Length );
		g_PrintSchemaSimpleData(st,"number_of_lanes",  g_LinkVector[i]->m_OutflowNumLanes );
		g_PrintSchemaSimpleData(st,"speed_limit_mph",  g_LinkVector[i]->m_SpeedLimit   );
		g_PrintSchemaSimpleData(st,"capacity_per_lane_per_hour",  g_LinkVector[i]->m_LaneCapacity   );
		g_PrintSchemaSimpleData(st,"capacity_per_link_per_hour",  g_LinkVector[i]->m_LaneCapacity*g_LinkVector[i]-> m_OutflowNumLanes  );
		g_PrintSchemaSimpleData(st,"free_flow_travel_time_(min)",  g_LinkVector[i]->m_FreeFlowTravelTime  );
	
	fprintf(st,"</SchemaData></ExtendedData>\n");

	fprintf(st,"<LineString><coordinates>");
			for(int mi= 0; mi< g_LinkVector[i]->m_ShapePoints.size()  ; mi++)
			{

			fprintf(st,"%f,%f,7",g_LinkVector[i]->m_ShapePoints[mi].x, g_LinkVector[i]->m_ShapePoints[mi].y);

				if(mi!= g_LinkVector[i]->m_ShapePoints.size()-1)
				{
				fprintf(st," ");
				}
			}
	
		fprintf(st,"</coordinates></LineString>\n</Placemark>\n");


 
}


fprintf(st,"</Folder>\n");

/////////////////////////// Zone layer
fprintf(st,"<Folder><name>Zone Layer</name> \n");

for (std::map<int, DTAZone>::iterator iterZone = g_ZoneMap.begin(); iterZone != g_ZoneMap.end(); iterZone++)
	{
		fprintf(st,"<Placemark id=\"Zone_%d\">\n",iterZone->first );
			fprintf(st,"\t\t<name>Zone %d</name>\n",iterZone->first );
        	fprintf(st,"\t\t<visibility>0</visibility>\n");
			fprintf(st,"\t\t<styleUrl>#%s</styleUrl>\n","yellow");

			fprintf(st,"\t\t<Polygon>\n");
			fprintf(st,"\t\t<extrude>1</extrude>\n");
			fprintf(st,"\t\t<altitudeMode>relativeToGround</altitudeMode>\n");
			fprintf(st,"\t\t<outerBoundaryIs>\n");
			fprintf(st,"\t\t<LinearRing>\n");
			fprintf(st,"\t\t<coordinates>\n");
			float height  =  iterZone->second.m_Demand;

			for(unsigned int si = 0; si< iterZone->second.m_ShapePoints.size(); si++)
			{


				fprintf(st,"\t\t\t%f,%f,%f\n", iterZone->second.m_ShapePoints[si].x, iterZone->second.m_ShapePoints[si].y,height);
			}

			fprintf(st,"\t\t</coordinates>\n");
			fprintf(st,"\t\t</LinearRing>\n");
			fprintf(st,"\t\t</outerBoundaryIs>\n");

			fprintf(st,"\t\t</Polygon>\n");
			fprintf(st,"\t</Placemark>\n");
	
		
	}

fprintf(st,"</Folder>\n");

/////////////////////////// work zone layer
fprintf(st,"<Folder><name>Work Zone Layer</name> \n");

int work_zone_count = 1;

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{
		if(g_LinkVector[li]->CapacityReductionVector.size()>=1)
		{
	
			std::vector<GDPoint> ShapePoints;

			if( g_LinkVector[li]->m_ShapePoints.size() == 2)
			{
				int size = max(1,g_LinkVector[li]->m_Length /0.05);
				for(int mi= 0; mi< size  ; mi+=1)
				{	
					GDPoint pt;
					float ratio  =mi*1.0/size;
					pt.x = (1-ratio)* g_LinkVector[li]->m_ShapePoints[0].x +  ratio *  g_LinkVector[li]->m_ShapePoints[1].x;
					pt.y = (1-ratio)* g_LinkVector[li]->m_ShapePoints[0].y +  ratio *  g_LinkVector[li]->m_ShapePoints[1].y;
					ShapePoints.push_back (pt);


				}			
			}
			else
			{
				 ShapePoints = g_LinkVector[li]->m_ShapePoints;
			}

	
			
			for(int mi= 0; mi< ShapePoints.size()  ; mi++)
			{


		fprintf(st,"<Placemark id=\"WorkZone%d_%d\">\n",work_zone_count,mi+1);
		fprintf(st,"<name>WorkZone %d -(%d)</name>\n", work_zone_count,mi+1);
		fprintf(st,"<description>Work Zone %d</description>\n",work_zone_count);

		double longitude = ShapePoints [mi].x ;
		double latitude  = ShapePoints [mi].y ;
		
		fprintf(st,"<LookAt> <longitude>%f</longitude> <latitude>%f</latitude> <altitude>0</altitude>",longitude,latitude);
		fprintf(st,"<range>127.2393107680517</range><tilt>65.74454495876547</tilt><heading>-27.70337734057933</heading></LookAt>\n");
		fprintf(st,"<Model id=\"model_4\"> <altitudeMode>relativeToGround</altitudeMode><Location>");
		fprintf(st,"<longitude>%f</longitude> 	<latitude>%f</latitude>\n",longitude,latitude);
		fprintf(st,"<altitude>0</altitude>\
			</Location>\
			<Orientation>\
				<heading>0</heading>\
				<tilt>0</tilt>\
				<roll>0</roll>\
			</Orientation>\
			<Scale>\
				<x>50</x>\
				<y>50</y>\
				<z>50</z>\
			</Scale>\
			<Link>\
				<href>WorkZone.dae</href>\
			</Link>\
		</Model>\
	</Placemark>\n");
			}
		
		work_zone_count++;
		}
	}
		fprintf(st,"</Folder>\n");


/////////////////////////// variable message sign layer
fprintf(st,"<Folder><name>Variable Message Sign</name> \n");

int VMS_count = 1;
	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{
		if(g_LinkVector[li]->MessageSignVector.size()>=1)
		{
	
			std::vector<GDPoint> ShapePoints;

			if( g_LinkVector[li]->m_ShapePoints.size() == 2)
			{
				int size = max(1,g_LinkVector[li]->m_Length /0.1);
				for(int mi= 0; mi< size  ; mi+=1)
				{	
					GDPoint pt;
					float ratio  =mi*1.0/size;
					pt.x = (1-ratio)* g_LinkVector[li]->m_ShapePoints[0].x +  ratio *  g_LinkVector[li]->m_ShapePoints[1].x;
					pt.y = (1-ratio)* g_LinkVector[li]->m_ShapePoints[0].y +  ratio *  g_LinkVector[li]->m_ShapePoints[1].y;
					ShapePoints.push_back (pt);


				}			
			}
			else
			{
				 ShapePoints = g_LinkVector[li]->m_ShapePoints;
			}

	
			
			for(int mi= 0; mi< ShapePoints.size()  ; mi++)
			{


		fprintf(st,"<Placemark id=\"VMS%d_%d\">\n",VMS_count,mi+1);
		fprintf(st,"<name>Message Sign %d -(%d)</name>\n", VMS_count,mi+1);
		fprintf(st,"<description>Message Sign %d</description>\n",VMS_count);

		double longitude = ShapePoints [mi].x ;
		double latitude  = ShapePoints [mi].y ;
		
		fprintf(st,"<LookAt> <longitude>%f</longitude> <latitude>%f</latitude> <altitude>0</altitude>",longitude,latitude);
		fprintf(st,"<range>127.2393107680517</range><tilt>65.74454495876547</tilt><heading>-27.70337734057933</heading></LookAt>\n");
		fprintf(st,"<Model id=\"model_4\"> <altitudeMode>relativeToGround</altitudeMode><Location>");
		fprintf(st,"<longitude>%f</longitude> 	<latitude>%f</latitude>\n",longitude,latitude);
		fprintf(st,"<altitude>0</altitude>\
			</Location>\
			<Orientation>\
				<heading>0</heading>\
				<tilt>0</tilt>\
				<roll>0</roll>\
			</Orientation>\
			<Scale>\
				<x>10</x>\
				<y>10</y>\
				<z>10</z>\
			</Scale>\
			<Link>\
				<href>VMS.dae</href>\
			</Link>\
		</Model>\
	</Placemark>\n");
			}
		
		VMS_count++;
		}
	}
		fprintf(st,"</Folder>\n");



/////////////////////////// Traffic Detector layer
fprintf(st,"<Folder><name>Traffic Detector</name> \n");

int sensor_count = 1;
	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{
		if(g_LinkVector[li]->m_bSensorData && g_LinkVector[li]->m_ShapePoints.size() >=2)
		{
	
		GDPoint pt;
		float ratio  = 0.5;
		pt.x = (1-ratio)* g_LinkVector[li]->m_ShapePoints[0].x +  ratio *  g_LinkVector[li]->m_ShapePoints[1].x;
		pt.y = (1-ratio)* g_LinkVector[li]->m_ShapePoints[0].y +  ratio *  g_LinkVector[li]->m_ShapePoints[1].y;


		fprintf(st,"<Placemark id=\"Detector%d\">\n",sensor_count);
		fprintf(st,"<name>Detector %d</name>\n", sensor_count);
		fprintf(st,"<description>Detector %d</description>\n",sensor_count);

		double longitude = pt.x ;
		double latitude  = pt.y ;
		
		fprintf(st,"<LookAt> <longitude>%f</longitude> <latitude>%f</latitude> <altitude>0</altitude>",longitude,latitude);
		fprintf(st,"<range>127.2393107680517</range><tilt>65.74454495876547</tilt><heading>-27.70337734057933</heading></LookAt>\n");
		fprintf(st,"<Model id=\"model_4\"> <altitudeMode>relativeToGround</altitudeMode><Location>");
		fprintf(st,"<longitude>%f</longitude> 	<latitude>%f</latitude>\n",longitude,latitude);
		fprintf(st,"<altitude>0</altitude>\
			</Location>\
			<Orientation>\
				<heading>0</heading>\
				<tilt>0</tilt>\
				<roll>0</roll>\
			</Orientation>\
			<Scale>\
				<x>3</x>\
				<y>3</y>\
				<z>3</z>\
			</Scale>\
			<Link>\
				<href>Detector.dae</href>\
			</Link>\
		</Model>\
	</Placemark>\n");
		sensor_count++;
			}
		
		}
	
		fprintf(st,"</Folder>\n");


/////////////////////////// Node bottleneck layer
fprintf(st,"<Folder><name>Node Bottleneck</name> \n");

std::vector<double> NodeDelayVector;

for(unsigned i = 0; i< g_NodeVector.size(); i++)
{

	NodeDelayVector.push_back (g_NodeVector[i].m_TotalDelay);

}


	std::sort(NodeDelayVector.begin (), NodeDelayVector.end ());


	double threshold  = 30;

	if(NodeDelayVector.size() >=11)
		threshold = max(30,NodeDelayVector[NodeDelayVector.size() -9]);

	double max_size = max(1,NodeDelayVector[NodeDelayVector.size()-1]);

	for(unsigned i = 0; i< g_NodeVector.size(); i++)
	{
		if(g_NodeVector[i].m_TotalDelay>threshold)
		{

			CString name;
			name.Format("Bottleneck %d ", g_NodeVector[i].m_NodeNumber  );

			
			std::vector<ChartData> DataVector;
			for(int In = 0; In< g_NodeVector[i].m_IncomingLinkVector.size(); In++)
		{

			int li = g_NodeVector[i].m_IncomingLinkVector[In];

			int NodeID = g_LinkVector[li]->m_FromNodeID;  // first node
			int NodeName = g_NodeVector[NodeID].m_NodeNumber ;

			ChartData element;
			element.x =  g_NodeVector[i].m_IncomingLinkDelay [In]/60;  // convert min to hour
			element.str.Format("%s: %3.1f(hr)", g_LinkVector[li]->m_Name.c_str (),element.x );

			DataVector.push_back(element);
		}

			double size_ratio = g_NodeVector[i].m_TotalDelay/max_size;
			g_DrawPieChart(st, name,g_NodeVector[i].m_pt.x, g_NodeVector[i].m_pt.y, size_ratio, DataVector,1);
		}
		
		}
	
		fprintf(st,"</Folder>\n");

		/////////////////// Link Layer
		fprintf(st,"<Folder>\n");
   		fprintf(st,"<name>Link MOE Layer</name>\n");
   		fprintf(st," <visibility>0</visibility>\n");
      	

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{

		DTALink* pLink = g_LinkVector[li];

		float m_BandWidthValue = g_LinkVector[li]->m_OutflowNumLanes;

		std::list<DTALink*>::iterator iLink;

		double lane_offset = g_UnitOfMileOrKM/5280*20;  // 20 feet per lane

		std::vector<GDPoint> m_BandLeftShapePoints, m_BandRightShapePoints, m_ReferenceBandLeftShapePoints, m_ReferenceBandRightShapePoints;


		if(pLink->m_ShapePoints.size() ==0)
			continue;

		int last_shape_point_id = pLink ->m_ShapePoints .size() -1;
		double DeltaX = pLink->m_ShapePoints[last_shape_point_id].x - pLink->m_ShapePoints[0].x;
		double DeltaY = pLink->m_ShapePoints[last_shape_point_id].y - pLink->m_ShapePoints[0].y;
		double theta = 0;

		if(fabs(DeltaY)>0.0000001)
			theta = atan2(DeltaY, DeltaX);


		for(unsigned int si = 0; si < pLink ->m_ShapePoints .size(); si++)
		{

			// calculate theta for each feature point segment
			if(si>= 1 && (pLink ->m_ShapePoints .size() >4 || g_LinkTypeMap[pLink->m_link_type].IsRamp ()))  // ramp or >4 feature points
			{
				last_shape_point_id = si;
				DeltaX = pLink->m_ShapePoints[last_shape_point_id].x - pLink->m_ShapePoints[si-1].x;
				DeltaY = pLink->m_ShapePoints[last_shape_point_id].y - pLink->m_ShapePoints[si-1].y;

				if(fabs(DeltaY)>0.00001)
					theta = atan2(DeltaY, DeltaX);
			}

			GDPoint pt;

			pt.x = pLink->m_ShapePoints[si].x ;
			pt.y = pLink->m_ShapePoints[si].y ;

			m_BandLeftShapePoints.push_back (pt);

			pt.x  = pLink->m_ShapePoints[si].x + m_BandWidthValue*lane_offset* cos(theta-PI/2.0f);
			pt.y = pLink->m_ShapePoints[si].y + m_BandWidthValue*lane_offset* sin(theta-PI/2.0f);

			m_BandRightShapePoints.push_back (pt);

			if(pLink->m_bSensorData) // refernece band
			{
				double m_ReferenceBandWidthValue = 0;

				pt.x = pLink->m_ShapePoints[si].x ;
				pt.y = pLink->m_ShapePoints[si].y ;

				m_ReferenceBandLeftShapePoints.push_back (pt);

				pt.x  = pLink->m_ShapePoints[si].x + m_ReferenceBandWidthValue*lane_offset* cos(theta-PI/2.0f);
				pt.y = pLink->m_ShapePoints[si].y +  m_ReferenceBandWidthValue*lane_offset* sin(theta-PI/2.0f);
				m_ReferenceBandRightShapePoints.push_back (pt);
			}

	
	}
	
		int time_step = 4;
		int t = 0;
		double ratio = 1;
		

	fprintf(st,"<Placemark id=\"Link3DVolume%d\">\n",pLink->m_LinkNo +1);
			fprintf(st,"\t\t<name>%s</name>\n",pLink->m_Name.c_str () );
        	fprintf(st,"\t\t<visibility>0</visibility>\n");


			CString color_code = "green";

			double average_travel_time = pLink->GetTravelTimeByMin(g_NumberOfIterations,0, pLink->m_SimulationHorizon,g_TrafficFlowModelFlag);
			double speed = pLink->m_Length / max(0.00001,average_travel_time) *60;  // unit: mph
			double capacity_simulation_horizon = pLink->m_LaneCapacity * pLink->m_OutflowNumLanes;


	// speed LOS bound

			int current_los_code = 1;

			Link_MOE e_lmoe = MOE_speed;

			float Value =  speed/pLink->m_SpeedLimit *100;
			for(int los = 1; los < MAX_LOS_SIZE-1; los++)
				{
					if( (m_LOSBound[e_lmoe]  [los] <= Value && Value < m_LOSBound[e_lmoe]  [los+1]*1.001)
						|| (m_LOSBound[e_lmoe]  [los] >= Value && Value > m_LOSBound[e_lmoe]  [los+1]*1.001))
					{

						current_los_code = los;
					break;
					}

				}	
			
			color_code = LOS_str[current_los_code];

			fprintf(st,"\t\t<styleUrl>#%s</styleUrl>\n",color_code);
			fprintf(st,"\t\t<Polygon>\n");
			fprintf(st,"\t\t<extrude>1</extrude>\n");
			fprintf(st,"\t\t<altitudeMode>relativeToGround</altitudeMode>\n");
			fprintf(st,"\t\t<outerBoundaryIs>\n");
			fprintf(st,"\t\t<LinearRing>\n");
			fprintf(st,"\t\t<coordinates>\n");

			float height = height_ratio * pLink->CFlowArrivalCount /max(1,(g_DemandLoadingEndTimeInMin - g_DemandLoadingStartTimeInMin )/60.0);

			int si;
			int band_point_index = 0;
			int size = pLink ->m_ShapePoints .size();
		for(si = 0; si < pLink ->m_ShapePoints .size(); si++)
		{
			if(no_curve_flag && size >=3 && si >=1 && si< size -1) // skip intermedate points
				continue;

			BandPoint[band_point_index++] = m_BandLeftShapePoints[si];
		}

		for(si = pLink ->m_ShapePoints .size()-1; si >=0 ; si--)
		{
			if(no_curve_flag && size >=3 && si >=1 && si< size -1) // skip intermedate points
				continue;

			BandPoint[band_point_index++] = m_BandRightShapePoints[si];
		}

		BandPoint[band_point_index++]= m_BandLeftShapePoints[0];



			for(unsigned int i = 0; i< band_point_index; i++)
			{
				fprintf(st,"\t\t\t%f,%f,%f\n", BandPoint[i].x,  BandPoint[i].y,height);
			}


			fprintf(st,"\t\t</coordinates>\n");
			fprintf(st,"\t\t</LinearRing>\n");
			fprintf(st,"\t\t</outerBoundaryIs>\n");

			fprintf(st,"\t\t</Polygon>\n");
			fprintf(st,"\t</Placemark>\n");
			
		}  // for each link
	 fprintf(st,"<ScreenOverlay>\n<name>Speed Legend Logo</name>\n<drawOrder>20000001</drawOrder>\n <Icon>\n<href>legend_link_speed.png</href>\n </Icon>\n<overlayXY x=\"1.0\" y=\"0.0\" xunits=\"fraction\" yunits=\"fraction\"/>\n");
	  fprintf(st,"<screenXY x=\"0.2\" y=\"0.05\" xunits=\"fraction\" yunits=\"fraction\"/>\n </ScreenOverlay>\n");

	  fprintf(st,"</Folder>\n");	
////////////////// impacted Volume
		{
		fprintf(st,"<Folder>\n");
   		fprintf(st,"<name>Impacted Volume Layer</name>\n");
   		fprintf(st," <visibility>0</visibility>\n");
      	

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{

		DTALink* pLink = g_LinkVector[li];

		float m_BandWidthValue = g_LinkVector[li]->m_OutflowNumLanes;

		std::list<DTALink*>::iterator iLink;

		double lane_offset = g_UnitOfMileOrKM/5280*20;  // 20 feet per lane

		std::vector<GDPoint> m_BandLeftShapePoints, m_BandRightShapePoints, m_ReferenceBandLeftShapePoints, m_ReferenceBandRightShapePoints;


		if(pLink->m_ShapePoints.size() ==0)
			continue;

		int last_shape_point_id = pLink ->m_ShapePoints .size() -1;
		double DeltaX = pLink->m_ShapePoints[last_shape_point_id].x - pLink->m_ShapePoints[0].x;
		double DeltaY = pLink->m_ShapePoints[last_shape_point_id].y - pLink->m_ShapePoints[0].y;
		double theta = 0;

		if(fabs(DeltaY)>0.0000001)
			theta = atan2(DeltaY, DeltaX);


		for(unsigned int si = 0; si < pLink ->m_ShapePoints .size(); si++)
		{

			// calculate theta for each feature point segment
			if(si>= 1 && (pLink ->m_ShapePoints .size() >4 || g_LinkTypeMap[pLink->m_link_type].IsRamp ()))  // ramp or >4 feature points
			{
				last_shape_point_id = si;
				DeltaX = pLink->m_ShapePoints[last_shape_point_id].x - pLink->m_ShapePoints[si-1].x;
				DeltaY = pLink->m_ShapePoints[last_shape_point_id].y - pLink->m_ShapePoints[si-1].y;

				if(fabs(DeltaY)>0.00001)
					theta = atan2(DeltaY, DeltaX);
			}

			GDPoint pt;

			pt.x = pLink->m_ShapePoints[si].x ;
			pt.y = pLink->m_ShapePoints[si].y ;

			m_BandLeftShapePoints.push_back (pt);

			pt.x  = pLink->m_ShapePoints[si].x + m_BandWidthValue*lane_offset* cos(theta-PI/2.0f);
			pt.y = pLink->m_ShapePoints[si].y + m_BandWidthValue*lane_offset* sin(theta-PI/2.0f);

			m_BandRightShapePoints.push_back (pt);

			if(pLink->m_bSensorData) // refernece band
			{
				double m_ReferenceBandWidthValue = 0;

				pt.x = pLink->m_ShapePoints[si].x ;
				pt.y = pLink->m_ShapePoints[si].y ;

				m_ReferenceBandLeftShapePoints.push_back (pt);

				pt.x  = pLink->m_ShapePoints[si].x + m_ReferenceBandWidthValue*lane_offset* cos(theta-PI/2.0f);
				pt.y = pLink->m_ShapePoints[si].y +  m_ReferenceBandWidthValue*lane_offset* sin(theta-PI/2.0f);
				m_ReferenceBandRightShapePoints.push_back (pt);
			}

	
	}
	
		int time_step = 4;
		int t = 0;
		double ratio = 1;
		
		fprintf(st,"<Placemark id=\"ImpactedVolume%d\">\n",pLink->m_LinkNo +1);
			fprintf(st,"\t\t<name>L%s</name>\n",pLink->m_Name.c_str () );
        	fprintf(st,"\t\t<visibility>0</visibility>\n");


			CString color_code = "red";

			fprintf(st,"\t\t<styleUrl>#%s</styleUrl>\n",color_code);
			fprintf(st,"\t\t<Polygon>\n");
			fprintf(st,"\t\t<extrude>1</extrude>\n");
			fprintf(st,"\t\t<altitudeMode>relativeToGround</altitudeMode>\n");
			fprintf(st,"\t\t<outerBoundaryIs>\n");
			fprintf(st,"\t\t<LinearRing>\n");
			fprintf(st,"\t\t<coordinates>\n");

			float height = height_ratio * pLink->CFlowImpactedCount /max(1,(g_DemandLoadingEndTimeInMin - g_DemandLoadingStartTimeInMin )/60.0);

			int si;
			int band_point_index = 0;
			int size = pLink ->m_ShapePoints .size();
		for(si = 0; si < pLink ->m_ShapePoints .size(); si++)
		{
			if(no_curve_flag && size >=3 && si >=1 && si< size -1) // skip intermedate points
				continue;

			BandPoint[band_point_index++] = m_BandLeftShapePoints[si];
		}

		for(si = pLink ->m_ShapePoints .size()-1; si >=0 ; si--)
		{
			if(no_curve_flag && size >=3 && si >=1 && si< size -1) // skip intermedate points
				continue;

			BandPoint[band_point_index++] = m_BandRightShapePoints[si];
		}

		BandPoint[band_point_index++]= m_BandLeftShapePoints[0];



			for(unsigned int i = 0; i< band_point_index; i++)
			{
				fprintf(st,"\t\t\t%f,%f,%f\n", BandPoint[i].x,  BandPoint[i].y,height);
			}


			fprintf(st,"\t\t</coordinates>\n");
			fprintf(st,"\t\t</LinearRing>\n");
			fprintf(st,"\t\t</outerBoundaryIs>\n");

			fprintf(st,"\t\t</Polygon>\n");
			fprintf(st,"\t</Placemark>\n");
			
		}  // for each link

	  fprintf(st,"</Folder>\n");	
fprintf(st,"<Folder><name>Summary</name> \n");
fprintf(st,"\t\t<visibility>1</visibility>\n");
// summary

	 fprintf(st,"<ScreenOverlay>\n<name>Logo</name>\n<drawOrder>20000000</drawOrder>\n <Icon>\n<href>Logo.png</href>\n </Icon>\n<overlayXY x=\"1.0\" y=\"0.0\" xunits=\"fraction\" yunits=\"fraction\"/>\n");
	  fprintf(st,"<screenXY x=\"0.8\" y=\"0.05\" xunits=\"fraction\" yunits=\"fraction\"/>\n </ScreenOverlay>\n");
		fprintf(st,"</Folder>\n");
	}

			fprintf(st,"</Document>\n");
			fprintf(st,"</kml>\n");
			fclose(st);
	} // end of file




}





