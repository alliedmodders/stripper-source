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

#include <pcre.h>
#include <sh_stack.h>
#include <ctype.h>
#include "parser.h"
#include <sh_string.h>
#include "support.h"

using namespace SourceHook;

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
        {
            i++;
        }

        // If whitespace chars in buffer then adjust string so first non-whitespace char is at start of buffer
        if (i != buffer)
        {
            memmove(buffer, i, (strlen(i) + 1) * sizeof(char));
        }
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
            {
                buffer[i] = '\0';
            } else {
                break;
            }
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

    while (!m_StringCache.empty())
    {
        delete m_StringCache.front();
        m_StringCache.pop();
    }
    while (!m_PropCache.empty())
    {
        delete m_PropCache.front();
        m_PropCache.pop();
    }
}

SourceHook::String *Stripper::AllocString()
{
    if (m_StringCache.empty())
    {
        return new SourceHook::String();
    } else {
        SourceHook::String *str = m_StringCache.front();
        m_StringCache.pop();
        return str;
    }
}

void Stripper::FreeString(SourceHook::String *str)
{
    m_StringCache.push(str);
}

ent_prop *Stripper::AllocProp()
{
    if (m_PropCache.empty())
    {
        return new ent_prop;
    } else {
        ent_prop *p = m_PropCache.front();
#if defined _DEBUG
        assert(p->marked == false);
        p->marked = true;
#endif
        m_PropCache.pop();
        return p;
    }
}

void Stripper::FreeProp(ent_prop *prop)
{
#if defined _DEBUG
    assert(prop->marked == true);
    prop->marked = false;
#endif
    m_PropCache.push(prop);
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
            /* check if we should expand the buffer */
            if (i-pos+1 >= _tmpsize)
            {
                delete [] _tmp;
                _tmpsize = i-pos+1;
                _tmp = new char[_tmpsize+1];
            }
            /* copy this line */
            f_strncpy_s(_tmp, &(ents[pos]), i-pos+1);
            /* make a new string out of it */
            String *pstr = AllocString();
            pstr->assign(_tmp);
            /* push it into the lines table */
            m_lines.push_back(pstr);
            /* iterate */
            pos = i+1;
        }
    }
    /* make sure there is no data we missed */
    if (pos < len)
    {
        /* expand buffer */
        if (len-pos+1 >= _tmpsize)
        {
            delete [] _tmp;
            _tmpsize =  len-pos+1;
            _tmp = new char[_tmpsize+1];
        }
        /* copy */
        f_strncpy_s(_tmp, &(ents[pos]), len-pos+1);
        /* new string */
        String *pstr = AllocString();
        pstr->assign(_tmp);
        /* push it into the lines table */
        m_lines.push_back(pstr);
    }
    /* delete temporary string */
    delete [] _tmp;
    
    /* build the real props list */
    _BuildPropList();
}

bool EntPropsMatch(parse_pair *p, ent_prop *e, int *ovector)
{
    int rc;
    if (stricmp(p->key.c_str(), e->key.c_str()) == 0)
    {
        if (p->re)
        {
            rc = pcre_exec(p->re, NULL, e->val.c_str(), e->val.size(), 0, 0, ovector, 30);
            if (rc >= 0)
            {
                return true;
            }
        } else {
            if (stricmp(e->val.c_str(), p->val.c_str()) == 0)
            {
                return true;
            }
        }
    }

    return false;
}

void ListRecycle(SourceHook::List<parse_pair *> &toread, SourceHook::List<parse_pair *> &towrite)
{
    SourceHook::List<parse_pair *>::iterator iter;
    for (iter=toread.begin(); iter!=toread.end(); iter++)
    {
        towrite.push_back((*iter));
    }
    toread.clear();
}

