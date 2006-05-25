/** ======== stripper_mm ========
 *  Copyright (C) 2005 David "BAILOPAN" Anderson
 *  No warranties of any kind.
 *  Based on the original concept of Stripper2 by botman
 *
 *  License: see LICENSE.TXT
 *  ============================
 */

#ifndef _INCLUDE_SAMPLEPLUGIN_H
#define _INCLUDE_SAMPLEPLUGIN_H

#include <ISmmPlugin.h>
#include <sh_string.h>
#include <sh_list.h>
#include "IStripper.h"

#define STRIPPER_VERSION		"1.02"

class StripperPlugin : 
	public ISmmPlugin, 
	public IStripper,
	public IMetamodListener,
	public IConCommandBaseAccessor
{
public:
	friend bool LevelInit_handler(char const *pMapName, char const *pMapEntities, char const *c, char const *d, bool e, bool f);
	friend const char *GetMapEntitiesString_handler();
public:
	bool Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late);
	bool Unload(char *error, size_t maxlen);
	bool Pause(char *error, size_t maxlen);
	bool Unpause(char *error, size_t maxlen);
	void AllPluginsLoaded();
public:
	const char *GetAuthor();
	const char *GetName();
	const char *GetDescription();
	const char *GetURL();
	const char *GetLicense();
	const char *GetVersion();
	const char *GetDate();
	const char *GetLogTag();
public:	//IStripper
	void AddMapListener(IStripperListener *pListener);
	void RemoveMapListener(IStripperListener *pListener);
	void AddMapEntityFilter(const char *file);
public:	//IMetamodListener
	void *OnEngineQuery(const char *iface, int *ret);
	void *OnMetamodQuery(const char *iface, int *ret);
public:	//IConCommandBaseAccessor
	bool RegisterConCommandBase(ConCommandBase *pVar);
private:
	const char *ParseAndFilter(const char *map, const char *ents);
private:
	SourceHook::List<IStripperListener *> m_hooks;
};

extern IVEngineServer *engine;
extern IServerGameDLL *server;
//unused
extern SourceHook::String g_mapname;
extern ConVar *sv_cheats;

extern StripperPlugin g_Plugin;

PLUGIN_GLOBALVARS();

#define	FIND_IFACE(func, assn_var, num_var, name, type) \
	do { \
		if ( (assn_var=(type)((ismm->func())(name, NULL))) != NULL ) { \
			num_var = 0; \
			break; \
		} \
		if (num_var >= 999) \
			break; \
	} while ( num_var=ismm->FormatIface(name, sizeof(name)-1) ); \
	if (!assn_var) { \
		if (error) \
			snprintf(error, maxlen, "Could not find interface %s", name); \
		return false; \
	}


const char *GetMapEntitiesString_handler();
bool LevelInit_handler(char const *pMapName, char const *pMapEntities, char const *c, char const *d, bool e, bool f);
void UTIL_PathFmt(char *buffer, size_t len, const char *fmt, ...);

#endif //_INCLUDE_SAMPLEPLUGIN_H
