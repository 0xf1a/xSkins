#pragma once
#include <Windows.h>
#include <TlHelp32.h>

/*
** WinAPI process and memory functions
*/
DWORD GetProcessIdByProcessName(LPCSTR name)
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
				if (_stricmp(ProcessEntry32.szExeFile, name) == 0)
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
DWORD GetProcessIdByWindowName(LPCSTR name)
{
	DWORD dwPid = 0;

	do {
		HWND hWindow = FindWindow(NULL, name);
		if (!hWindow) { continue; }
		GetWindowThreadProcessId(hWindow, &dwPid);
		if (hWindow) { CloseHandle(hWindow); }
	} while (!dwPid);

	return dwPid;
}
DWORD GetModuleBaseAddress(DWORD pid, LPCSTR name)
{
	DWORD dwBase = 0;

	do {
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
		if (hSnapshot == INVALID_HANDLE_VALUE) { continue; }

		MODULEENTRY32 ModuleEntry32;
		ModuleEntry32.dwSize = sizeof(MODULEENTRY32);

		if (Module32First(hSnapshot, &ModuleEntry32))
		{
			do
			{
				if (_stricmp(ModuleEntry32.szModule, name) == 0)
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
DWORD GetModuleSize(DWORD pid, LPCSTR name)
{
	DWORD dwSize = 0;

	do {
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
		if (hSnapshot == INVALID_HANDLE_VALUE) { continue; }

		MODULEENTRY32 ModuleEntry32;
		ModuleEntry32.dwSize = sizeof(MODULEENTRY32);

		if (Module32First(hSnapshot, &ModuleEntry32))
		{
			do
			{
				if (_stricmp(ModuleEntry32.szModule, name) == 0)
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
PVOID ReadMem(HANDLE process, DWORD address, LPVOID buffer, DWORD size)
{
	PVOID ret = 0;
	BOOL status = ReadProcessMemory(process, (LPCVOID)(address), buffer ? buffer : &ret, size, NULL);
	return buffer ? (PVOID)status : ret;
}
BOOL WriteMem(HANDLE process, DWORD address, LPCVOID buffer, DWORD size)
{
	return WriteProcessMemory(process, (LPVOID)(address), buffer, size, NULL);
}

/*
** Offset scanning functions
*/
UCHAR GetWildcard(PBYTE pattern, UINT length, UCHAR wildcard)
{
	UCHAR wc = wildcard;

	for (UINT i = 0; i < length; i++)
	{
		if (pattern[i] == wc)
		{
			wc = GetWildcard(pattern, length, ++wc);
		}
	}

	return wc;
}
BOOLEAN CompareBytes(PBYTE bytes, PBYTE pattern, UINT length, UCHAR wildcard)
{
	for (UINT i = 0; i < length; i++)
	{
		if (pattern[i] != wildcard && bytes[i] != pattern[i])
		{
			return FALSE;
		}
	}

	return TRUE;
}
DWORD FindPattern(HANDLE process, DWORD base, DWORD size, PBYTE bytes, PBYTE pattern, UINT length,
	UCHAR wildcard, UINT offset, UINT extra, BOOLEAN relative, BOOLEAN subtract)
{
	DWORD dwAddress = 0;

	for (DWORD i = 0; i < size - length; i++)
	{
		if (CompareBytes(bytes + i, pattern, length, wildcard))
		{
			dwAddress = base + i + offset;

			if (relative)
			{
				dwAddress = (DWORD)ReadMem(process, dwAddress, NULL, sizeof(DWORD));
			}
			if (subtract)
			{
				dwAddress -= base;
			}

			dwAddress += extra;
			break;
		}

	}

	return dwAddress;
}
DWORD ChunkFindPattern(HANDLE process, DWORD base, DWORD size, DWORD chunk, PBYTE pattern, UINT length,
	UCHAR wildcard, UINT offset, UINT extra, BOOLEAN relative, BOOLEAN subtract)
{
	// both variables start from 0x1000 in order to skip PE header
	DWORD x = 0x1000; // counter variable
	DWORD i = base + 0x1000; // current virtual address
	DWORD dwAddress = 0;

	for (; i < base + size; i += chunk, x += chunk)
	{
		PBYTE pbChunk = (PBYTE)malloc(chunk); // allocate space for chunk using defined size
		if (ReadMem(process, i, pbChunk, chunk)) // read and scan through current memory chunk
		{
			dwAddress = FindPattern(process,
				i,
				chunk,
				pbChunk,
				pattern,
				length,
				wildcard,
				offset,
				extra,
				relative,
				subtract);

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
BOOL GetPropName(HANDLE process, DWORD address, PVOID buffer) // CRecvProp
{
	DWORD dwNameAddr;
	return ReadMem(process, address + 0x0, &dwNameAddr, sizeof(DWORD)) &&
		ReadMem(process, dwNameAddr, buffer, 128);
}
DWORD GetDataTable(HANDLE process, DWORD address) // CRecvProp
{
	return (DWORD)ReadMem(process, address + 0x28, NULL, sizeof(DWORD));
}
int GetOffset(HANDLE process, DWORD address) // CRecvProp
{
	return (int)ReadMem(process, address + 0x2C, NULL, sizeof(int));
}
DWORD GetPropById(HANDLE process, DWORD address, int index) // CRecvTable
{
	DWORD dwPropAddr = (DWORD)ReadMem(process, address + 0x0, NULL, sizeof(DWORD));
	return (DWORD)(dwPropAddr + 0x3C * index);
}
int GetPropCount(HANDLE process, DWORD address) // CRecvTable
{
	return (int)ReadMem(process, address + 0x4, NULL, sizeof(int));
}
BOOL GetTableName(HANDLE process, DWORD address, PVOID buffer) // CRecvTable
{
	DWORD dwNameAddr;
	return ReadMem(process, address + 0xC, &dwNameAddr, sizeof(DWORD)) &&
		ReadMem(process, dwNameAddr, buffer, 128);
}
DWORD GetTable(HANDLE process, DWORD address) // CClientClass
{
	return (DWORD)ReadMem(process, address + 0xC, NULL, sizeof(DWORD));
}
DWORD GetNextClass(HANDLE process, DWORD address) // CClientClass
{
	return (DWORD)ReadMem(process, address + 0x10, NULL, sizeof(DWORD));
}

/*
** Netvar scanning functions
*/
DWORD ScanTable(HANDLE handle, DWORD table, LPCSTR varname, DWORD level)
{
	for (int i = 0; i < GetPropCount(handle, table); i++)
	{
		DWORD dwPropAddr = GetPropById(handle, table, i);
		if (!dwPropAddr) { continue; }

		char szPropName[128] = { 0 };
		if (!GetPropName(handle, dwPropAddr, szPropName) ||
			isdigit(szPropName[0])) { continue; }

		int iOffset = GetOffset(handle, dwPropAddr);

		if (_stricmp(szPropName, varname) == 0)
		{
			return level + iOffset;
		}

		DWORD dwTableAddr = GetDataTable(handle, dwPropAddr);
		if (!dwTableAddr) { continue; }

		DWORD dwRes = ScanTable(handle, dwTableAddr, varname, level + iOffset);
		if (dwRes)
		{
			return dwRes;
		}
	}

	return 0;
}
DWORD FindNetvar(HANDLE handle, DWORD start, LPCSTR classname, LPCSTR varname)
{
	for (DWORD dwClass = start; dwClass; dwClass = GetNextClass(handle, dwClass))
	{
		DWORD dwTableAddr = GetTable(handle, dwClass);

		char szTableName[128] = { 0 };
		if (!GetTableName(handle, dwTableAddr, szTableName)) { continue; }

		if (_stricmp(szTableName, classname) == 0)
		{
			return ScanTable(handle, dwTableAddr, varname, 0);
		}
	}

	return 0;
}