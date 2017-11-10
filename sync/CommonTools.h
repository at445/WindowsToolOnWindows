#pragma once

#include "stdafx.h"

#ifdef COMMON_TOOLS_CPP
#define COMMON_TOOLS_HEAD
#else
#define COMMON_TOOLS_HEAD extern
#endif // COMMON_TOOLS_CPP

COMMON_TOOLS_HEAD std::wstring getTextContentFromClipboard(VOID);
COMMON_TOOLS_HEAD void MouseSetup(INPUT *buffer);
COMMON_TOOLS_HEAD void MouseMoveAbsolute(INPUT *buffer, int x, int y);
COMMON_TOOLS_HEAD void MouseClick(INPUT *buffer);
COMMON_TOOLS_HEAD std::wstring GetProcessNameByID(DWORD processID);