void Stripper::RunReplaceFilter(replace_prop &replace, SourceHook::List<parse_pair *> &props)
{
    List<List<ent_prop *> *>::iterator ent_iter;
    List<ent_prop *>::iterator prop_iter;
    List<parse_pair *>::iterator parse_iter;
    List<ent_prop *> *proplist;
    ent_prop *e;
    parse_pair *p;
    size_t num_match = 0;
    int ovector[30];

    for (ent_iter = m_props.begin();
         ent_iter != m_props.end();
         ent_iter++)
    {
        num_match = 0;
        proplist = (*ent_iter);
        /* loop through the list of properties for this block */
        for (prop_iter = proplist->begin();
             prop_iter != proplist->end();
             prop_iter++)
        {
            e = (*prop_iter);
            /* try to match the current property against any we're looking for */
            for (parse_iter = replace.match.begin();
                 parse_iter != replace.match.end();
                 parse_iter++)
            {
                p = (*parse_iter);
                if (EntPropsMatch(p, e, ovector))
                {
                    num_match++;
                    break;
                }
            }
            /* if we've reached the end of possible matches, we can stop iterating */
            if (num_match == replace.match.size())
            {
                break;
            }
        }
        /* check if this was a full match */
        if (num_match == replace.match.size())
        {
            /* first do deletions */
            prop_iter = proplist->begin();
            bool matched;
            while (prop_iter != proplist->end())
            {
                e = (*prop_iter);
                matched = false;
                for (parse_iter = replace.to_remove.begin();
                     parse_iter != replace.to_remove.end();
                     parse_iter++)
                {
                    p = (*parse_iter);
                    if (EntPropsMatch(p, e, ovector))
                    {
                        matched = true;
                        break;
                    }
                }
                if (matched)
                {
                    FreeProp(e);
#if defined _DEBUG
                    e->last_alloc = __LINE__;
#endif
                    prop_iter = proplist->erase(prop_iter);
                } else {
                    prop_iter++;
                }
            }
            /* do replacements */
            for (prop_iter = proplist->begin();
                 prop_iter != proplist->end();
                 prop_iter++)
            {
                e = (*prop_iter);
                for (parse_iter = replace.to_replace.begin();
                     parse_iter != replace.to_replace.end();
                     parse_iter++)
                {
                    p = (*parse_iter);
                    /* check if the keys match */
                    if (stricmp(p->key.c_str(), e->key.c_str()) == 0)
                    {
                        e->val.assign(p->val.c_str());
                        break;
                    }
                }
            }
            /* do insertions */
            for (parse_iter = replace.to_insert.begin();
                 parse_iter != replace.to_insert.end();
                 parse_iter++)
            {
                p = (*parse_iter);

                e = AllocProp();
#if defined _DEBUG
                e->last_alloc = __LINE__;
#endif
                e->key.assign(p->key.c_str());
                e->val.assign(p->val.c_str());

                proplist->push_back(e);
            }
        }
    }

    ListRecycle(replace.match, props);
    ListRecycle(replace.to_insert, props);
    ListRecycle(replace.to_remove, props);
    ListRecycle(replace.to_replace, props);
}

