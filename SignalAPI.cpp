#include "stdafx.h"
#include "DTALite.h"
#include "GlobalData.h"
#include <iostream>
#include <fstream>
#include <omp.h>
#include <algorithm>
#include <stdlib.h>  
#include <math.h>    



using namespace std;

float DTALink::GetTimeDependentCapacityAtSignalizedIntersectionFromTimingStatus(double CurrentTimeInMin)
{
	float simulation_time_interval_in_second = g_DTASimulationInterval_InMin * 60;

	int simulation_start_time = CurrentTimeInMin * 60;

	int green_time_count_in_seconds = 0;

	for (int t = simulation_start_time; t < simulation_start_time + simulation_time_interval_in_second; t++)
	{

		if (m_Timing_Status_Map.find(t) != m_Timing_Status_Map.end())  // has been defined as status of 1
		{
			green_time_count_in_seconds++;
		}
	}

	return green_time_count_in_seconds*1.0 / simulation_time_interval_in_second * 1800;  // 1800 is the saturation flow rate per hour

}

float DTALink::GetTimeDependentLeftTurnCapacityAtSignalizedIntersectionFromTimingStatus(double CurrentTimeInMin)
{
	float simulation_time_interval_in_second = g_DTASimulationInterval_InMin * 60;

	int simulation_start_time = CurrentTimeInMin * 60;

	int green_time_count_in_seconds = 0;

	for (int t = simulation_start_time; t < simulation_start_time + simulation_time_interval_in_second; t++)
	{

		if (m_Timing_Status_LeftTurnMap.find(t) != m_Timing_Status_LeftTurnMap.end())  // has been defined as status of 1
		{
			green_time_count_in_seconds++;
		}
	}

	return green_time_count_in_seconds*1.0 / simulation_time_interval_in_second * 1800;  // 1800 is the saturation flow rate per hour

}

void g_ReadSignalTimingStatusFile()
{
	for (unsigned li = 0; li < g_LinkVector.size(); li++)
	{

		g_LinkVector[li]->m_Timing_Status_Map.clear();
		g_LinkVector[li]->m_Timing_Status_LeftTurnMap.clear();

	}

	DTALink* pLink = 0;
	CCSVParser parser_link;
	int count = 0;
	if (parser_link.OpenCSVFile("input_timing_status.csv"))
	{
		while (parser_link.ReadRecord())
		{
			int start_time_in_sec = 0;
			int end_time_in_sec = 0;
			int signal_status = -1;
			int from_node_id, to_node_id;

			string turn_type;

			int from_node_name = 0;
			int to_node_name = 0;

			parser_link.GetValueByFieldNameRequired("from_node_id", from_node_name);

			parser_link.GetValueByFieldNameRequired("to_node_id", to_node_name);

			if (g_LinkMap.find(GetLinkStringID(from_node_name, to_node_name)) == g_LinkMap.end())
			{
				cout << "Link " << from_node_name << "-> " << to_node_name << " at line " << count + 1 << " of file input_timging_status.csv" << " has not been defined in input_link.csv. Please check.";
				g_ProgramStop();
			}

			parser_link.GetValueByFieldNameRequired("start_time_in_sec", start_time_in_sec);
			parser_link.GetValueByFieldNameRequired("end_time_in_sec", end_time_in_sec);
			parser_link.GetValueByFieldNameRequired("signal_status", signal_status);
			parser_link.GetValueByFieldNameRequired("turn_type", turn_type);


			DTALink* pLink = g_LinkMap[GetLinkStringID(from_node_name, to_node_name)];

			if (pLink != NULL && signal_status == 1)
			{
				pLink->m_bSignalizedArterialType = true;

				if (turn_type == "T" || turn_type == "R") // jun edit 2/24/2019
				{
					for (int t = start_time_in_sec; t < end_time_in_sec; t++)
					{
						pLink->m_Timing_Status_Map[t] = 1;    // assign the values to timing map, for the use in function GetTimeDependentCapacityAtSignalizedIntersectionFromTimingStatus
					}
				}
				if (turn_type == "L")
				{
					for (int t = start_time_in_sec; t < end_time_in_sec; t++)
					{
						pLink->m_Timing_Status_LeftTurnMap[t] = 1;   // assign the values to left turn timing map, for the use in function GetTimeDependentCapacityAtSignalizedIntersectionFromTimingStatus
					}
				}

				count++;

			}
		}

		g_LogFile << "timing records = " << count << endl;
		g_SummaryStatFile.WriteTextLabel("# of signal timing records=");
		g_SummaryStatFile.WriteNumber(count);

		}
}
