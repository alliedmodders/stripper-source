/** ======== stripper_mm ========
 *  Copyright (C) 2005-2006 David "BAILOPAN" Anderson
 *  No warranties of any kind.
 *  Based on the original concept of Stripper2 by botman
 *
 *  License: see LICENSE.TXT
 *  ============================
 */

#include "stripper_mm.h"
#include "parser.h"
#if defined WIN32
#include <windows.h>
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

ConVar stripper_version("stripper_version", STRIPPER_VERSION, FCVAR_SPONLY|FCVAR_REPLICATED|FCVAR_NOTIFY, "Stripper Version");

CON_COMMAND(stripper_dump, "Dumps the map entity list to a file")
{
	char path[255];
	char gamedir[64];

	engine->GetGameDir(gamedir, sizeof(gamedir)-1);
	g_SMAPI->PathFormat(path, sizeof(path)-1, "%s/addons/stripper/dumps", gamedir);

#ifdef WIN32
	DWORD attr = GetFileAttributes(path);
	if ( (attr = INVALID_FILE_ATTRIBUTES) || (!(attr & FILE_ATTRIBUTE_DIRECTORY)) )
	{
		unlink(path);
		mkdir(path);
	}
#else
	struct stat s;
	
	if (stat(path, &s) != 0)
	{
		mkdir(path, 0775);
	} else {
		if (!(S_ISDIR(s.st_mode)))
		{
			unlink(path);
			mkdir(path, 0775);
		}
	}
#endif

	int num = 0;
	char file[255];
	do
	{
		g_SMAPI->PathFormat(file, sizeof(file)-1, "%s/%s.%04d.cfg", path, g_mapname.c_str(), num);
		FILE *fp = fopen(file, "rt");
		if (!fp)
			break;
		fclose(fp);
	} while (++num);

	FILE *fp = fopen(file, "wt");
	fprintf(fp, "%s", g_Stripper.ToString());
	fclose(fp);

	META_LOG(g_PLAPI, "Logged map %s to file %s", g_mapname.c_str(), file);
}