void Stripper::RunRemoveFilter(SourceHook::List<parse_pair *> &filters)
{
    List<List<ent_prop *> *>::iterator proplist_iter;
    List<ent_prop *> *proplist;
    List<ent_prop *>::iterator ent_iter;
    List<parse_pair *>::iterator pair_iter, pair_iter_begin, pair_iter_end;
    parse_pair *p;
    ent_prop *e;

    int ovector[30];
    size_t num_match = 0;

    pair_iter_begin = filters.begin();
    pair_iter_end = filters.end();
    proplist_iter = m_props.begin();
    while (proplist_iter != m_props.end())
    {
        num_match = 0;
        proplist = (*proplist_iter);
        for (ent_iter = proplist->begin();
             ent_iter != proplist->end();
             ent_iter++)
        {
            e = (*ent_iter);
            for (pair_iter = pair_iter_begin;
                 pair_iter != pair_iter_end;
                 pair_iter++)
            {
                p = (*pair_iter);
                if (EntPropsMatch(p, e, ovector))
                {
                    num_match++;
                    break;
                }
            }
            if (num_match == filters.size())
            {
                break;
            }
        }
        if (num_match == filters.size())
        {
            //remove this one...
            ent_iter = proplist->begin();
            while (ent_iter != proplist->end())
            {
                e = (*ent_iter);
                FreeProp(e);
                ent_iter = proplist->erase(ent_iter);
#if defined _DEBUG
                e->last_free = __LINE__;
#endif
            }
            proplist->clear();
            delete proplist;
            proplist_iter = m_props.erase(proplist_iter);
        } else {
            proplist_iter++;
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
        e = AllocProp();
#if defined _DEBUG
        e->last_alloc = __LINE__;
#endif
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
            /* if we reached a '{' and we're not in a block, begin a new block */
            in_block = true;
            if (!cl)
            {
                cl = new List<ent_prop *>();
            }
        } else if (*(s->c_str()) == '}' && in_block) {
            /* if we reached a '}' ane we're in a block, end the old block
             * and push it onto the block list
             */
            in_block = false;
            m_props.push_back(cl);
            cl = NULL;
        } else {
            /* try to match our precompiled expression for "..." "..." */
            int rc = pcre_exec(brk_re, brk_re_extra, s->c_str(), s->size(), 0, 0, ovector, 30);
            if (rc >= 3)
            {
                size_t l = ovector[3] - ovector[2];
                if (l > _keysize)
                {
                    delete [] _key;
                    _keysize = l;
                    _key = new char[l+1];
                }
                /* copy the key */
                f_strncpy_s(_key, s->c_str() + ovector[2], l);
                l = ovector[5] - ovector[4];
                if (l > _valsize)
                {
                    delete [] _val;
                    _valsize = l;
                    _val = new char[l+1];
                }
                /* copy the value */
                f_strncpy_s(_val, s->c_str() + ovector[4], l);
                if (cl)
                {
                    /* add a new ent to the list */
                    ent_prop *e = AllocProp();
#if defined _DEBUG
                    e->last_alloc = __LINE__;
#endif
                    e->key.assign(_key);
                    e->val.assign(_val);
                    cl->push_back(e);
                }
            }
        }
    }

    delete [] _val;
    delete [] _key;
    delete cl;
}

