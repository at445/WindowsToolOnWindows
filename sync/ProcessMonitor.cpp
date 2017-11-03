#include "stdafx.h"
#include <thread>
#include <chrono>
#include <string>
#include <map>
#include <list>
#include <algorithm>
#define PROCESS_MONITOR_CPP
#include "ProcessMonitor.h"
#include "CommonTools.h"
#include "LogToEventTraceForWindows.h"
using namespace std;

static std::thread g_threadHandle;
static BOOL g_MonitorInited = FALSE;
static HANDLE g_MonitorStartEvent = NULL;
static HANDLE g_MonitorStopEvent = NULL;
static HANDLE g_MonitorTerminate = NULL;
static multimap<wstring, ProMonitorRef&> g_MonitorRef;


VOID MonitorProcessPROC(VOID)
{
	DWORD aProcesses[1024 * 4];
	DWORD cbNeeded;
	map<DWORD, wstring> BaseProcessInfo;
	map<DWORD, wstring> CurrentProcessInfo;

start:
	WaitForSingleObject(g_MonitorStartEvent, INFINITE);
	memset(aProcesses, 0, sizeof(DWORD) * 1024 * 4);
	EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded);
	for (DWORD i = 0; i < cbNeeded / sizeof(DWORD); i++)
	{
		wstring prcName = GetProcessNameByID(aProcesses[i]);
		if (g_MonitorRef.find(prcName) != g_MonitorRef.end()) {
			BaseProcessInfo[aProcesses[i]] = prcName;
		}
	}
	while (true) {
		if (WaitForSingleObject(g_MonitorStopEvent, 300) == WAIT_OBJECT_0) {
			goto start;
		}

		if (WaitForSingleObject(g_MonitorTerminate, 0) == WAIT_OBJECT_0) {
			return;
		}

		memset(aProcesses, 0, sizeof(DWORD) * 1024 * 4);

		if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
			continue;

		if(!CurrentProcessInfo.empty()) CurrentProcessInfo.clear();
		for (DWORD i = 0; i < cbNeeded / sizeof(DWORD); i++)
		{
			wstring prcName = GetProcessNameByID(aProcesses[i]);
			if(g_MonitorRef.find(prcName) != g_MonitorRef.end()) {
				CurrentProcessInfo[aProcesses[i]] = prcName;
			}
		}		

		//generate create event
		for_each(CurrentProcessInfo.begin(), CurrentProcessInfo.end(), [&BaseProcessInfo](pair<DWORD, wstring> item1) {
			if (BaseProcessInfo.end() == BaseProcessInfo.find(item1.first)) {
				BaseProcessInfo.insert(item1);
				for_each(g_MonitorRef.begin(), g_MonitorRef.end(), [&item1](pair<wstring, ProMonitorRef&> item2) {
					if (item1.second == item2.first) {
						SetEvent(item2.second.EventCreated);
						ResetEvent(item2.second.EventDestoryed);
					}
				});
			}
		});

		//generate destory event
		for (auto Itor = BaseProcessInfo.begin(); Itor != BaseProcessInfo.end(); ) {
			if (CurrentProcessInfo.end() == CurrentProcessInfo.find(Itor->first)) {
				for_each(g_MonitorRef.begin(), g_MonitorRef.end(), [&Itor](pair<wstring, ProMonitorRef&> item) {
					if (Itor->second == item.first) {
						SetEvent(item.second.EventDestoryed);
						ResetEvent(item.second.EventCreated);
					}
				});
				Itor = BaseProcessInfo.erase(Itor);
			}
			else {
				Itor++;
			}
		}
	}
}

BOOL ProcessMonitorInit(VOID)
{
	g_MonitorStartEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	g_MonitorStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	g_MonitorTerminate = CreateEvent(NULL, FALSE, FALSE, NULL);
	if ((g_MonitorStartEvent == NULL) || (g_MonitorStopEvent == NULL) || (g_MonitorTerminate == NULL)) {
		ErrorInfoToEventLogWrapper(L"Failed in create event in process Monitor init, error code %d", GetLastError());
		return FALSE;
	}

	g_threadHandle = std::thread(MonitorProcessPROC);
	g_MonitorInited = TRUE;
	return TRUE;
}

BOOL ProcessDetectDeInit(VOID)
{
	if (g_MonitorInited) {
		SetEvent(g_MonitorTerminate);
		g_threadHandle.join();

		CloseHandle(g_MonitorStartEvent);
		CloseHandle(g_MonitorStopEvent);
		CloseHandle(g_MonitorTerminate);
	}
	return TRUE;
}

VOID NotifyProcessMonitorStart(VOID)
{
	if (g_MonitorInited) {
		SetEvent(g_MonitorStartEvent);
	}
}

VOID NotifyProcessMonitorStop(VOID)
{
	if (g_MonitorInited) {
		SetEvent(g_MonitorStopEvent);
	}
}

BOOL AddMonitorTarget(const std::wstring& Name, ProMonitorRef &ref)
{
	if (Name == L"") {
		return FALSE;
	}

	if (RPC_S_OK != UuidCreate(&ref.uid)) {
		ErrorInfoToEventLogWrapper(L"Failed in create uuid when adding process Monitor Target, error code %d", GetLastError());
		return FALSE;
	}

	ref.EventCreated = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (ref.EventCreated == NULL) {
		ErrorInfoToEventLogWrapper(L"Failed in create event when adding process Monitor Target, error code %d", GetLastError());
		return FALSE;
	}

	ref.EventDestoryed = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (ref.EventDestoryed == NULL) {
		ErrorInfoToEventLogWrapper(L"Failed in create event when adding process Monitor Target, error code %d", GetLastError());
		CloseHandle(ref.EventCreated);
		return FALSE;
	}

	g_MonitorRef.insert(pair<wstring, ProMonitorRef&>(Name, ref));
	return TRUE;
}

BOOL RemoveMonitorTarget(const ProMonitorRef &ref)
{
	BOOL ret = FALSE;
	for (auto Itor = g_MonitorRef.begin(); Itor != g_MonitorRef.end(); ) {
		if (0 == memcmp(&(Itor->second.uid), &ref.uid, sizeof(UUID))) {
			Itor = g_MonitorRef.erase(Itor);
			CloseHandle(ref.EventCreated);
			CloseHandle(ref.EventDestoryed);
			ret = TRUE;
		} else {
			Itor++; 
		}
	}
	return ret;
}