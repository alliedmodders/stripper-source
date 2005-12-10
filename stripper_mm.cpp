/* ======== stub_mm ========
* Copyright (C) 2004-2005 Metamod:Source Development Team
* No warranties of any kind
*
* License: zlib/libpng
*
* Author(s): David "BAILOPAN" Anderson
* ============================
*/

#include <oslink.h>
#include "stripper_mm.h"
#include "parser.h"

StripperPlugin g_Plugin;

PLUGIN_EXPOSE(StripperPlugin, g_Plugin);
IVEngineServer *engine = NULL;
IServerGameDLL *server = NULL;
SourceHook::CallClass<IVEngineServer> *enginepatch = NULL;
SourceHook::CallClass<IServerGameDLL> *serverpatch = NULL;
SourceHook::String g_mapname;

//This has all of the necessary hook declarations.  Read it!
#include "meta_hooks.h"

bool StripperPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	char iface_buffer[255];
	int num=0;

	strcpy(iface_buffer, INTERFACEVERSION_SERVERGAMEDLL);
	FIND_IFACE(serverFactory, server, num, iface_buffer, IServerGameDLL *);
	strcpy(iface_buffer, INTERFACEVERSION_VENGINESERVER);
	FIND_IFACE(engineFactory, engine, num, iface_buffer, IVEngineServer *);

	SH_ADD_HOOK_STATICFUNC(IVEngineServer, GetMapEntitiesString, engine, GetMapEntitiesString_handler, false);
	SH_ADD_HOOK_STATICFUNC(IServerGameDLL, LevelInit, server, LevelInit_handler, false);

	enginepatch = SH_GET_CALLCLASS(engine);
	serverpatch = SH_GET_CALLCLASS(server);

	return true;
}

bool StripperPlugin::Unload(char *error, size_t maxlen)
{
	SH_RELEASE_CALLCLASS(serverpatch);
	SH_RELEASE_CALLCLASS(enginepatch);

	SH_REMOVE_HOOK_STATICFUNC(IVEngineServer, GetMapEntitiesString, engine, GetMapEntitiesString_handler, false);
	SH_REMOVE_HOOK_STATICFUNC(IServerGameDLL, LevelInit, server, LevelInit_handler, false);

	return true;
}

const char *StripperPlugin::ParseAndFilter(const char *map, const char *ents)
{
	char gamedir[24], path[255];
	engine->GetGameDir(gamedir, sizeof(gamedir)-1);

	FILE *fp;

	g_Stripper.SetEntityList(ents);

	UTIL_PathFmt(path, sizeof(path)-1, "%s/addons/stripper/global_filters.cfg", gamedir);
	fp = fopen(path, "rt");
	if (!fp)
		META_LOG(g_PLAPI, "Could not find global filter file: %s", path);
	else
	{
		fclose(fp);
		g_Stripper.ApplyFileFilter(path);
	}

	UTIL_PathFmt(path, sizeof(path)-1, "%s/addons/stripper/maps/%s.cfg", gamedir, map);
	fp = fopen(path, "rt");
	if (fp)
	{
		fclose(fp);
		g_Stripper.ApplyFileFilter(path);
	}

	return g_Stripper.ToString();
}

const char *GetMapEntitiesString_handler()
{
	RETURN_META_VALUE(MRES_SUPERCEDE, g_Stripper.ToString());
}

bool LevelInit_handler(char const *pMapName, char const *pMapEntities, char const *c, char const *d, bool e, bool f)
{
	g_mapname.assign(pMapName);
	const char *ents = g_Plugin.ParseAndFilter(g_mapname.c_str(), pMapEntities);
	bool result = DLLCALL(LevelInit)(pMapName, ents, c, d, e, f);

	RETURN_META_VALUE(MRES_SUPERCEDE, result);
}

bool StripperPlugin::Pause(char *error, size_t maxlen)
{
	return true;
}

bool StripperPlugin::Unpause(char *error, size_t maxlen)
{
	return true;
}

void StripperPlugin::AllPluginsLoaded()
{
}

const char *StripperPlugin::GetAuthor()
{
	return "BAILOPAN";
}

const char *StripperPlugin::GetName()
{
	return "Stripper";
}

const char *StripperPlugin::GetDescription()
{
	return "Strips/Adds Map Entities";
}

const char *StripperPlugin::GetURL()
{
	return "http://www.bailopan.net/";
}

const char *StripperPlugin::GetLicense()
{
	return "LICENSE.TXT";
}

const char *StripperPlugin::GetVersion()
{
	return "1.00";
}

const char *StripperPlugin::GetDate()
{
	return __DATE__;
}

const char *StripperPlugin::GetLogTag()
{
	return "STRIPPER";
}

/** 
 * Formats a path name for an OS
 */
void UTIL_PathFmt(char *buffer, size_t len, const char *fmt, ...)
{
	va_list ap;
	va_start(ap,fmt);
	size_t mylen = vsnprintf(buffer, len, fmt, ap);
	va_end(ap);

	for (size_t i=0; i<mylen; i++)
	{
		if (buffer[i] == ALT_SEP_CHAR)
			buffer[i] = PATH_SEP_CHAR;
	}
}
