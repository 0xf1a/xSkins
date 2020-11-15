#include <stdio.h>
#include "xLiteMem.h"
#include "xSkins.h"

HANDLE hProcess = NULL;

DWORD dwClientState = 0;
DWORD dwLocalPlayer = 0;
DWORD dwEntityList = 0;
DWORD m_hViewModel = 0;
DWORD m_iViewModelIndex = 0;
DWORD m_flFallbackWear = 0;
DWORD m_nFallbackPaintKit = 0;
DWORD m_iItemIDHigh = 0;
DWORD m_iEntityQuality = 0;
DWORD m_iItemDefinitionIndex = 0;
DWORD m_hActiveWeapon = 0;
DWORD m_hMyWeapons = 0;
DWORD m_nModelIndex = 0;
DWORD m_dwModelPrecache = 0;

UINT GetModelIndexByName(const char* modelName)
{
	DWORD cstate = (DWORD)ReadMemory(hProcess, dwClientState, NULL, sizeof(DWORD));

	// CClientState + 0x529C -> INetworkStringTable* m_pModelPrecacheTable
	DWORD nst = (DWORD)ReadMemory(hProcess, cstate + m_dwModelPrecache, NULL, sizeof(DWORD));

	// INetworkStringTable + 0x40 -> INetworkStringDict* m_pItems
	DWORD nsd = (DWORD)ReadMemory(hProcess, nst + 0x40, NULL, sizeof(DWORD));

	// INetworkStringDict + 0xC -> void* m_pItems
	DWORD nsdi = (DWORD)ReadMemory(hProcess, nsd + 0xC, NULL, sizeof(DWORD));

	for (UINT i = 0; i < 1024; i++)
	{
		DWORD nsdi_i = (DWORD)ReadMemory(hProcess, nsdi + 0xC + i * 0x34, NULL, sizeof(DWORD));
		char str[128] = { 0 };
		if (ReadMemory(hProcess, nsdi_i, str, sizeof(str)))
		{
			if (_stricmp(str, modelName) == 0)
			{
				return i;
			}
		}
	}

	return 0;
}
UINT GetModelIndex(const short itemIndex)
{
	UINT ret = 0;
	switch (itemIndex)
	{
	case WEAPON_KNIFE:
		ret = GetModelIndexByName("models/weapons/v_knife_default_ct.mdl");
		break;
	case WEAPON_KNIFE_T:
		ret = GetModelIndexByName("models/weapons/v_knife_default_t.mdl");
		break;
	case WEAPON_KNIFE_BAYONET:
		ret = GetModelIndexByName("models/weapons/v_knife_bayonet.mdl");
		break;
	case WEAPON_KNIFE_FLIP:
		ret = GetModelIndexByName("models/weapons/v_knife_flip.mdl");
		break;
	case WEAPON_KNIFE_GUT:
		ret = GetModelIndexByName("models/weapons/v_knife_gut.mdl");
		break;
	case WEAPON_KNIFE_KARAMBIT:
		ret = GetModelIndexByName("models/weapons/v_knife_karam.mdl");
		break;
	case WEAPON_KNIFE_M9_BAYONET:
		ret = GetModelIndexByName("models/weapons/v_knife_m9_bay.mdl");
		break;
	case WEAPON_KNIFE_TACTICAL:
		ret = GetModelIndexByName("models/weapons/v_knife_tactical.mdl");
		break;
	case WEAPON_KNIFE_FALCHION:
		ret = GetModelIndexByName("models/weapons/v_knife_falchion_advanced.mdl");
		break;
	case WEAPON_KNIFE_SURVIVAL_BOWIE:
		ret = GetModelIndexByName("models/weapons/v_knife_survival_bowie.mdl");
		break;
	case WEAPON_KNIFE_BUTTERFLY:
		ret = GetModelIndexByName("models/weapons/v_knife_butterfly.mdl");
		break;
	case WEAPON_KNIFE_PUSH:
		ret = GetModelIndexByName("models/weapons/v_knife_push.mdl");
		break;
	case WEAPON_KNIFE_URSUS:
		ret = GetModelIndexByName("models/weapons/v_knife_ursus.mdl");
		break;
	case WEAPON_KNIFE_GYPSY_JACKKNIFE:
		ret = GetModelIndexByName("models/weapons/v_knife_gypsy_jackknife.mdl");
		break;
	case WEAPON_KNIFE_STILETTO:
		ret = GetModelIndexByName("models/weapons/v_knife_stiletto.mdl");
		break;
	case WEAPON_KNIFE_WIDOWMAKER:
		ret = GetModelIndexByName("models/weapons/v_knife_widowmaker.mdl");
		break;
	case WEAPON_KNIFE_CSS:
		ret = GetModelIndexByName("models/weapons/v_knife_css.mdl");
		break;
	case WEAPON_KNIFE_CORD:
		ret = GetModelIndexByName("models/weapons/v_knife_cord.mdl");
		break;
	case WEAPON_KNIFE_CANIS:
		ret = GetModelIndexByName("models/weapons/v_knife_canis.mdl");
		break;
	case WEAPON_KNIFE_OUTDOOR:
		ret = GetModelIndexByName("models/weapons/v_knife_outdoor.mdl");
		break;
	case WEAPON_KNIFE_SKELETON:
		ret = GetModelIndexByName("models/weapons/v_knife_skeleton.mdl");
		break;
	default:
		break;
	}
	return ret;
}
UINT GetWeaponSkin(const short itemIndex)
{
	// set your desired weapon skin values here
	UINT paint = 0;
	switch (itemIndex)
	{
	case WEAPON_DEAGLE:
		paint = 711;
		break;
	case WEAPON_GLOCK:
		paint = 38;
		break;
	case WEAPON_AK47:
		paint = 180;
		break;
	case WEAPON_AWP:
		paint = 344;
		break;
	case WEAPON_M4A1:
		paint = 309;
		break;
	case WEAPON_SSG08:
		paint = 222;
		break;
	case WEAPON_M4A1_SILENCER:
		paint = 445;
		break;
	case WEAPON_USP_SILENCER:
		paint = 653;
		break;
	default:
		break;
	}
	return paint;
}

