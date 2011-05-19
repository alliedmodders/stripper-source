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
#ifndef _INCLUDE_STRIPPER_INTERCOM_H_
#define _INCLUDE_STRIPPER_INTERCOM_H_

struct stripper_game_t
{
    const char* game_path;
    const char* stripper_path;
    const char* stripper_cfg_path;
    void (*log_message)(const char *fmt, ...);
    void (*path_format)(char* buffer, size_t maxlength, const char* fmt, ...);
    const char* (*get_map_name)();
};

struct stripper_core_t
{
    const char* (*parse_map)(const char* map, const char* entities);
    const char* (*ent_string)();
    void (*command_dump)();
    void (*unload)();
};

typedef void (*STRIPPER_LOAD)(const stripper_game_t* game, stripper_core_t* core);

#endif /* _INCLUDE_STRIPPER_INTERCOM_H_ */

