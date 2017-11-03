#include "stdafx.h"
#include <string>
#define COMMON_TOOLS_CPP
#include "CommonTools.h"
#include "LogToEventTraceForWindows.h"
using namespace std;
wstring getTextContentFromClipboard()
{
	wstring strData;

	if (OpenClipboard(NULL))
	{
		HANDLE hClipboardData = GetClipboardData(CF_UNICODETEXT);
		if (hClipboardData)
		{
			WCHAR *pchData = (WCHAR*)GlobalLock(hClipboardData);
			if (pchData)
			{
				strData = pchData;
				GlobalUnlock(hClipboardData);
			}
		}
		CloseClipboard();
	}
	return strData;
}

void MouseSetup(INPUT *buffer)
{
	buffer->type = INPUT_MOUSE;
	buffer->mi.dx = (0 * (0xFFFF / GetSystemMetrics(SM_CXSCREEN)));
	buffer->mi.dy = (0 * (0xFFFF / GetSystemMetrics(SM_CYSCREEN)));
	buffer->mi.mouseData = 0;
	buffer->mi.dwFlags = MOUSEEVENTF_ABSOLUTE;
	buffer->mi.time = 0;
	buffer->mi.dwExtraInfo = 0;
}


void MouseMoveAbsolute(INPUT *buffer, int x, int y)
{
	buffer->mi.dx = (x * (0xFFFF / GetSystemMetrics(SM_CXSCREEN)));
	buffer->mi.dy = (y * (0xFFFF / GetSystemMetrics(SM_CYSCREEN)));
	buffer->mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE);

	SendInput(1, buffer, sizeof(INPUT));
}


void MouseClick(INPUT *buffer)
{
	buffer->mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN);
	SendInput(1, buffer, sizeof(INPUT));

	Sleep(10);

	buffer->mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP);
	SendInput(1, buffer, sizeof(INPUT));
}

wstring GetProcessNameByID(DWORD processID)
{
	TCHAR szProcessName[MAX_PATH] = { 0 };

	// Get a handle to the process.

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, processID);

	// Get the process name.
	if (hProcess == NULL) {
		return L"";
	}

	HMODULE hMod;
	DWORD cbNeeded;

	if (0 == EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
	{
		ErrorInfoToEventLogWrapper(L"Failed in Retrieves a handle for each module, process id is %d, error code is %d", processID, GetLastError());
		CloseHandle(hProcess);
		return L"";
	}

	if (0 == GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR))) {
		ErrorInfoToEventLogWrapper(L"Failed in get module, process id is %d, error code is %d", processID, GetLastError());
		CloseHandle(hProcess);
		return L"";
	}

	CloseHandle(hProcess);
	return szProcessName;
}
