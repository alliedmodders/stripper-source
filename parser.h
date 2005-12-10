#ifndef _INCLUDE_STRIP_PARSER_H
#define _INCLUDE_STRIP_PARSER_H

#include <sh_string.h>
#include <sh_list.h>
#include <pcre.h>

struct parse_pair
{
	SourceHook::String key;
	SourceHook::String val;
	pcre *re;
};

struct ent_prop
{
	SourceHook::String key;
	SourceHook::String val;
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
	void Clear();
	void _BuildPropList();
private:
	SourceHook::String m_tostring;
	SourceHook::List<SourceHook::List<ent_prop *> *> m_props;
	SourceHook::List<SourceHook::String *> m_lines;
	bool m_resync;
	pcre *brk_re;
	pcre_extra *brk_re_extra;
};

extern Stripper g_Stripper;

#endif //_INCLUDE_STRIP_PARSER_H
