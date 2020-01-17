#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <tchar.h>

// Expand byte pattern
#define EXP(x) x, sizeof(x) - 1

#define nCRecvPropSize 0x3C

// CRecvProp struct offsets
#define m_pVarName 0x0
#define m_pDataTable 0x28
#define m_iOffset 0x2C

// CRecvTable struct offsets
#define m_pProps 0x0
#define m_nProps 0x4
#define m_pNetTableName 0xC

// CClientClass struct offsets
#define m_pRecvTable 0xC
#define m_pNext 0x10

/*
** WinAPI process and memory wrappers
*/
DWORD GetProcessIdByProcessName(LPCTSTR lpName)
{
	DWORD dwPid = 0;

	do
	{
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE) { continue; }

		PROCESSENTRY32 ProcessEntry32;
		ProcessEntry32.dwSize = sizeof(PROCESSENTRY32);

		if (Process32First(hSnapshot, &ProcessEntry32))
		{
			do
			{
				if (_tcsicmp(ProcessEntry32.szExeFile, lpName) == 0)
				{
					dwPid = ProcessEntry32.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnapshot, &ProcessEntry32));
		}

		CloseHandle(hSnapshot);
	} while (!dwPid);

	return dwPid;
}
DWORD GetProcessIdByWindowName(LPCTSTR lpName)
{
	DWORD dwPid = 0;

	do {
		HWND hWindow = FindWindow(NULL, lpName);
		if (!hWindow) { continue; }
		GetWindowThreadProcessId(hWindow, &dwPid);
		if (hWindow) { CloseHandle(hWindow); }
	} while (!dwPid);

	return dwPid;
}
DWORD GetModuleBaseAddress(DWORD dwPid, LPCTSTR lpName)
{
	DWORD dwBase = 0;

	do {
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwPid);
		if (hSnapshot == INVALID_HANDLE_VALUE) { continue; }

		MODULEENTRY32 ModuleEntry32;
		ModuleEntry32.dwSize = sizeof(MODULEENTRY32);

		if (Module32First(hSnapshot, &ModuleEntry32))
		{
			do
			{
				if (_tcsicmp(ModuleEntry32.szModule, lpName) == 0)
				{
					dwBase = (DWORD)ModuleEntry32.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnapshot, &ModuleEntry32));
		}

		CloseHandle(hSnapshot);
	} while (!dwBase);

	return dwBase;
}
DWORD GetModuleSize(DWORD dwPid, LPCTSTR lpName)
{
	DWORD dwSize = 0;

	do {
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwPid);
		if (hSnapshot == INVALID_HANDLE_VALUE) { continue; }

		MODULEENTRY32 ModuleEntry32;
		ModuleEntry32.dwSize = sizeof(MODULEENTRY32);

		if (Module32First(hSnapshot, &ModuleEntry32))
		{
			do
			{
				if (_tcsicmp(ModuleEntry32.szModule, lpName) == 0)
				{
					dwSize = ModuleEntry32.modBaseSize;
					break;
				}
			} while (Module32Next(hSnapshot, &ModuleEntry32));
		}

		CloseHandle(hSnapshot);
	} while (!dwSize);

	return dwSize;
}
PVOID ReadMemory(HANDLE hProcess, DWORD dwAddr, LPVOID lpBuffer, DWORD dwSize)
{
	PVOID ret = 0;
	BOOL status = ReadProcessMemory(hProcess, (LPCVOID)(dwAddr), lpBuffer ? lpBuffer : &ret, dwSize, NULL);
	return lpBuffer ? (PVOID)status : ret;
}
BOOL WriteMemory(HANDLE hProcess, DWORD dwAddr, LPCVOID lpBuffer, DWORD dwSize)
{
	return WriteProcessMemory(hProcess, (LPVOID)(dwAddr), lpBuffer, dwSize, NULL);
}

/*
** Offset scanning functions
*/
BOOLEAN CheckPattern(PBYTE pbBytes, PBYTE pbPattern, UINT uLength, UCHAR bWildcard)
{
	for (UINT i = 0; i < uLength; i++)
	{
		if (pbPattern[i] != bWildcard && pbBytes[i] != pbPattern[i])
		{
			return FALSE;
		}
	}

	return TRUE;
}
DWORD FindPattern(PBYTE pbBuffer, DWORD dwBase, DWORD dwSize, PBYTE pbPattern, UINT uLength,
	UCHAR bWildcard, UINT uOffset, UINT uExtra, BOOLEAN bRelative, BOOLEAN bSubtract)
{
	DWORD dwAddress = 0;

	for (DWORD i = 0; i < dwSize - uLength; i++)
	{
		if (CheckPattern(pbBuffer + i, pbPattern, uLength, bWildcard))
		{
			dwAddress = dwBase + i + uOffset;

			if (bRelative)
			{
				dwAddress = *(DWORD*)(pbBuffer + i + uOffset);
			}

			if (bSubtract)
			{
				dwAddress -= dwBase;
			}

			dwAddress += uExtra;
			break;
		}
	}

	return dwAddress;
}
DWORD ChunkFindPattern(HANDLE hProcess, DWORD dwBase, DWORD dwSize, DWORD dwChunkSize, PBYTE pbPattern,
	UINT uLength, UCHAR bWildcard, UINT uOffset, UINT uExtra, BOOLEAN bRelative, BOOLEAN bSubtract)
{
	// both variables start from 0x1000 in order to skip PE header
	DWORD x = 0x1000; // counter variable
	DWORD i = dwBase + 0x1000; // current virtual address
	DWORD dwAddress = 0;

	for (; i < dwBase + dwSize; i += dwChunkSize, x += dwChunkSize)
	{
		PBYTE pbChunk = (PBYTE)malloc(dwChunkSize); // allocate space for chunk using defined size
		if (ReadMemory(hProcess, i, pbChunk, dwChunkSize)) // read and scan through current memory chunk
		{
			dwAddress = FindPattern(pbChunk,
				i,
				dwChunkSize,
				pbPattern,
				uLength,
				bWildcard,
				uOffset,
				uExtra,
				bRelative,
				bSubtract);

			if (dwAddress)
			{
				dwAddress += x;
			}
		}
		free(pbChunk);

		if (dwAddress) { break; }
	}

	return dwAddress;
}

