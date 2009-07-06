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
#ifndef _INCLUDE_SAMPLEPLUGIN_H
#define _INCLUDE_SAMPLEPLUGIN_H

#include <ISmmPlugin.h>
#include <sh_string.h>
#include <sh_list.h>
#include <sh_stack.h>

#define STRIPPER_VERSION        "1.2.1"

class StripperPlugin : 
    public ISmmPlugin, 
    public IConCommandBaseAccessor
{
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
public:    //IConCommandBaseAccessor
    bool RegisterConCommandBase(ConCommandBase *pVar);
};

PLUGIN_GLOBALVARS();

#define    FIND_IFACE(func, assn_var, num_var, name, type) \
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

#endif //_INCLUDE_SAMPLEPLUGIN_H
