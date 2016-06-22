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

#ifndef __DOWNLOAD_H__
#define __DOWNLOAD_H__

#include <curl/curl.h>

#include <glib.h>
#include "daemon.h"

/* 下载完成后的回调函数 */
typedef void (*downloaded_handler)(const gchar  *file_path, 
                                   const gchar  *url);
/* 下载出错的回调函数 */
typedef void (*downloaded_handler_error)(const gchar    *file_path, 
                                         const gchar    *url, 
                                         CURLcode   error);
/* 下载进度的回调函数 */
typedef void (*downloaded_handler_progress)(double      *progress,
                                            void        *arg_data,
                                            const char  *pkgName,
                                            Status      status);

typedef struct {
    GMutex lock;

    const char *file_path;
    const char *url;
    
    downloaded_handler handler;
    downloaded_handler_error handler_error;
    downloaded_handler_progress handler_progress;
    void *arg_data;
} DownloadData;

/* 构造函数，将成员全部指向空指针 */
DownloadData *download_data_new();

/* 下载回调函数，线程安全 */
void *download_routine(void *arg);

#endif /* __DOWNLOAD_H__ */
