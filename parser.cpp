/** ======== stripper_mm ========
 *  Copyright (C) 2005-2006 David "BAILOPAN" Anderson
 *  No warranties of any kind.
 *  Based on the original concept of Stripper2 by botman
 *
 *  License: see LICENSE.TXT
 *  ============================
 */

#include <pcre.h>
#include <sh_stack.h>
#include <ctype.h>
#include "parser.h"
#include "stripper_mm.h"

using namespace SourceHook;

Stripper g_Stripper;

void f_strncpy_s(char *dest, const char *str, size_t n)
{
	while (
		n--
		&&
		(*dest++=*str++)
		);
	*dest = '\0';
}

/* UTIL_TrimLeft
 * Removes whitespace characters from left side of string
 */
void UTIL_TrimLeft(char *buffer)
{
	// Let's think of this as our iterator
	char *i = buffer;

	// Make sure the buffer isn't null
	if (i && *i)
	{
		// Add up number of whitespace characters
		while(isspace(*i))
			i++;

		// If whitespace chars in buffer then adjust string so first non-whitespace char is at start of buffer
		// :TODO: change this to not use memcpy()!
		if (i != buffer)
			memcpy(buffer, i, (strlen(i) + 1) * sizeof(char));
	}
}

/* UTIL_TrimLeft
 * Removes whitespace characters from right side of string
 */
void UTIL_TrimRight(char *buffer)
{
	// Make sure buffer isn't null
	if (buffer)
	{
		// Loop through buffer backwards while replacing whitespace chars with null chars
		for (int i = (int)strlen(buffer) - 1; i >= 0; i--)
		{
			if (isspace(buffer[i]))
				buffer[i] = '\0';
			else
				break;
		}
	}
}

Stripper::Stripper()
{
	m_resync = false;

	const char *pattern = "\"([^\"]+)\"\\s+\"([^\"]+)\"";
	const char *error;
	int err_no;
	brk_re = pcre_compile(pattern, 0, &error, &err_no, NULL);
	brk_re_extra = pcre_study(brk_re, 0, &error);
}

Stripper::~Stripper()
{
	Clear();
}

void Stripper::SetEntityList(const char *ents)
{
	Clear();
	
	m_resync = false;
	m_tostring.assign(ents);

	size_t len = m_tostring.size();
	char *_tmp = NULL;
	size_t _tmpsize = 0;
	size_t pos = 0;
	for (size_t i=0; i<len; i++)
	{
		if (ents[i] == '\n' || ents[i] == '\r')
		{
			if (i-pos+1 >= _tmpsize)
			{
				if (_tmp)
					delete [] _tmp;
				_tmpsize = i-pos+1;
				_tmp = new char[_tmpsize+1];
			}
			f_strncpy_s(_tmp, &(ents[pos]), i-pos+1);
			String *pstr = new String(_tmp);
			m_lines.push_back(pstr);
			pos = i+1;
		}
	}
	if (pos < len)
	{
		if (len-pos+1 >= _tmpsize)
		{
			if (_tmp)
				delete [] _tmp;
			_tmpsize =  len-pos+1;
			_tmp = new char[_tmpsize+1];
		}
		f_strncpy_s(_tmp, &(ents[pos]), len-pos+1);
		String *pstr = new String(_tmp);
		m_lines.push_back(pstr);
	}
	if (_tmp)
		delete [] _tmp;
	_BuildPropList();
}

void Stripper::RunRemoveFilter(SourceHook::List<parse_pair *> &filters)
{
	List<List<ent_prop *> *>::iterator iter, end;
	List<ent_prop *>::iterator ei, ee, et;
	List<parse_pair *>::iterator pi, pe;

	int ovector[30];
	iter = m_props.begin();
	end = m_props.end();
	bool remove = false;
	size_t num_match = 0;
	pe = filters.end();
	int rc;
	while (iter != end)
	{
		num_match = 0;
		ei = (*iter)->begin();
		ee = (*iter)->end();
		while (ei != ee)
		{
			pi = filters.begin();
			while (pi != pe)
			{
				//this key matches.
				//does the val match?
				if (stricmp((*ei)->key.c_str(), (*pi)->key.c_str())==0)
				{
					if ( (*pi)->re )
					{
						rc = pcre_exec((*pi)->re, NULL, (*ei)->val.c_str(), (*ei)->val.size(), 0, 0, ovector, 30);
						if (rc >= 0)
							num_match++;
					} else {
						if (stricmp((*ei)->val.c_str(), (*pi)->val.c_str())==0)
							num_match++;
					}
					break;
				}
				pi++;
			}
			if (num_match == filters.size())
				break;
			ei++;
		}
		if (num_match == filters.size())
		{
			//remove this one...
			et = (*iter)->begin();
			while (et != ee)
			{
				delete (*et);
				et++;
			}
			(*iter)->clear();
			iter = m_props.erase(iter);
		} else {
			iter++;
		}
	}
}