void Stripper::ApplyFileFilter(const char *file)
{
    FILE *fp = fopen(file, "rt");
    if (!fp)
    {
        return;
    }

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
        Mode_Replace,
    };

    enum SubMode
    {
        SubMode_None,
        SubMode_Match,
        SubMode_Replace,
        SubMode_Remove,
        SubMode_Insert
    };

    bool in_block = false;
    List<parse_pair *> props;
    CStack<parse_pair *> fpairs;
    Mode mode = Mode_Filter;
    SubMode submode = SubMode_None;
    const char *error = NULL;
    int err_offs = -1;
    int line = 0;
    List<parse_pair *>::iterator iter, end;
    replace_prop replace;
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
        } else if (strncmp(buffer, "remove:", 7) == 0) {
            mode = Mode_Filter;
        } else if (strncmp(buffer, "add:", 4) == 0) {
            mode = Mode_Add;
        } else if (strncmp(buffer, "modify:", 8) == 0) {
            mode = Mode_Replace;
            submode = SubMode_None;
        } else if (!strncmp(buffer, "match:", 6) && (mode == Mode_Replace)) {
            submode = SubMode_Match;
        } else if (!strncmp(buffer, "replace:", 8) && (mode == Mode_Replace)) {
            submode = SubMode_Replace;
        } else if (!strncmp(buffer, "delete:", 7) && (mode == Mode_Replace)) {
            submode = SubMode_Remove;
        } else if (!strncmp(buffer, "insert:", 7) && (mode == Mode_Replace)) {
            submode = SubMode_Insert;
        } else if (strcmp(buffer, "{")==0 && !in_block) {
            /* if we reach a new block and we're not in a block, 
             * dump the current props table and reset.
             */
            if (mode != Mode_Replace || (mode == Mode_Replace && submode != SubMode_None))
            {
                in_block = true;
                props.clear();
            }
        } else if (strncmp(buffer, "}", 1) == 0 && in_block) {
            /* if we reach the end of a block and we're in a block, 
             * mark the end and flush what we've done
             */
            in_block = false;
            
            if (mode == Mode_Replace)
            {
                assert(submode != SubMode_None);
                switch (submode)
                {
                case SubMode_None:
                    {
                        break;
                    }
                case SubMode_Insert:
                    {
                        replace.to_insert = props;
                        props.clear();
                        break;
                    }
                case SubMode_Remove:
                    {
                        replace.to_remove = props;
                        props.clear();
                        break;
                    }
                case SubMode_Match:
                    {
                        replace.match = props;
                        props.clear();
                        break;
                    }
                case SubMode_Replace:
                    {
                        replace.to_replace = props;
                        props.clear();
                        break;
                    }
                }
            } else {
                /* run the appropriate routine */
                if (mode == Mode_Filter)
                {
                    RunRemoveFilter(props);
                } else if (mode == Mode_Add) {
                    RunAddFilter(props);
                }
    
                /* push each of the unused filters 
                * and then clear the property list
                */
                end = props.end();
                for (iter=props.begin(); iter!=end; iter++)
                {
                    fpairs.push((*iter));
                }
                props.clear();
            }
        } else if (strncmp(buffer, "}", 1) == 0 && !in_block && mode == Mode_Replace) {
            /* run replace filter */
            RunReplaceFilter(replace, props);
            end = props.end();
            for (iter=props.begin(); iter!=end; iter++)
            {
                fpairs.push((*iter));
            }
            props.clear();
        } else if (in_block) {
            /* attempt to run our precompiled property match expression */
            len = strlen(buffer);
            int rc = pcre_exec(brk_re, brk_re_extra, buffer, len, 0, 0, ovector, 30);
            if (rc >= 3)
            {
                size_t len = ovector[3] - ovector[2];
                if (len > _keysize)
                {
                    delete [] _key;
                    _keysize = len;
                    _key = new char[len+1];
                }
                /* copy the key */
                f_strncpy_s(_key, buffer + ovector[2], len);
                len = ovector[5] - ovector[4];
                if (len > _valsize)
                {
                    delete [] _val;
                    _valsize = len;
                    _val = new char[len+1];
                }
                /* copy the value */
                f_strncpy_s(_val, buffer + ovector[4], len);
                pcre *re = NULL;
                if (_val[0] == '/' && _val[len-1] == '/')
                {
                    _val[--len] = '\0';
                    re = pcre_compile(&(_val[1]), PCRE_CASELESS, &error, &err_offs, NULL);
                    if (!re)
                    {
                        stripper_game.log_message("File %s parse error (line %d):", file, line);
                        stripper_game.log_message("Expression(%s): At pos %d, %s", _val, err_offs, error);
                        continue;
                    }
                }
                /* get a parse pair from the cache */
                parse_pair *p;
                if (fpairs.empty())
                {
                    p = new parse_pair;
                } else {
                    p = fpairs.front();
                    if (p->re)
                    {
                        pcre_free(p->re);
                    }
                    fpairs.pop();
                }
                /* assign property information */
                p->key.assign(_key);
                p->val.assign(_val);
                p->re = re;
                props.push_back(p);
            }
        }
    }

    /* flush anything remaining */
    end=props.end();
    for (iter=props.begin(); iter!=props.end(); iter++)
    {
        fpairs.push( (*iter) );
    }

    /* delete cached pairs */
    parse_pair *k = NULL;
    while (!fpairs.empty())
    {
        k = fpairs.front();
        if (k->re)
        {
            pcre_free(k->re);
        }
        delete k;
        fpairs.pop();
    }

    fclose(fp);

    m_resync = true;
}

const char *Stripper::ToString()
{
    if (!m_resync)
    {
        return m_tostring.c_str();
    }

    m_tostring.clear();

    List<List<ent_prop *> *>::iterator iter, end=m_props.end(), begin=m_props.begin();
    List<ent_prop *>::iterator eiter, eend;
    bool first = true;

    for (iter=m_props.begin(); iter!=end; iter++)
    {
        eend = (*iter)->end();
        if (!first)
        {
            m_tostring.append("\n");
        } else {
            first = true;
        }
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
    List<String *>::iterator lines_iter;

    lines_iter = m_lines.begin();
    while (lines_iter != m_lines.end())
    {
        FreeString((*lines_iter));
        lines_iter = m_lines.erase(lines_iter);
    }
    m_lines.clear();

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
            FreeProp((*iter2));
#if defined _DEBUG
            (*iter2)->last_free = __LINE__;
#endif
            iter2++;
        }
        delete (*iter3);
        iter3++;
    }

    m_props.clear();
    m_tostring.clear();
}