UINT LoadSkins(const char* file, char*** names, UINT** values)
{
	FILE* fp;
	UINT i = 0;

	// make sure this txt file is encoded as ANSI
	if (fopen_s(&fp, file, "r") == 0)
	{
		char line[64];
		while (fgets(line, sizeof(line), fp))
		{
			// use this for verifying & splitting lines
			char* pch = strstr(line, ": ");
			if (!pch) { continue; }

			// remove trailing newline char
			size_t len = strlen(line) - 1;
			if (line[len] == '\n')
			{
				line[len] = '\0';
			}

			*values = (UINT*)realloc(*values, (i + 1) * sizeof(UINT));
			(*values)[i] = atoi(line);

			*names = (char**)realloc(*names, (i + 1) * sizeof(char*));
			(*names)[i] = _strdup(pch + 2);

			i++;
		}

		if (fp) { fclose(fp); }
	}

	return i;
}
void SortSkins(UINT count, char*** names, UINT** values)
{
	UINT vtmp = 0;
	char* ntmp = NULL;

	// use bubble sort algorithm to sort skins alphabetically
	// start from 1 to keep vanilla skin as the first choice
	for (UINT i = 1; i < count; i++)
	{
		for (UINT j = 1; j < count; j++)
		{
			if (strcmp((*names)[i], (*names)[j]) < 0)
			{
				vtmp = (*values)[i];
				ntmp = (*names)[i];

				(*values)[i] = (*values)[j];
				(*names)[i] = (*names)[j];

				(*values)[j] = vtmp;
				(*names)[j] = ntmp;
			}
		}
	}
}