/*
** Reversed Source SDK netvar classes
*/
BOOL GetPropName(HANDLE hProcess, DWORD dwAddress, PVOID pBuffer)
{
	DWORD dwNameAddr;
	return ReadMemory(hProcess, dwAddress + m_pVarName, &dwNameAddr, sizeof(DWORD)) &&
		ReadMemory(hProcess, dwNameAddr, pBuffer, 128);
}
DWORD GetDataTable(HANDLE hProcess, DWORD dwAddress)
{
	return (DWORD)ReadMemory(hProcess, dwAddress + m_pDataTable, NULL, sizeof(DWORD));
}
int GetOffset(HANDLE hProcess, DWORD dwAddress)
{
	return (int)ReadMemory(hProcess, dwAddress + m_iOffset, NULL, sizeof(int));
}
DWORD GetPropById(HANDLE hProcess, DWORD dwAddress, int iIndex)
{
	DWORD dwPropAddr = (DWORD)ReadMemory(hProcess, dwAddress + m_pProps, NULL, sizeof(DWORD));
	return (DWORD)(dwPropAddr + nCRecvPropSize * iIndex);
}
int GetPropCount(HANDLE hProcess, DWORD dwAddress)
{
	return (int)ReadMemory(hProcess, dwAddress + m_nProps, NULL, sizeof(int));
}
BOOL GetTableName(HANDLE hProcess, DWORD dwAddress, PVOID pBuffer)
{
	DWORD dwNameAddr;
	return ReadMemory(hProcess, dwAddress + m_pNetTableName, &dwNameAddr, sizeof(DWORD)) &&
		ReadMemory(hProcess, dwNameAddr, pBuffer, 128);
}
DWORD GetTable(HANDLE hProcess, DWORD dwAddress)
{
	return (DWORD)ReadMemory(hProcess, dwAddress + m_pRecvTable, NULL, sizeof(DWORD));
}
DWORD GetNextClass(HANDLE hProcess, DWORD dwAddress)
{
	return (DWORD)ReadMemory(hProcess, dwAddress + m_pNext, NULL, sizeof(DWORD));
}

/*
** Netvar scanning functions
*/
DWORD ScanTable(HANDLE hProcess, DWORD dwTableAddr, LPCSTR lpVarName, DWORD dwLevel)
{
	for (int i = 0; i < GetPropCount(hProcess, dwTableAddr); i++)
	{
		DWORD dwPropAddr = GetPropById(hProcess, dwTableAddr, i);
		if (!dwPropAddr) { continue; }

		char szPropName[128] = { 0 };
		if (!GetPropName(hProcess, dwPropAddr, szPropName) || isdigit(szPropName[0]))
		{
			continue;
		}

		int iOffset = GetOffset(hProcess, dwPropAddr);

		if (_stricmp(szPropName, lpVarName) == 0)
		{
			return dwLevel + iOffset;
		}

		DWORD dwTableAddr = GetDataTable(hProcess, dwPropAddr);
		if (!dwTableAddr) { continue; }

		DWORD dwResult = ScanTable(hProcess, dwTableAddr, lpVarName, dwLevel + iOffset);
		if (dwResult)
		{
			return dwResult;
		}
	}

	return 0;
}
DWORD FindNetvar(HANDLE hProcess, DWORD dwStart, LPCSTR lpClassName, LPCSTR lpVarName)
{
	for (DWORD dwClass = dwStart; dwClass; dwClass = GetNextClass(hProcess, dwClass))
	{
		DWORD dwTableAddr = GetTable(hProcess, dwClass);

		char szTableName[128] = { 0 };
		if (!GetTableName(hProcess, dwTableAddr, szTableName)) { continue; }

		if (_stricmp(szTableName, lpClassName) == 0)
		{
			return ScanTable(hProcess, dwTableAddr, lpVarName, 0);
		}
	}

	return 0;
}