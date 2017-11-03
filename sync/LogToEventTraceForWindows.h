#pragma once

#include "stdafx.h"

#ifdef LOG_TO_EVENT_TRACE_FOR_WINDOWS_CPP
#define LOG_TO_EVENT_HEAD
#else
#define LOG_TO_EVENT_HEAD extern
#endif // LOG_TO_EVENT_TRACE_FOR_WINDOWS_CPP

LOG_TO_EVENT_HEAD HANDLE hEventLog;

LOG_TO_EVENT_HEAD VOID DebugInfoToEventLogWrapper(LPCTSTR fmt, ...);
LOG_TO_EVENT_HEAD VOID WarnInfoToEventLogWrapper(LPCTSTR fmt, ...);
LOG_TO_EVENT_HEAD VOID ErrorInfoToEventLogWrapper(LPCTSTR fmt, ...);

