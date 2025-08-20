#include "stdafx.h"
#include "Autostart.h"
#include "registry_value.h"

typedef registry_string<tstring> RRegTString;
static const TCHAR cc_sAutostartPath[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run\\");


void AutostartRegEntry(const tstring& a_sKey, HKEY a_hKeyBase, bool a_bRun)
{
	tstring l_sRegPath = cc_sAutostartPath + a_sKey;
	RRegTString l_regStart(l_sRegPath, a_hKeyBase);
	if (l_regStart.exists() && !a_bRun)
	{
		l_regStart.remove_value();
	}
	else if (!l_regStart.exists() && a_bRun)
	{
		TCHAR l_sModule[MAX_PATH];
		ZeroMemory(l_sModule, sizeof(l_sModule));
		::GetModuleFileName(0, l_sModule, ArraySize(l_sModule));
		l_regStart = l_sModule;
	}
}