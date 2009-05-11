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
#ifndef _INCLUDE_STRIP_PARSER_H
#define _INCLUDE_STRIP_PARSER_H

#include <sh_string.h>
#include <sh_list.h>
#include <sh_stack.h>
#include <pcre.h>

struct CACHEABLE
{
#if defined _DEBUG
    CACHEABLE() : marked(true), last_alloc(0), last_free(0) { };
    bool marked;
    int last_alloc;
    int last_free;
#endif
};

struct parse_pair
{
    SourceHook::String key;
    SourceHook::String val;
    pcre *re;
};

struct ent_prop : public CACHEABLE
{
    SourceHook::String key;
    SourceHook::String val;
};

struct replace_prop
{
    SourceHook::List<parse_pair *> match;
    SourceHook::List<parse_pair *> to_replace;
    SourceHook::List<parse_pair *> to_remove;
    SourceHook::List<parse_pair *> to_insert;
};

class Stripper
{
public:
    Stripper();
    ~Stripper();
public:
    void SetEntityList(const char *ents);
    void ApplyFileFilter(const char *file);
    const char *ToString();
private:
    void RunRemoveFilter(SourceHook::List<parse_pair *> &filters);
    void RunAddFilter(SourceHook::List<parse_pair *> &list);
    void RunReplaceFilter(replace_prop &replace, SourceHook::List<parse_pair *> &props);
    void Clear();
    void _BuildPropList();
private:
    SourceHook::String *AllocString();
    void FreeString(SourceHook::String *str);
    ent_prop *AllocProp();
    void FreeProp(ent_prop *prop);
private:
    SourceHook::CStack<SourceHook::String *> m_StringCache;
    SourceHook::CStack<ent_prop *> m_PropCache;
private:
    SourceHook::String m_tostring;
    SourceHook::List<SourceHook::List<ent_prop *> *> m_props;
    SourceHook::List<SourceHook::String *> m_lines;
    bool m_resync;
    pcre *brk_re;
    pcre_extra *brk_re_extra;
};

#endif /* _INCLUDE_STRIP_PARSER_H */