void PrintMenu(const char* title, char** name, UINT sz, UINT x)
{
	printf("%s %c %s %c\t\t\t\r", title, x > 0 ? '<' : '|', name[x], x < sz ? '>' : '|');
	Sleep(sz < 20 ? 150 : 35);
}
UINT ItemSelect(const char* title, char** name, UINT sz)
{
	UINT x = 0; // index of current item
	PrintMenu(title, name, sz, x);

	while (!GetAsyncKeyState(VK_RETURN))
	{
		if (GetAsyncKeyState(VK_RIGHT) && x < sz)
		{
			PrintMenu(title, name, sz, ++x);
		}
		else if (GetAsyncKeyState(VK_LEFT) && x > 0)
		{
			PrintMenu(title, name, sz, --x);
		}
	}

	printf("%s %s\t\t\t\n", title, name[x]);
	Sleep(50);
	return x;
}

void xSkins(const short knifeIndex, const UINT knifeSkin)
{
	const int itemIDHigh = -1;
	const int entityQuality = 3;
	const float fallbackWear = 0.0001f;

	UINT modelIndex = 0;
	DWORD localPlayer = 0;

	while (!GetAsyncKeyState(VK_F6))
	{
		// model index is different for each server and map
		// below is a simple way to keep track of local base in order to reset model index
		// while also avoiding doing unnecessary extra reads because of the external RPM overhead
		DWORD tempPlayer = (DWORD)ReadMemory(hProcess, dwLocalPlayer, NULL, sizeof(DWORD));
		if (!tempPlayer) // client not connected to any server (works most of the time)
		{
			modelIndex = 0;
			continue;
		}
		else if (tempPlayer != localPlayer) // local base changed (new server join/demo record)
		{
			localPlayer = tempPlayer;
			modelIndex = 0;
		}

		while (!modelIndex)
		{
			modelIndex = GetModelIndex(knifeIndex);
		}

		// loop through m_hMyWeapons slots (8 will be enough)
		for (UINT i = 0; i < 8; i++)
		{
			// get entity of weapon in current slot
			DWORD currentWeapon = (DWORD)ReadMemory(hProcess, localPlayer + m_hMyWeapons + i * 0x4, NULL, sizeof(DWORD)) & 0xfff;
			currentWeapon = (DWORD)ReadMemory(hProcess, dwEntityList + (currentWeapon - 1) * 0x10, NULL, sizeof(DWORD));
			if (!currentWeapon) { continue; }

			short weaponIndex = (short)ReadMemory(hProcess, currentWeapon + m_iItemDefinitionIndex, NULL, sizeof(short));
			UINT weaponSkin = GetWeaponSkin(weaponIndex);

			// for knives, set item and model related properties
			if (weaponIndex == WEAPON_KNIFE || weaponIndex == WEAPON_KNIFE_T || weaponIndex == knifeIndex)
			{
				WriteMemory(hProcess, currentWeapon + m_iItemDefinitionIndex, &knifeIndex, sizeof(short));
				WriteMemory(hProcess, currentWeapon + m_nModelIndex, &modelIndex, sizeof(UINT));
				WriteMemory(hProcess, currentWeapon + m_iViewModelIndex, &modelIndex, sizeof(UINT));
				WriteMemory(hProcess, currentWeapon + m_iEntityQuality, &entityQuality, sizeof(int));
				weaponSkin = knifeSkin;
			}

			if (weaponSkin) // set skin properties
			{
				WriteMemory(hProcess, currentWeapon + m_iItemIDHigh, &itemIDHigh, sizeof(int));
				WriteMemory(hProcess, currentWeapon + m_nFallbackPaintKit, &weaponSkin, sizeof(UINT));
				WriteMemory(hProcess, currentWeapon + m_flFallbackWear, &fallbackWear, sizeof(float));
			}
		}

		// get entity of weapon in our hands
		DWORD activeWeapon = (DWORD)ReadMemory(hProcess, localPlayer + m_hActiveWeapon, NULL, sizeof(DWORD)) & 0xfff;
		activeWeapon = (DWORD)ReadMemory(hProcess, dwEntityList + (activeWeapon - 1) * 0x10, NULL, sizeof(DWORD));
		if (!activeWeapon) { continue; }

		short weaponIndex = (short)ReadMemory(hProcess, activeWeapon + m_iItemDefinitionIndex, NULL, sizeof(short));
		if (weaponIndex != knifeIndex) { continue; } // skip if current weapon is not already set to chosen knife

		// get viewmodel entity
		DWORD activeViewModel = (DWORD)ReadMemory(hProcess, localPlayer + m_hViewModel, NULL, sizeof(DWORD)) & 0xfff;
		activeViewModel = (DWORD)ReadMemory(hProcess, dwEntityList + (activeViewModel - 1) * 0x10, NULL, sizeof(DWORD));
		if (!activeViewModel) { continue; }

		WriteMemory(hProcess, activeViewModel + m_nModelIndex, &modelIndex, sizeof(UINT));
	}
}

