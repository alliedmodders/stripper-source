/* ======== stub_mm ========
* Copyright (C) 2004-2005 Metamod:Source Development Team
* No warranties of any kind
*
* License: zlib/libpng
*
* Author(s): David "BAILOPAN" Anderson
* ============================
*/

#ifndef _INCLUDE_SAMPLEPLUGIN_H
#define _INCLUDE_SAMPLEPLUGIN_H

#include <ISmmPlugin.h>
#include <sh_string.h>

class StripperPlugin : public ISmmPlugin
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
private:
	const char *ParseAndFilter(const char *map, const char *ents);
};

extern IVEngineServer *engine;
extern IServerGameDLL *server;
extern SourceHook::CallClass<IVEngineServer> *enginepatch;
extern SourceHook::CallClass<IServerGameDLL> *serverpatch;
extern SourceHook::String g_mapname;

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

#define ENGCALL(func)	SH_CALL(enginepatch, &IVEngineServer::func)
#define DLLCALL(func)	SH_CALL(serverpatch, &IServerGameDLL::func)

const char *GetMapEntitiesString_handler();
bool LevelInit_handler(char const *pMapName, char const *pMapEntities, char const *c, char const *d, bool e, bool f);
void UTIL_PathFmt(char *buffer, size_t len, const char *fmt, ...);

#endif //_INCLUDE_SAMPLEPLUGIN_H
