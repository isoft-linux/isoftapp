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

#include "conf-helper.h"
#include "splitstring.h"

#include <iostream>
#include <glib.h>

namespace isoftapp 
{

ConfHelper::ConfHelper() 
{
    std::string default_conf_path = ISOFTAPP_CONF_DIR + std::string("/default.conf");
    std::string conf_dir = ISOFTAPP_CONF_DIR + std::string("/config.d");

    if (g_file_test(default_conf_path.c_str(), 
                    G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
        parseConfFile(default_conf_path.c_str());
    }

    if (g_file_test(conf_dir.c_str(), G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))
        traverseDirectory(conf_dir.c_str());
}

ConfHelper::~ConfHelper() 
{
}

void ConfHelper::parseConfFile(const char *path) 
{
    GKeyFile *conf = NULL;
    gchar *host = NULL;
    gchar *version = NULL;
    gchar *arch = NULL;
    std::string url = "";
    gchar *repos = NULL;
    splitstring *s = nullptr;
    std::vector<std::string> flds;
    gboolean enable = TRUE;

    conf = g_key_file_new();
    if (conf == NULL)
        goto exit;

    g_key_file_load_from_file(conf, path, G_KEY_FILE_NONE, NULL);
    host = g_key_file_get_string(conf, "repo", "url", NULL);
    if (host == NULL) {
        std::cerr << path << " \033[31murl\033[0m was not found" << std::endl;
        goto exit;
    }
    version = g_key_file_get_string(conf, "repo", "version", NULL);
    arch = g_key_file_get_string(conf, "repo", "arch", NULL);

    url = std::string(host);
    if (!g_str_has_prefix(url.c_str(), "http://") && 
        !g_str_has_prefix(url.c_str(), "file://")) {
        std::cerr << path << " is wrong\n"
                    "url=\033[31m" << url << "\033[0m\n"
                    "you might forget \033[32mhttp://\033[0m or "
                    "\033[32mfile://\033[0m before \033[31m" << url << "\033[0m" 
                    << std::endl;
        goto exit;
    }
    if (version)
        url += "/" + std::string(version);
    if (arch)
        url += "/" + std::string(arch);

    repos = g_key_file_get_string(conf, "repo", "repos", NULL);
    if (repos == NULL) {
        std::cerr << path << " \033[31mrepos\033[0m not found" << std::endl;
        goto exit;
    }

    enable = g_key_file_get_boolean(conf, "repo", "enable", NULL);
    if (!g_key_file_has_key(conf, "repo", "enable", NULL))
        enable = TRUE;

    if (!enable)
        goto exit;

    s = new splitstring(repos);
    flds = s->split(',');
    if (flds.size() == 0) {
        std::cerr << path << " is wrong\n"
                    "repos should be seprated by comma" << std::endl;
        goto exit;
    }
    for (unsigned int i = 0; i < flds.size(); i++) {
        source_t source;
        source.type = RPM;
        source.url = url;
        source.group = flds[i];
        m_sources.push_back(source);
    }

exit:
    if (conf) {
        g_key_file_unref(conf);
        conf = NULL;
    }

    if (s) {
        delete s;
        s = nullptr;
    }
}

void ConfHelper::traverseDirectory(const char *path) 
{
    GDir *dir = NULL;
    GError *error = NULL;
    const gchar *file = NULL;
    gchar *newpath = NULL;

    dir = g_dir_open(path, 0, &error);
    if (!dir) {
        std::cerr << error->message << std::endl;
        g_error_free(error);
        error = NULL;
        return;
    }

    while (file = g_dir_read_name(dir)) {
        newpath = g_strdup_printf("%s/%s", path, file);
        if (g_file_test(newpath, G_FILE_TEST_IS_DIR)) {
            traverseDirectory(newpath);
        } else if (g_file_test(newpath, G_FILE_TEST_IS_REGULAR)) {
            if (g_str_has_suffix(newpath, ".conf"))
                parseConfFile(newpath);
        }

        g_free(newpath);
        newpath = NULL;
    }

    g_dir_close(dir);
    dir = NULL;
}

};
