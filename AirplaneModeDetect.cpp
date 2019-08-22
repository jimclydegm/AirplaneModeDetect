// AirplaneModeDetect.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <atlpath.h>

// AirplaneMode
#define REG_AIRPLANE_MODE_SETTINGS		_T("SYSTEM\\CurrentControlSet\\Control\\RadioManagement\\SystemRadioState")	

bool RegQueryDWORD(HKEY hKeyParent, LPCTSTR lpszKeyName, LPCTSTR lpszValueName, DWORD &value);

void main()
{
	DWORD  dwFilter = REG_NOTIFY_CHANGE_NAME |
		REG_NOTIFY_CHANGE_ATTRIBUTES |
		REG_NOTIFY_CHANGE_LAST_SET |
		REG_NOTIFY_CHANGE_SECURITY;

	DWORD value;

	HANDLE hEvent;
	HKEY   hMainKey;
	HKEY   hKey;
	LONG   lErrorCode;
	hMainKey = HKEY_LOCAL_MACHINE;
	bool airplaneMode;

	// Open a key.
	lErrorCode = RegOpenKeyEx(hMainKey, REG_AIRPLANE_MODE_SETTINGS, 0, KEY_NOTIFY, &hKey);
	if (lErrorCode != ERROR_SUCCESS)
	{
		_tprintf(TEXT("Error in RegOpenKeyEx (%d).\n"), lErrorCode);
		return;
	}

	// Create an event.
	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEvent == NULL)
	{
		_tprintf(TEXT("Error in CreateEvent (%d).\n"), GetLastError());
		return;
	}

	// wait for the end of this session
	HANDLE hToWait[] = {
		hEvent,
	};

	// Watch the registry key for a change of value.
	lErrorCode = RegNotifyChangeKeyValue(hKey,
		TRUE,
		dwFilter,
		hEvent,
		TRUE);
	if (lErrorCode != ERROR_SUCCESS)
	{
		_tprintf(TEXT("Error in RegNotifyChangeKeyValue (%d).\n"), lErrorCode);
		return;
	}

	// Wait for an event to occur.
	_tprintf(TEXT("Try to toggle Airplane mode ON/OFF...\n"));
	if (WaitForMultipleObjects(ARRAYSIZE(hToWait), hToWait, false, INFINITE) != WAIT_FAILED)
	{
		airplaneMode = RegQueryDWORD(HKEY_LOCAL_MACHINE, REG_AIRPLANE_MODE_SETTINGS, _T("") /*(Default)*/, value) && value != 0;
		_tprintf(TEXT("\nAirplane mode changed. %s\n"), airplaneMode ? TEXT("Enabled") : TEXT("DISABLED"));

	}
	else
	{
		_tprintf(TEXT("\nError in WaitForMultipleObjects (%d).\n"), GetLastError());
	}

	// Close the key.
	lErrorCode = RegCloseKey(hKey);
	if (lErrorCode != ERROR_SUCCESS)
	{
		_tprintf(TEXT("Error in RegCloseKey (%d).\n"), GetLastError());
		return;
	}

	// Close the handle.
	if (!CloseHandle(hEvent))
	{
		_tprintf(TEXT("Error in CloseHandle.\n"));
		return;
	}
}

bool RegQueryDWORD(HKEY hKeyParent, LPCTSTR lpszKeyName, LPCTSTR lpszValueName, DWORD &value)
{
	CRegKey key;
	return key.Open(hKeyParent, lpszKeyName, KEY_READ) == ERROR_SUCCESS &&
		key.QueryDWORDValue(lpszValueName, value) == ERROR_SUCCESS;
}