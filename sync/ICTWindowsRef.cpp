#include "stdafx.h"
#include <string>
#include <vector>
#include <Oleacc.h>
#include <fstream>
#include "CommonTools.h"
#include "LogToEventTraceForWindows.h"
#include "ProcessMonitor.h"

// Global variable.
HWINEVENTHOOK g_hook;
BOOL g_windowsRefInited = FALSE;
HWND g_WindowsHanlder;
HWND g_CopyToClipboardButton;

#define ICT_WINDOWS_TEXT L"try"
#define ICT_EXECUTABLE_FILE_NAME L"try.exe"
#define ICT_COPY_TO_CLIPBOARD_BUTTON L"copy to clipboard"
#define ICT_PREVIOUS_COPY_TOCLIPBOARD_BUTTON L"Print"
#define LOCATION_OF_FAIL_LOG L""

BOOL CALLBACK EnumChildWindowsProc(HWND hWnd, LPARAM lParam) {
	WCHAR TextContent[1024];
	int iLen = GetWindowText(hWnd, TextContent, 1024);
	TextContent[iLen] = 0;
	if (NULL != wcsstr(TextContent, ICT_COPY_TO_CLIPBOARD_BUTTON)) {
		
		HWND previousWin = GetNextWindow(hWnd, GW_HWNDPREV);
		if (NULL == previousWin) {
			ErrorInfoToEventLogWrapper(L"Failed in get previous windows, error code %d", GetLastError());
		}
		int iLen = GetWindowText(previousWin, TextContent, 1024);
		TextContent[iLen] = 0;
		if (NULL != wcsstr(TextContent, ICT_PREVIOUS_COPY_TOCLIPBOARD_BUTTON)) {
			DebugInfoToEventLogWrapper(L"get the button of copy to clipboard, handle is %d", hWnd); 
			g_CopyToClipboardButton = hWnd;
			g_windowsRefInited = TRUE;
		}
	}
	return TRUE;
}
BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) {
	WCHAR TextContent[1024];
	if (!hWnd)
		return TRUE;
	if (!::IsWindowVisible(hWnd))
		return TRUE;
	int iLen = GetWindowText(hWnd, TextContent, 1024);
	TextContent[iLen] = 0;
	if (NULL != wcsstr(TextContent, ICT_WINDOWS_TEXT)) {
		g_WindowsHanlder = hWnd;
		EnumChildWindows(hWnd, EnumChildWindowsProc, NULL);
	}
	return TRUE;
}

BOOL ContentInit()
{
	if (g_windowsRefInited) return TRUE;

	EnumWindows(EnumWindowsProc, NULL);
	return g_windowsRefInited;
}

void SaveErrorLOG(void)
{
	try {
		RECT rect;
		GetWindowRect(g_CopyToClipboardButton, &rect);
		INPUT input;
		MouseSetup(&input);
		MouseMoveAbsolute(&input, (rect.left + rect.right) / 2, (rect.bottom + rect.top) / 2);
		MouseClick(&input);

		std::wofstream ofs(LOCATION_OF_FAIL_LOG, std::ios::out);
		if (!ofs) return;
		ofs << getTextContentFromClipboard() << std::endl;
		ofs.close();
	}
	catch(...) {
		ErrorInfoToEventLogWrapper(L"error in saving the error log to %s, error code %d", LOCATION_OF_FAIL_LOG, GetLastError());
	}
}

// Callback function that handles events.
//
void CALLBACK HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
	LONG idObject, LONG idChild,
	DWORD dwEventThread, DWORD dwmsEventTime)
{
	static std::wstring sLastStatus;
	static std::vector<DWORD> sErrorTime;
	WCHAR ClassName[1024];
	int iLen = GetClassName(hwnd, ClassName, 1024);
	ClassName[iLen] = 0;
	if (0 != memcmp(ClassName, L"TPanel", sizeof(L"TPanel"))) {
		return;
	}
	WCHAR TextContent[1024];
	iLen = GetWindowText(hwnd, TextContent, 1024);
	TextContent[iLen] = 0;
	if (0 != memcmp(TextContent, L"Fail", sizeof(L"Fail"))) {
		DebugInfoToEventLogWrapper(L"Detect a fail status");
		if (TRUE == ContentInit()) {
			// save the log first
			SaveErrorLOG();

			if (sLastStatus == L"Test") {
				DebugInfoToEventLogWrapper(L"Detect a Test->Fail transfer message on %u", GetTickCount());
				sErrorTime.push_back(GetTickCount());
				if ((sErrorTime.size() > 2) &&
					(sErrorTime[sErrorTime.size() - 1] - sErrorTime[sErrorTime.size() - 3] < 60 * 60 * 1000)) {

					MessageBox(g_WindowsHanlder, L"Detect 3 errors during one hour, contact Maintance Team for more support.", L"ICTEnhancementTools", MB_OK);
					sErrorTime.clear();
				}
			}
		}
		else {
			ErrorInfoToEventLogWrapper(L"Failed in getting the handle of the copy to clipboard");
		}
		
	}

	sLastStatus = TextContent;
}

// Initializes COM and sets up the event hook.
//
void InitializeMSAA(DWORD processID)
{
	CoInitialize(NULL);
	g_hook = SetWinEventHook(
		EVENT_OBJECT_NAMECHANGE, EVENT_OBJECT_NAMECHANGE,  // Range of events.
		NULL,                                          // Handle to DLL.
		HandleWinEvent,                                // The callback.
		processID, 0,              // Process and thread IDs of interest (0 = all)
		WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS); // Flags.
}

// Unhooks the event and shuts down COM.
//
void ShutdownMSAA()
{
	UnhookWinEvent(g_hook);
	CoUninitialize();
	g_windowsRefInited = FALSE;
}

int main()
{
	ProcessMonitorInit();
	ProMonitorRef ICTWindows;
	AddMonitorTarget(ICT_EXECUTABLE_FILE_NAME, ICTWindows);
	NotifyProcessMonitorStart();
	HANDLE ghEvents[2];
	ghEvents[0] = ICTWindows.EventCreated;
	ghEvents[1] = ICTWindows.EventDestoryed;
	while (1) {
		DWORD dwEvent = WaitForMultipleObjects(2, ghEvents, FALSE, INFINITE);
		switch (dwEvent)
		{
		case WAIT_OBJECT_0 + 0:
			DebugInfoToEventLogWrapper(L"get an event of creating an instance of %s", ICT_EXECUTABLE_FILE_NAME);
			InitializeMSAA(ICTWindows.ProcessID);
			break;
		case WAIT_OBJECT_0 + 1:
			DebugInfoToEventLogWrapper(L"get an event of destroying an instance of %s", ICT_EXECUTABLE_FILE_NAME);
			ShutdownMSAA();
			break;
		case WAIT_TIMEOUT:
			break;
		default:
			ErrorInfoToEventLogWrapper(L"WaitForMultipleObjects error, error code %d", GetLastError());
			ExitProcess(0);
		}
	}

	return 0;
}