int main()
{
	printf("[xSkins] External Knife & Skin Changer\n");

	char** skinNames = 0;
	UINT* skinIDs = 0;

	UINT count = LoadSkins("skins.txt", &skinNames, &skinIDs);
	if (!count || !skinNames || !skinIDs)
	{
		printf("[!] Error loading skins from file!\n");
		return 1;
	}

	printf("[+] Loaded %d skins from file\n", count);
	SortSkins(count, &skinNames, &skinIDs);

	UINT knifeID = ItemSelect("[+] Knife model:", knifeNames, sizeof(knifeIDs) / sizeof(knifeIDs[0]) - 1);
	UINT skinID = ItemSelect("[+] Knife skin:", skinNames, count - 1);
	skinID = skinIDs[skinID];

	free(skinNames);
	free(skinIDs);

	DWORD dwProcessId = GetProcessIdByProcessName(_T("csgo.exe"));
	printf("[+] csgo.exe process id: %u\n", dwProcessId);

	DWORD dwClientBase = GetModuleBaseAddress(dwProcessId, _T("client.dll"));
	printf("[+] client.dll base: 0x%x\n", dwClientBase);

	DWORD dwClientSize = GetModuleSize(dwProcessId, _T("client.dll"));
	printf("[+] client.dll size: 0x%x\n", dwClientSize);

	DWORD dwEngineBase = GetModuleBaseAddress(dwProcessId, _T("engine.dll"));
	printf("[+] engine.dll base: 0x%x\n", dwEngineBase);

	DWORD dwEngineSize = GetModuleSize(dwProcessId, _T("engine.dll"));
	printf("[+] engine.dll size: 0x%x\n", dwEngineSize);

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
	if (hProcess == INVALID_HANDLE_VALUE)
	{
		printf("[!] Error opening handle to CSGO!\n");
		return 1;
	}

	PBYTE pbEngine = (PBYTE)malloc(dwEngineSize);
	if (ReadMemory(hProcess, dwEngineBase, pbEngine, dwEngineSize))
	{
		dwClientState = FindPattern(pbEngine,
			dwEngineBase,
			dwEngineSize,
			EXP("\xA1\xAA\xAA\xAA\xAA\x33\xD2\x6A\x00\x6A\x00\x33\xC9\x89\xB0"),
			0xAA,
			0x1,
			0x0,
			TRUE,
			FALSE);
		printf("[+] dwClientState: 0x%x\n", dwClientState);

		m_dwModelPrecache = FindPattern(pbEngine,
			dwEngineBase,
			dwEngineSize,
			EXP("\x0C\x3B\x81\xAA\xAA\xAA\xAA\x75\x11\x8B\x45\x10\x83\xF8\x01\x7C\x09\x50\x83"),
			0xAA,
			0x3,
			0x0,
			TRUE,
			FALSE);
		printf("[+] m_dwModelPrecache: 0x%x\n", m_dwModelPrecache);
	}
	free(pbEngine);

	PBYTE pbClient = (PBYTE)malloc(dwClientSize);
	if (ReadMemory(hProcess, dwClientBase, pbClient, dwClientSize))
	{
		dwEntityList = FindPattern(pbClient,
			dwClientBase,
			dwClientSize,
			EXP("\xBB\xAA\xAA\xAA\xAA\x83\xFF\x01\x0F\x8C\xAA\xAA\xAA\xAA\x3B\xF8"),
			0xAA,
			0x1,
			0x0,
			TRUE,
			FALSE);
		printf("[+] dwEntityList: 0x%x\n", dwEntityList);

		dwLocalPlayer = FindPattern(pbClient,
			dwClientBase,
			dwClientSize,
			EXP("\x8D\x34\x85\xAA\xAA\xAA\xAA\x89\x15\xAA\xAA\xAA\xAA\x8B\x41\x08\x8B\x48\x04\x83\xF9\xFF"),
			0xAA,
			0x3,
			0x4,
			TRUE,
			FALSE);
		printf("[+] dwLocalPlayer: 0x%x\n", dwLocalPlayer);

		DWORD dwGetAllClasses = FindPattern(pbClient,
			dwClientBase,
			dwClientSize,
			EXP("\x44\x54\x5F\x54\x45\x57\x6F\x72\x6C\x64\x44\x65\x63\x61\x6C"),
			0xAA,
			0x0,
			0x0,
			FALSE,
			FALSE);

		dwGetAllClasses = FindPattern(pbClient,
			dwClientBase,
			dwClientSize,
			(PBYTE)&dwGetAllClasses,
			sizeof(PBYTE),
			0x0,
			0x2B,
			0x0,
			TRUE,
			FALSE);

		m_hViewModel = FindNetvar(hProcess, dwGetAllClasses, "DT_BasePlayer", "m_hViewModel[0]");
		printf("[+] m_hViewModel: 0x%x\n", m_hViewModel);

		m_iViewModelIndex = FindNetvar(hProcess, dwGetAllClasses, "DT_BaseCombatWeapon", "m_iViewModelIndex");
		printf("[+] m_iViewModelIndex: 0x%x\n", m_iViewModelIndex);

		m_flFallbackWear = FindNetvar(hProcess, dwGetAllClasses, "DT_BaseAttributableItem", "m_flFallbackWear");
		printf("[+] m_flFallbackWear: 0x%x\n", m_flFallbackWear);

		m_nFallbackPaintKit = FindNetvar(hProcess, dwGetAllClasses, "DT_BaseAttributableItem", "m_nFallbackPaintKit");
		printf("[+] m_nFallbackPaintKit: 0x%x\n", m_nFallbackPaintKit);

		m_iItemIDHigh = FindNetvar(hProcess, dwGetAllClasses, "DT_BaseAttributableItem", "m_iItemIDHigh");
		printf("[+] m_iItemIDHigh: 0x%x\n", m_iItemIDHigh);

		m_iEntityQuality = FindNetvar(hProcess, dwGetAllClasses, "DT_BaseAttributableItem", "m_iEntityQuality");
		printf("[+] m_iEntityQuality: 0x%x\n", m_iEntityQuality);

		m_iItemDefinitionIndex = FindNetvar(hProcess, dwGetAllClasses, "DT_BaseAttributableItem", "m_iItemDefinitionIndex");
		printf("[+] m_iItemDefinitionIndex: 0x%x\n", m_iItemDefinitionIndex);

		m_hActiveWeapon = FindNetvar(hProcess, dwGetAllClasses, "DT_BaseCombatCharacter", "m_hActiveWeapon");
		printf("[+] m_hActiveWeapon: 0x%x\n", m_hActiveWeapon);

		m_hMyWeapons = FindNetvar(hProcess, dwGetAllClasses, "DT_BaseCombatCharacter", "m_hMyWeapons");
		printf("[+] m_hMyWeapons: 0x%x\n", m_hMyWeapons);

		m_nModelIndex = FindNetvar(hProcess, dwGetAllClasses, "DT_BaseViewModel", "m_nModelIndex");
		printf("[+] m_nModelIndex: 0x%x\n", m_nModelIndex);
	}
	free(pbClient);

	if (dwClientState &&
		dwLocalPlayer &&
		dwEntityList &&
		m_hViewModel &&
		m_iViewModelIndex &&
		m_flFallbackWear &&
		m_nFallbackPaintKit &&
		m_iItemIDHigh &&
		m_iEntityQuality &&
		m_iItemDefinitionIndex &&
		m_hActiveWeapon &&
		m_hMyWeapons &&
		m_nModelIndex &&
		m_dwModelPrecache)
	{
		xSkins(knifeIDs[knifeID], skinID);
	}

	CloseHandle(hProcess);
	return 0;
}
