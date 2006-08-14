/** ======== stripper_mm ========
 *  Copyright (C) 2005-2006 David "BAILOPAN" Anderson
 *  No warranties of any kind.
 *  Based on the original concept of Stripper2 by botman
 *
 *  License: see LICENSE.TXT
 *  ============================
 */

#include <oslink.h>
#include "stripper_mm.h"
#include "parser.h"

using namespace SourceHook;

StripperPlugin g_Plugin;

PLUGIN_EXPOSE(StripperPlugin, g_Plugin);
IVEngineServer *engine = NULL;
IServerGameDLL *server = NULL;
SourceHook::String g_mapname;
ConVar *sv_cheats = NULL;

SH_DECL_HOOK0(IVEngineServer, GetMapEntitiesString, SH_NOATTRIB, 0, const char *);
SH_DECL_HOOK6(IServerGameDLL, LevelInit, SH_NOATTRIB, 0, bool, char const *, char const *, char const *, char const *, bool, bool);

#define GET_V_IFACE(v_factory, v_var, v_type, v_name) \
	v_var = (v_type *)ismm->VInterfaceMatch(ismm->v_factory(), v_name); \
	if (!v_var) { \
		if (error && maxlen) { \
			snprintf(error, maxlen, "Could not find interface: %s", v_name); \
		} \
		return false; \
	}

bool StripperPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE(serverFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE(engineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);

	SH_ADD_HOOK_STATICFUNC(IVEngineServer, GetMapEntitiesString, engine, GetMapEntitiesString_handler, false);
	SH_ADD_HOOK_STATICFUNC(IServerGameDLL, LevelInit, server, LevelInit_handler, false);

	ismm->AddListener(this, this);

	ConCommandBaseMgr::OneTimeInit(this);

	return true;
}

void StripperPlugin::AddMapEntityFilter(const char *file)
{
	g_Stripper.ApplyFileFilter(file);
}

void *StripperPlugin::OnEngineQuery(const char *iface, int *ret)
{
	if (strcmp(iface, STRIPPER_INTERFACE) == 0)
	{
		if (ret)
			*ret = IFACE_OK;
		return (void *)(static_cast<IStripper *>(&g_Plugin));
	}

	return NULL;
}

void *StripperPlugin::OnMetamodQuery(const char *iface, int *ret)
{
	if (strcmp(iface, STRIPPER_INTERFACE)==0)
	{
		if (ret)
			*ret = IFACE_OK;
		return (void *)(static_cast<IStripper *>(&g_Plugin));
	}

	return NULL;
}

bool StripperPlugin::Unload(char *error, size_t maxlen)
{
	List<IStripperListener *>::iterator iter, end;
	for (iter=m_hooks.begin(); iter!=m_hooks.end(); iter++)
		(*iter)->Unloading();

	m_hooks.clear();

	SH_REMOVE_HOOK_STATICFUNC(IVEngineServer, GetMapEntitiesString, engine, GetMapEntitiesString_handler, false);
	SH_REMOVE_HOOK_STATICFUNC(IServerGameDLL, LevelInit, server, LevelInit_handler, false);

	return true;
}

const char *StripperPlugin::ParseAndFilter(const char *map, const char *ents)
{
	char gamedir[256], path[256];
	engine->GetGameDir(gamedir, sizeof(gamedir)-1);

	FILE *fp;

	g_Stripper.SetEntityList(ents);

	g_SMAPI->PathFormat(path, sizeof(path)-1, "%s/addons/stripper/global_filters.cfg", gamedir);
	fp = fopen(path, "rt");
	if (!fp)
		META_LOG(g_PLAPI, "Could not find global filter file: %s", path);
	else
	{
		fclose(fp);
		g_Stripper.ApplyFileFilter(path);
	}

	g_SMAPI->PathFormat(path, sizeof(path)-1, "%s/addons/stripper/maps/%s.cfg", gamedir, map);
	fp = fopen(path, "rt");
	if (fp)
	{
		fclose(fp);
		g_Stripper.ApplyFileFilter(path);
	}

	List<IStripperListener *>::iterator iter, end;
	for (iter=m_hooks.begin(); iter!=m_hooks.end(); iter++)
		(*iter)->OnMapInitialize(map);

	return g_Stripper.ToString();
}

void StripperPlugin::AddMapListener(IStripperListener *pListener)
{
	m_hooks.push_back(pListener);
}

void StripperPlugin::RemoveMapListener(IStripperListener *pListener)
{
	m_hooks.remove(pListener);
}

const char *GetMapEntitiesString_handler()
{
	RETURN_META_VALUE(MRES_SUPERCEDE, g_Stripper.ToString());
}

bool LevelInit_handler(char const *pMapName, char const *pMapEntities, char const *c, char const *d, bool e, bool f)
{
	g_mapname.assign(pMapName);
	const char *ents = g_Plugin.ParseAndFilter(g_mapname.c_str(), pMapEntities);
	RETURN_META_VALUE_NEWPARAMS(MRES_IGNORED, true, &IServerGameDLL::LevelInit, (pMapName, ents, c, d, e, f));
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
	ICvar *icvar = (ICvar *)((g_SMAPI->engineFactory())(VENGINE_CVAR_INTERFACE_VERSION, NULL));
	if (icvar)
	{
		ConCommandBase *pBase = icvar->GetCommands();
		while (pBase)
		{
			if (strcmp(pBase->GetName(), "sv_cheats")==0)
				break;
			pBase = const_cast<ConCommandBase *>(pBase->GetNext());
		}
		if (pBase)
			sv_cheats = (ConVar *)pBase;
	}
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
	return STRIPPER_VERSION;
}

const char *StripperPlugin::GetDate()
{
	return __DATE__;
}

const char *StripperPlugin::GetLogTag()
{
	return "STRIPPER";
}

bool StripperPlugin::RegisterConCommandBase(ConCommandBase *pVar)
{
	return META_REGCVAR(pVar);
}