void Stripper::RunAddFilter(SourceHook::List<parse_pair *> &list)
{
	List<parse_pair *>::iterator iter, end=list.end();
	iter=list.begin();

	List<ent_prop *> *cl = new List<ent_prop *>();
	ent_prop *e;
	while (iter != end)
	{
		e = new ent_prop;
		e->key.assign( (*iter)->key.c_str() );
		e->val.assign( (*iter)->val.c_str() );
		cl->push_back(e);
		iter++;
	}
	m_props.push_back(cl);
}

void Stripper::_BuildPropList()
{
	int ovector[30];
	char *_key = NULL;
	char *_val = NULL;
	size_t _keysize = 0;
	size_t _valsize = 0;

	bool in_block = false;
	List<String *>::iterator iter, end=m_lines.end();
	String *s;
	List<ent_prop *> *cl = NULL;
	for (iter=m_lines.begin(); iter!=end; iter++)
	{
		s = (*iter);
		if (*(s->c_str()) == '{' && !in_block)
		{
			in_block = true;
			if (!cl)
				cl = new List<ent_prop *>();
		} else if (*(s->c_str()) == '}' && in_block) {
			in_block = false;
			m_props.push_back(cl);
			cl = NULL;
		} else {
			int rc = pcre_exec(brk_re, brk_re_extra, s->c_str(), s->size(), 0, 0, ovector, 30);
			if (rc >= 3)
			{
				size_t l = ovector[3] - ovector[2];
				if (l > _keysize)
				{
					if (_key)
						delete [] _key;
					_keysize = l;
					_key = new char[l+1];
				}
				f_strncpy_s(_key, s->c_str() + ovector[2], l);
				l = ovector[5] - ovector[4];
				if (l > _valsize)
				{
					if (_val)
						delete [] _val;
					_valsize = l;
					_val = new char[l+1];
				}
				f_strncpy_s(_val, s->c_str() + ovector[4], l);
				if (cl)
				{
					ent_prop *e = new ent_prop;
					e->key.assign(_key);
					e->val.assign(_val);
					cl->push_back(e);
				}
			}
		}
	}

	if (_val)
		delete [] _val;
	if (_key)
		delete [] _key;
	if (cl)
		delete cl;
}

