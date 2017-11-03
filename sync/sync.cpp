// sync.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Oleacc.h>
#include "LogToEventTraceForWindows.h"
#include "ProcessMonitor.h"

int main()
{
	ProcessMonitorInit();
	ProMonitorRef windows1, windows2;
	AddMonitorTarget(L"WindowsFormsApp1.exe", windows1);
	AddMonitorTarget(L"WindowsFormsApp1 - Copy.exe", windows2);

	NotifyProcessMonitorStart();
	return 0;
}

