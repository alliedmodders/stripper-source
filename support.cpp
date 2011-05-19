/** vim: set ts=4 sw=4 et tw=99:
 * 
 * === Stripper for Metamod:Source ===
 * Copyright (C) 2005-2009 David "BAILOPAN" Anderson
 * No warranties of any kind.
 * Based on the original concept of Stripper2 by botman
 *
 * License: see LICENSE.TXT
 * ===================================
 */
#include <new>
#include "support.h"
#include "parser.h"
#if defined WIN32
#include <windows.h>
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

Stripper g_Stripper;
stripper_game_t stripper_game;

#if defined _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#elif defined __GNUC__
#if __GNUC__ == 4
#define EXPORT extern "C" __attribute__ ((visibility("default")))
#else
#define EXPORT extern "C"
#endif
#endif

static const char*
parse_map(const char* map, const char* entities)
{
    FILE *fp;
    char path[256];

    g_Stripper.SetEntityList(entities);

    stripper_game.path_format(path,
            sizeof(path),
            "%s/%s/global_filters.cfg",
            stripper_game.game_path,
            stripper_game.stripper_cfg_path);
    fp = fopen(path, "rt");
    if (fp == NULL)
    {
        stripper_game.log_message("Could not find global filter file: %s", path);
    }
    else
    {
        fclose(fp);
        g_Stripper.ApplyFileFilter(path);
    }

    stripper_game.path_format(path,
            sizeof(path),
            "%s/%s/maps/%s.cfg",
            stripper_game.game_path,
            stripper_game.stripper_cfg_path,
            map);
    fp = fopen(path, "rt");
    if (fp)
    {
        fclose(fp);
        g_Stripper.ApplyFileFilter(path);
    }

    return g_Stripper.ToString();
}

static const char*
ent_string()
{
    return g_Stripper.ToString();
}

static void
command_dump()
{
	char path[255];

	stripper_game.path_format(path,
            sizeof(path),
            "%s/%s/dumps",
            stripper_game.game_path,
            stripper_game.stripper_path);

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
	}
    else
    {
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
		stripper_game.path_format(file,
                sizeof(file),
                "%s/%s.%04d.cfg",
                path,
                stripper_game.get_map_name(),
                num);
		FILE *fp = fopen(file, "rt");
		if (!fp)
			break;
		fclose(fp);
	} while (++num);

	FILE *fp = fopen(file, "wt");
	fprintf(fp, "%s", g_Stripper.ToString());
	fclose(fp);

	stripper_game.log_message("Logged map %s to file %s", stripper_game.get_map_name(), file);
}

static void
plugin_unload()
{
}

static stripper_core_t stripper_core =
{
    parse_map,
    ent_string,
    command_dump,
    plugin_unload
};

EXPORT void
LoadStripper(const stripper_game_t* game, stripper_core_t* core)
{
    memcpy(&stripper_game, game, sizeof(stripper_game_t));
    memcpy(core, &stripper_core, sizeof(stripper_core_t));
}

/* Overload a few things to prevent libstdc++ linking */
#if defined __linux__ || defined __APPLE__
extern "C" void __cxa_pure_virtual(void)
{
}

void *operator new(size_t size)
{
	return malloc(size);
}

void *operator new[](size_t size) 
{
	return malloc(size);
}

void operator delete(void *ptr) 
{
	free(ptr);
}

void operator delete[](void * ptr)
{
	free(ptr);
}
#endif