void Stripper::ApplyFileFilter(const char *file)
{
	FILE *fp = fopen(file, "rt");
	if (!fp)
		return;

	char buffer[255];
	int ovector[30];
	char *_key = NULL;
	char *_val = NULL;
	size_t _keysize = 0;
	size_t _valsize = 0;
	size_t len;

	enum Mode
	{
		Mode_Filter,
		Mode_Add,
	};

	bool in_block = false;
	List<parse_pair *> props;
	CStack<parse_pair *> fpairs;
	Mode mode = Mode_Filter;
	const char *error = NULL;
	int err_offs = -1;
	int line = 0;
	List<parse_pair *>::iterator iter, end;
	while (!feof(fp))
	{
		line++;
		buffer[0] = '\0';
		fgets(buffer, sizeof(buffer)-1, fp);
		//strip whitespace
		UTIL_TrimRight(buffer);
		UTIL_TrimLeft(buffer);
		//strip comments
		if (buffer[0] == ';' ||
			(buffer[0] == '/' && buffer[1] == '/') ||
			(buffer[0] == '#') ||
			(buffer[0] == '\0'))
			continue;
		if (strncmp(buffer, "filter:", 7) == 0)
		{
			mode = Mode_Filter;
		} else if (strncmp(buffer, "add:", 4) == 0) {
			mode = Mode_Add;
		} else if (strcmp(buffer, "{")==0 && !in_block) {
			in_block = true;
			props.clear();
		} else if (strncmp(buffer, "}", 1)==0 && in_block) {
			in_block = false;
			if (mode == Mode_Filter)
				RunRemoveFilter(props);
			else if (mode == Mode_Add)
				RunAddFilter(props);
			end = props.end();
			for (iter=props.begin(); iter!=end; iter++)
				fpairs.push( (*iter) );
			props.clear();
		} else if (in_block) {
			len = strlen(buffer);
			int rc = pcre_exec(brk_re, brk_re_extra, buffer, len, 0, 0, ovector, 30);
			if (rc >= 3)
			{
				size_t len = ovector[3] - ovector[2];
				if (len > _keysize)
				{
					if (_key)
						delete [] _key;
					_keysize = len;
					_key = new char[len+1];
				}
				f_strncpy_s(_key, buffer + ovector[2], len);
				len = ovector[5] - ovector[4];
				if (len > _valsize)
				{
					if (_val)
						delete [] _val;
					_valsize = len;
					_val = new char[len+1];
				}
				f_strncpy_s(_val, buffer + ovector[4], len);
				pcre *re = NULL;
				if (_val[0] == '/' && _val[len-1] == '/')
				{
					_val[--len] = '\0';
					re = pcre_compile(&(_val[1]), PCRE_CASELESS, &error, &err_offs, NULL);
					if (!re)
					{
						META_LOG(g_PLAPI, "File %s parse error (line %d):", file, line);
						META_LOG(g_PLAPI, "Expression(%s): At pos %d, %s", _val, err_offs, error);
						continue;
					}
				}
				parse_pair *p;
				if (fpairs.empty())
				{
					p = new parse_pair;
				} else {
					p = fpairs.front();
					if (p->re)
						pcre_free(p->re);
					fpairs.pop();
				}
				p->key.assign(_key);
				p->val.assign(_val);
				p->re = re;
				props.push_back(p);
			}
		}
	}

	end=props.end();
	for (iter=props.begin(); iter!=props.end(); iter++)
		fpairs.push( (*iter) );
	parse_pair *k = NULL;
	while (!fpairs.empty())
	{
		k = fpairs.front();
		if (k->re)
			pcre_free(k->re);
		delete k;
		fpairs.pop();
	}

	fclose(fp);

	m_resync = true;
}

const char *Stripper::ToString()
{
	if (!m_resync)
		return m_tostring.c_str();

	m_tostring.clear();

	List<List<ent_prop *> *>::iterator iter, end=m_props.end(), begin=m_props.begin();
	List<ent_prop *>::iterator eiter, eend;
	bool first = true;

	for (iter=m_props.begin(); iter!=end; iter++)
	{
		eend = (*iter)->end();
		if (!first)
			m_tostring.append("\n");
		else
			first = true;
		m_tostring.append("{\n");
		for (eiter=(*iter)->begin(); eiter!=eend; eiter++)
		{
			m_tostring.append('"');
			m_tostring.append( (*eiter)->key.c_str() );
			m_tostring.append("\" \"");
			m_tostring.append( (*eiter)->val.c_str() );
			m_tostring.append("\"\n");
		}
		m_tostring.append("}");
	}

	m_resync = false;

	return m_tostring.c_str();
}

void Stripper::Clear()
{
	List<String *>::iterator iter, end;

	iter = m_lines.begin();
	end = m_lines.end();
	while (iter != end)
	{
		delete (*iter);
		iter++;
	}

	List<ent_prop *>::iterator iter2, end2;
	List<List<ent_prop *> *>::iterator iter3, end3;

	iter3 = m_props.begin();
	end3 = m_props.end();
	while (iter3 != end3)
	{
		iter2 = (*iter3)->begin();
		end2 = (*iter3)->end();
		while (iter2 != end2)
		{
			delete (*iter2);
			iter2++;
		}
		delete (*iter3);
		iter3++;
	}

	m_lines.clear();
	m_props.clear();
	m_tostring.clear();
}
