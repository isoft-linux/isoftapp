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

#ifndef __CONF_HELPER_H__
#define __CONF_HELPER_H__

#include <string>
#include <vector>

namespace isoftapp 
{

typedef enum { 
    RPM, 
    SRPM
} source_type_t;

typedef struct {
    source_type_t type;
    std::string url;
    std::string group;
} source_t;

typedef std::vector<source_t> sources_t;

class ConfHelper 
{
public:
    explicit ConfHelper();
    ~ConfHelper();

    sources_t sources() const { return m_sources; }

private:
    void parseConfFile(const char *path);
    void traverseDirectory(const char *path);

    sources_t m_sources;
};

};

#endif /* __CONF_HELPER_H__ */
