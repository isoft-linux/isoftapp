/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __URI_HELPER_H__
#define __URI_HELPER_H__

#include <string>

#include <uriparser/Uri.h>

class urihelper 
{
public:
    urihelper(std::string uri, std::string delim = ".", int level = 18) 
    {  
        UriParserStateA state;
        state.uri = &m_uriParse;
        if (uriParseUriA(&state, uri.c_str()) == URI_SUCCESS) {
            m_scheme = fromRange(m_uriParse.scheme);
            m_host = fromRange(m_uriParse.hostText);
            m_path = fromList(m_uriParse.pathHead, delim, level);
        }
    }
    ~urihelper() { uriFreeUriMembersA(&m_uriParse); }

    std::string scheme() const { return m_scheme; }
    std::string host() const { return m_host; }
    std::string path() const { return m_path; }

private:
    std::string fromRange(const UriTextRangeA & rng) const 
    {
        return std::string(rng.first, rng.afterLast);
    }

    std::string fromList(UriPathSegmentA * xs, const std::string & delim, int level) const
    {
        UriPathSegmentStructA * head(xs);
        std::string accum;

        while (head && level--) {
            accum += delim + fromRange(head->text);
            head = head->next;
        }

        return accum;
    }

    UriUriA m_uriParse;
    std::string m_scheme;
    std::string m_host;
    std::string m_path;
};

#endif // __URI_HELPER_H__
