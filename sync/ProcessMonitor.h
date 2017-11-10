#pragma once

#include "stdafx.h"
#include <string>
#include <vector>
#ifdef PROCESS_MONITOR_CPP
#define PROCESS_MONITOR_HEAD
#else
#define PROCESS_MONITOR_HEAD extern
#endif // PROCESS_MONITOR_CPP
typedef struct {
	HANDLE EventCreated;
	HANDLE EventDestoryed;
	DWORD ProcessID;
	UUID uid;
}ProMonitorRef;

PROCESS_MONITOR_HEAD BOOL ProcessMonitorInit(VOID);
PROCESS_MONITOR_HEAD BOOL ProcessMonitorDeInit(VOID);
PROCESS_MONITOR_HEAD VOID NotifyProcessMonitorStart(VOID);
PROCESS_MONITOR_HEAD VOID NotifyProcessMonitorStop(VOID);

PROCESS_MONITOR_HEAD BOOL AddMonitorTarget(const std::wstring&, ProMonitorRef&);
PROCESS_MONITOR_HEAD BOOL RemoveMonitorTarget(const ProMonitorRef &);
