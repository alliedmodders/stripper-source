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

#include <string>
#include <list>
#include <stack>
#include "pcre2.h"

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
    parse_pair() : re(NULL), match_data(NULL) { }

    std::string key;
    std::string val;
    pcre2_code *re;
    pcre2_match_data *match_data;
};

struct ent_prop : public CACHEABLE
{
    std::string key;
    std::string val;
};

struct replace_prop
{
    std::list<parse_pair *> match;
    std::list<parse_pair *> to_replace;
    std::list<parse_pair *> to_remove;
    std::list<parse_pair *> to_insert;
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
    void RunRemoveFilter(std::list<parse_pair *> &filters);
    void RunAddFilter(std::list<parse_pair *> &list);
    void RunReplaceFilter(replace_prop &replace, std::list<parse_pair *> &props);
    void Clear();
    void _BuildPropList();
private:
    bool JITCompile();
    std::string *AllocString();
    void FreeString(std::string *str);
    ent_prop *AllocProp();
    void FreeProp(ent_prop *prop);
private:
    std::stack<std::string *> m_StringCache;
    std::stack<ent_prop *> m_PropCache;
    void AppendToString(const char* buf, size_t len);
private:
    char* m_tostring;
    size_t m_tostring_len;
    size_t m_tostring_maxlen;
    std::list<std::list<ent_prop *> *> m_props;
    std::list<std::string *> m_lines;
    bool m_resync;
    bool jit_compiled;
    pcre2_code *brk_re;
    pcre2_match_data *brk_match_data;
};

#endif /* _INCLUDE_STRIP_PARSER_H */
