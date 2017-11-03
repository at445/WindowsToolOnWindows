
#include "stdafx.h"
#include <stdarg.h>
#include <wchar.h>
#define LOG_TO_EVENT_TRACE_FOR_WINDOWS_CPP
#include "LogToEventTraceForWindows.h"
#include "EventProvider.h"
#define PROVIDER_NAME L"SyncPAMAAndPackingTools"

static void EventToETW( WORD SubType, LPCTSTR pContent)
{
	if (NULL == hEventLog) {
		hEventLog = RegisterEventSource(NULL, PROVIDER_NAME);
		if (NULL == hEventLog) {
			return;
		}
	}

	int dwEventDataSize = ((DWORD)wcslen(pContent) + 1) * sizeof(WCHAR);

	if (!ReportEventW(hEventLog, SubType, UI_CATEGORY, MSG_GENERAL_INFO, NULL, 1, dwEventDataSize, &pContent, (LPVOID)pContent))
	{
		return;
	}
}

VOID DebugInfoToEventLogWrapper(LPCTSTR format, ...)
{
	wchar_t printf_buf[1024 * 4];

	va_list args;
	va_start(args, format);
	wvsprintf(printf_buf, format, args);
	va_end(args);

	EventToETW(EVENTLOG_INFORMATION_TYPE, printf_buf);
}

VOID WarnInfoToEventLogWrapper(LPCTSTR format, ...)
{
	wchar_t printf_buf[1024 * 4];

	va_list args;
	va_start(args, format);
	wvsprintf(printf_buf, format, args);
	va_end(args);

	EventToETW(EVENTLOG_WARNING_TYPE, printf_buf);
}

VOID ErrorInfoToEventLogWrapper(LPCTSTR format, ...)
{
	wchar_t printf_buf[1024 * 4];

	va_list args;
	va_start(args, format);
	wvsprintf(printf_buf, format, args);
	va_end(args);

	EventToETW(EVENTLOG_ERROR_TYPE, printf_buf);
}