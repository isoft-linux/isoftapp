/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 * Copyright (C) 2015 fujiang <fujiang.zhu@i-soft.com.cn>
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

#include <stdlib.h>
#include <stdarg.h>
#include <locale.h>
#include <libintl.h>
#include <syslog.h>
#include <sys/stat.h>
#include <errno.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib-unix.h>
#include <gio/gio.h>
#include <stdio.h>
#include <math.h>
#include <NetworkManager.h>


#include "daemon.h"

#define NAME_TO_CLAIM "org.isoftlinux.Isoftapp"
#define VERSION_ID_PREFIX "VERSION_ID="

#define CACHE_UPDATE_DURATION (4*60*60*1000) // 12 hours=>4
#define  ICON_CACHE_DIR "/var/cache/isoftapp/qjade"
struct icon_cache_info {
    char name[128];
    char icon[512];
    char title[128];
    char desc[512];
    char cate[128];
};

enum {
    PROP_0,
    PROP_DAEMON_VERSION,
    PROP_UPDATE_DURATION
};


struct DaemonPrivate {
    GDBusConnection *bus_connection;
    GHashTable *extension_ifaces;
    GDBusMethodInvocation *context;

    gint64 update_duration;

    gboolean call_by_client;
    guint update_timer;
    gboolean task_locked;

    GDBusProxy *logind;
    GDBusProxy *nm;
    GDBusProxy *upgrade_proxy;

    GMutex lock;
    char *arg;
    gboolean nodep;
};


static void daemon_isoftapp_iface_init(GdbusIsoftappIface *iface);

G_DEFINE_TYPE_WITH_CODE(Daemon, daemon, GDBUS_TYPE_ISOFTAPP_SKELETON, G_IMPLEMENT_INTERFACE(GDBUS_TYPE_ISOFTAPP, daemon_isoftapp_iface_init));

#define DAEMON_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), TYPE_DAEMON, DaemonPrivate))

static const GDBusErrorEntry update_error_entries[] =
{
    { ERROR_FAILED, "org.isoftlinux.Isoftapp.Error.Failed" },
    { ERROR_PERMISSION_DENIED, "org.isoftlinux.Isoftapp.Error.PermissionDenied" },
    { ERROR_NOT_SUPPORTED, "org.isoftlinux.Isoftapp.Error.NotSupported" }
};

void checkDB(Daemon *object);
static void
nm_signal(GDBusProxy *proxy,
          gchar      *sender_name,
          gchar      *signal_name,
          GVariant   *parameters,
          gpointer    user_data)
{
    Daemon *daemon = (Daemon *)user_data;

    printf("%s, line %d:name[%s]!\n", __func__, __LINE__,signal_name);

    if (strcmp(signal_name, "StateChanged") != 0)
        return;

    GError *error = NULL;
    GVariant *state;
    NMState nm_state;
    state = g_dbus_proxy_call_sync (daemon->priv->nm, "state",
                                    NULL,
                                    G_DBUS_CALL_FLAGS_NONE, -1,
                                    NULL, &error);
    if (error != NULL) {
       printf("%s, line %d:msg[%s]!\n", __func__, __LINE__, error->message);
       g_error_free (error);
       error = NULL;
       return;
    }

    g_variant_get (state, "(u)", &nm_state);
    g_variant_unref (state);
#if 0
    if(nm_state >= NM_STATE_CONNECTED_LOCAL) {
        checkDB(daemon);
    }
#else
    printf("%s, line %d:nm[%d] vs [%d]\n", __func__, __LINE__, (int) nm_state,(int)NM_STATE_CONNECTED_LOCAL);
    // adjust nm_state
    if(nm_state > NM_STATE_CONNECTED_LOCAL) {
        checkDB(daemon);

        char errStr[512]="";
        snprintf(errStr,"%s","online");
        gdbus_isoftapp_emit_error(GDBUS_ISOFTAPP(daemon),
            ERROR_NETWORK_ONLINE,
            errStr,
            ERROR_CODE_NETWORK);
    } else if (nm_state <= NM_STATE_CONNECTED_LOCAL) { // offline
        char errStr[512]="";
        snprintf(errStr,"%s","offline");
        gdbus_isoftapp_emit_error(GDBUS_ISOFTAPP(daemon),
            ERROR_NETWORK_OFFLINE,
            errStr,
            ERROR_CODE_NETWORK);
    }

#endif
}

static void
daemon_maybe_add_extension_interface(GHashTable         *ifaces,
                                     GDBusInterfaceInfo *iface)
{
    gint i;

    /* We visit the XDG data dirs in precedence order, so if we
     * already have this one, we should not add it again.
     */
    if (g_hash_table_contains(ifaces, iface->name))
        return;

    /* Only accept interfaces with only properties */
    if ((iface->methods && iface->methods[0]) ||
        (iface->signals && iface->signals[0])) {
        return;
    }

    /* Add it only if we can find the annotation */
    for (i = 0; iface->annotations && iface->annotations[i]; i++) {
        if (g_str_equal(iface->annotations[i]->key,
                        "org.isoftlinux.Isoftapp.VendorExtension")) {
            g_hash_table_insert(ifaces, g_strdup(iface->name),
                                g_dbus_interface_info_ref(iface));
            return;
        }
    }
}

static void
daemon_read_extension_file(GHashTable  *ifaces,
                           const gchar *filename)
{
    GError *error = NULL;
    GDBusNodeInfo *node;
    gchar *contents;
    gint i;

    if (!g_file_get_contents(filename, &contents, NULL, &error)) {
        g_warning("Unable to read extension file %s: %s.  Ignoring.",
                  filename, error->message);
        g_error_free(error);
        return;
    }

    node = g_dbus_node_info_new_for_xml(contents, &error);
    if (node) {
        for (i = 0; node->interfaces && node->interfaces[i]; i++)
            daemon_maybe_add_extension_interface(ifaces, node->interfaces[i]);

        g_dbus_node_info_unref(node);
    } else {
        g_warning("Failed to parse file %s: %s", filename, error->message);
        g_error_free(error);
    }

    g_free(contents);
}

static void
daemon_read_extension_directory(GHashTable  *ifaces,
                                const gchar *path)
{
    const gchar *name;
    GDir *dir;

    dir = g_dir_open(path, 0, NULL);
    if (!dir)
        return;

    while ((name = g_dir_read_name(dir))) {
        gchar *filename;
        gchar *symlink;


        filename = g_build_filename(path, name, NULL);
        symlink = g_file_read_link(filename, NULL);

        if (!symlink) {
            g_warning("Found isoft app vendor extension file %s, but file "
                      "must be a symlink to '../../dbus-1/interfaces/%s' for "
                      "forwards-compatibility reasons.", filename, name);
            g_free(filename);
            continue;
        }

        /* Ensure it looks like "../../dbus-1/interfaces/${name}" */
        const gchar * const prefix = "../../dbus-1/interfaces/";
        if (g_str_has_prefix(symlink, prefix) &&
            g_str_equal(symlink + strlen (prefix), name)) {
            daemon_read_extension_file(ifaces, filename);
        } else {
            g_warning("Found isoft app vendor extension symlink %s, but it "
                      "must be exactly equal to '../../dbus-1/interfaces/%s' "
                      "for forwards-compatibility reasons.", filename, name);
        }

        g_free(filename);
        g_free(symlink);
    }

    g_dir_close(dir);
}


GHashTable *
daemon_read_extension_ifaces()
{
    const gchar * const *data_dirs;
    GHashTable *ifaces;
    gint i;

    ifaces = g_hash_table_new_full(g_str_hash,
                                   g_str_equal,
                                   g_free,
                                   (GDestroyNotify)g_dbus_interface_info_unref);

    data_dirs = g_get_system_data_dirs();
    for (i = 0; data_dirs[i]; i++) {
        gchar *path = g_build_filename(data_dirs[i],
                                       "isoftapp/interfaces", NULL);

        daemon_read_extension_directory(ifaces, path);

        g_free(path);
    }

    return ifaces;
}

static void upgrade_signal(GdbusIsoftapp *object, const gchar *arg_pkgName);
static void *update_routine(void *arg);

static gboolean
register_isoftapp_daemon(Daemon *daemon)
{
    GError *error = NULL;

    daemon->priv->bus_connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
    if (daemon->priv->bus_connection == NULL) {
        if (error != NULL) {
            printf("error getting system bus: %s", error->message);
            g_error_free(error);
        }
        goto exitit;
    }

    if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(daemon),
                                          daemon->priv->bus_connection,
                                          "/org/isoftlinux/Isoftapp",
                                          &error)) {
        if (error != NULL) {
            printf("error exporting interface: %s", error->message);
            g_error_free(error);
        }
        goto exitit;
    }

    g_signal_connect(daemon, "upgrade-changed", G_CALLBACK(upgrade_signal), daemon);

    return TRUE;

exitit:
    return FALSE;
}

static const gchar *
daemon_get_daemon_version(GdbusIsoftapp *object)
{
    return PROJECT_VERSION;
}
static gint64
daemon_get_update_duration(GdbusIsoftapp *object)
{
    Daemon *daemon = (Daemon *)object;
    return daemon->priv->update_duration;
}

extern void install(int argc, char *argv[],
                    void (*report_progress)(double *progress, void *arg_data,const char *filename,Status status),
                    void *arg_data,
                    bool reInstall);
extern void remove(int argc, char *argv[],
                   void (*report_progress)(double *progress, void *arg_data,const char *filename,Status status),
                   void *arg_data);

extern void upgrade(int argc, char *argv[],
             void (*report_progress)(double *progress, void *arg_data,const char *filename,Status status),
             void *arg_data);

extern void search(int argc, char *argv[],
             void (*result_handle)(void *arg_data, char *pkgName, gint64 *arg_status),
             void *arg_data,
             bool getAllPkg);
extern void update();

extern void listInstalled(int argc, char *argv[],bool needSignal,
            void (*result_handle)(void *arg_data, char *pkgName),
            void *arg_data);

extern void listUnInstalled(int argc, char *argv[],
            bool needSignal,
            void (*result_handle)(void *arg_data, char *pkgName),
            void *arg_data,
            bool searchAll);

extern char *getDesktopName(char *pkgName,char getDesktopName[512]);
extern void check(int argc, char *argv[],
            void (*result_handle)(void *arg_data, char *pkgName),
            void *arg_data);
extern void listUpdate(int argc, char *argv[],
                       void (*result_handle)(void *arg_data, char *pkgName),
                       void *arg_data);
static void 
rpm_progress_handle(double      *progress, 
                    void        *arg_data, 
                    const char  *pkgName, 
                    Status      status)
{
    Daemon *daemon = (Daemon *)arg_data;

    char errStr[512]="";
    snprintf(errStr,"%s",pkgName==NULL?"null":pkgName);

    if (status == STATUS_INSTALL_ERROR) {
        gdbus_isoftapp_emit_error(GDBUS_ISOFTAPP(daemon),
            ERROR_PKG_NOT_EXIST,
            errStr,
            ERROR_CODE_OTHERS);
        return;
    } else if (status == STATUS_REMOVE_ERROR) {
        gdbus_isoftapp_emit_error(GDBUS_ISOFTAPP(daemon),
            ERROR_PKG_NOT_REMOVED,
            errStr,
            ERROR_CODE_OTHERS);
        return;
    } else if (status == STATUS_UPGRADE_ERROR) {
        gdbus_isoftapp_emit_error(GDBUS_ISOFTAPP(daemon),
            ERROR_PKG_NOT_UPGRADED,
            errStr,
            ERROR_CODE_OTHERS);
        return;
    }

    if ( fabs(*progress - 110.0) < 0.9) {

        gdbus_isoftapp_emit_error(GDBUS_ISOFTAPP(daemon),
            ERROR_INSTALL,
            errStr,
            ERROR_CODE_INSTALL);

        printf("%s, line %d: %f,Failed to install rpm file!\n", __func__, __LINE__,  *progress);
    } else {
        gdbus_isoftapp_emit_percent_changed(GDBUS_ISOFTAPP(daemon),
                                           status,
                                           pkgName,
                                           *progress);
    }
}
bool needUpdate();

void checkDB(Daemon *object)
{
    Daemon *daemon = (Daemon *)object;

    if (needUpdate()) {

        gdbus_isoftapp_emit_error(GDBUS_ISOFTAPP(daemon),
                ERROR_UPDATE,
                g_strdup_printf(_("updating.")),
                ERROR_CODE_UPDATEING);

        update();

        printf("%s, line %d:......!\n", __func__, __LINE__);

        gdbus_isoftapp_emit_finished(GDBUS_ISOFTAPP(daemon),"", STATUS_UPDATED);

    }

}

void emit_search_result_handle(void *arg_data, char *pkgName, gint64 *arg_status )
{
    Daemon *daemon = (Daemon *)arg_data;

    if ( *arg_status != 1 && *arg_status!= 0 ) {
        char errStr[512]="";
        snprintf(errStr,"%s",pkgName==NULL?"null":pkgName);
        gdbus_isoftapp_emit_error(GDBUS_ISOFTAPP(daemon),
            ERROR_SEARCH,
            errStr,
            ERROR_CODE_SEARCH);

        printf("%s, line %d:,No this package[%s]!\n", __func__, __LINE__, pkgName);
    } else {
        gdbus_isoftapp_emit_search_changed(GDBUS_ISOFTAPP(daemon),
                                           pkgName,
                                           *arg_status);
    }
}


void emit_search_all_result_handle(void *arg_data, char *pkgName, gint64 *arg_status)
{
    Daemon *daemon = (Daemon *)arg_data;
    gdbus_isoftapp_emit_list_all_changed(GDBUS_ISOFTAPP(daemon),
                                         pkgName,
                                         arg_status);
}

void emit_list_update_changed_handle(void *arg_data, char *pkgName )
{
    Daemon *daemon = (Daemon *)arg_data;

    gdbus_isoftapp_emit_list_update_finished(GDBUS_ISOFTAPP(daemon),
                                           pkgName);
}
void emit_list_changed_handle(void *arg_data, char *pkgName )
{
    Daemon *daemon = (Daemon *)arg_data;

    gdbus_isoftapp_emit_list_changed(GDBUS_ISOFTAPP(daemon),
                                           pkgName);
}

static  void upgrade_signal(GdbusIsoftapp *object,
                            const gchar *arg_pkgName)
{
    Daemon *daemon = (Daemon *)object;

    printf("\n%s, line %d:recieved[%s].\n", __func__, __LINE__,arg_pkgName);
    printf("\n%s, line %d:recieved[%llu].\n", __func__, __LINE__,daemon->priv->update_duration);

    char *argv[3];
    argv[0]=(char *)"";
    argv[1]=(char *)"";
    argv[2]=(char *)arg_pkgName;
    upgrade(3,argv,rpm_progress_handle, daemon);

}

/*
 * realInstall() isoftapp.cpp
*/
extern int g_installed_num;
static void *
install_routine(void *arg)
{
    Daemon *daemon = (Daemon *)arg;
    if (!g_mutex_trylock(&daemon->priv->lock)) {
        // TODO: emit error

        gdbus_isoftapp_emit_finished(GDBUS_ISOFTAPP(daemon),"", STATUS_INSTALLED);
        return NULL;
    }

    checkDB(daemon);

    char *argv[3];
    argv[0]=(char *)"";
    argv[1]=(char *)"";
    argv[2]=(char *)daemon->priv->arg;
    install(3, argv, rpm_progress_handle, daemon, false);
    gdbus_isoftapp_emit_finished(GDBUS_ISOFTAPP(daemon),(char *)daemon->priv->arg, STATUS_INSTALLED);

#if 1
    for(int j = 7;j < 2000;j++) {
        close(j);
    }

    if(g_installed_num > 50) {
        exit(0);
    }

    printf("\n%s, line %d:g_installed_num[%d].\n", __func__, __LINE__,g_installed_num);

#endif
    g_mutex_unlock(&daemon->priv->lock);

    return NULL;
}

static gboolean
daemon_install(GdbusIsoftapp *object,
               GDBusMethodInvocation *context,
               const gchar *pkgName)
{
    Daemon *daemon = (Daemon *)object;
    GThread *thread = NULL;

    daemon->priv->arg = g_strdup(pkgName);

    thread = g_thread_new(NULL, install_routine, daemon);
    if (thread) {
        g_thread_unref(thread);
        thread = NULL;
    }

    return TRUE;
}

static void *
remove_routine(void *arg) 
{
    Daemon *daemon = (Daemon *)arg;
    if (!g_mutex_trylock(&daemon->priv->lock)) {
        gdbus_isoftapp_emit_finished(GDBUS_ISOFTAPP(daemon),"", STATUS_REMOVED);
        return NULL;
    }

    checkDB(daemon);

    if (daemon->priv->nodep) {
        char *argv[4];
        argv[0] = (char *)"";
        argv[1] = (char *)"";
        argv[2] = "-f";
        argv[3] = daemon->priv->arg;
        remove(4, argv, rpm_progress_handle, daemon);
    } else {
        char *argv[3];
        argv[0] = (char *)"";
        argv[1] = (char *)"";
        argv[2] = daemon->priv->arg;
        remove(3, argv, rpm_progress_handle, daemon);
    }
    gdbus_isoftapp_emit_finished(GDBUS_ISOFTAPP(daemon),daemon->priv->arg, STATUS_REMOVED);
    g_mutex_unlock(&daemon->priv->lock);
    return NULL;
}

static gboolean
daemon_remove(GdbusIsoftapp *object,
               GDBusMethodInvocation *context,
               const gchar *pkgName, 
               gboolean nodep)
{
    Daemon *daemon = (Daemon *)object;
    GThread *thread = NULL;
    
    daemon->priv->arg = g_strdup(pkgName);
    daemon->priv->nodep = nodep;
    thread = g_thread_new(NULL, remove_routine, daemon);
    if (thread) {
        g_thread_unref(thread);
        thread = NULL;
    }
    return TRUE;
}

static void *
upgrade_routine(void *arg) 
{
    Daemon *daemon = (Daemon *)arg;
    if (!g_mutex_trylock(&daemon->priv->lock)) {
        gdbus_isoftapp_emit_finished(GDBUS_ISOFTAPP(daemon),"", STATUS_UPGRADED);
        return NULL;
    }
    checkDB(daemon);

    char *argv[3];
    argv[0] = (char *)"";
    argv[1] = (char *)"";
    argv[2] = daemon->priv->arg;
    upgrade(3, argv, rpm_progress_handle, daemon);
    gdbus_isoftapp_emit_finished(GDBUS_ISOFTAPP(daemon),daemon->priv->arg, STATUS_UPGRADED);
    g_mutex_unlock(&daemon->priv->lock);
    return NULL;
}

static gboolean
daemon_upgrade(GdbusIsoftapp *object,
               GDBusMethodInvocation *context,
               const gchar *pkgName)
{
    Daemon *daemon = (Daemon *)object;
    GThread *thread = NULL;
    
    daemon->priv->arg = g_strdup(pkgName);
    thread = g_thread_new(NULL, upgrade_routine, daemon);
    if (thread) {
        g_thread_unref(thread);
        thread = NULL;
    }

    return TRUE;
}

static void *
search_routine(void *arg) 
{
    Daemon *daemon = (Daemon *)arg;
    if (!g_mutex_trylock(&daemon->priv->lock)) {
        gdbus_isoftapp_emit_finished(GDBUS_ISOFTAPP(daemon),"", STATUS_SEARCHED);
        return NULL;
    }
    // TODO: 如果pkgcache.db没有准备好，其他routine也类似
    checkDB(daemon);
    char *argv[3];
    argv[0] = (char *)"";
    argv[1] = (char *)"";
    argv[2] = daemon->priv->arg;
    search(3, argv, emit_search_result_handle, daemon,false);
    gdbus_isoftapp_emit_finished(GDBUS_ISOFTAPP(daemon),daemon->priv->arg, STATUS_SEARCHED);
    g_mutex_unlock(&daemon->priv->lock);

    return NULL;
}

static gboolean
daemon_search(GdbusIsoftapp *object,
               GDBusMethodInvocation *invocation,
               const gchar *arg_term)
{
    Daemon *daemon = (Daemon *)object;
    GThread *thread = NULL;

    daemon->priv->arg = g_strdup(arg_term);
    thread = g_thread_new(NULL, search_routine, daemon);
    if (thread) {
        g_thread_unref(thread);
        thread = NULL;
    }

    return TRUE;
}

static gboolean
daemon_update(GdbusIsoftapp *object,
               GDBusMethodInvocation *invocation)
{
    Daemon *daemon = (Daemon *)object;
    GThread *thread = NULL;

    thread = g_thread_new(NULL, update_routine, daemon);
    if (thread) {
        g_thread_unref(thread);
        thread = NULL;
    }
    return TRUE;
}

static void *
list_installed_routine(void *arg)
{
    Daemon *daemon = (Daemon *)arg;
    if (!g_mutex_trylock(&daemon->priv->lock)) {
        gdbus_isoftapp_emit_finished(GDBUS_ISOFTAPP(daemon),"", STATUS_UNKNOWN);
        return NULL;
    }

    checkDB(daemon);

    char *argv[3];
    argv[0] = (char *)"";
    argv[1] = (char *)"";
    argv[2] = (char *)"";
    listInstalled(3, argv,true, emit_list_changed_handle, daemon);
    gdbus_isoftapp_emit_finished(GDBUS_ISOFTAPP(daemon),"", STATUS_UNKNOWN);
    g_mutex_unlock(&daemon->priv->lock);
    return NULL;
}

static gboolean
daemon_list_installed(GdbusIsoftapp *object,
               GDBusMethodInvocation *invocation)
{
    Daemon *daemon = (Daemon *)object;
    GThread *thread = NULL;

    thread = g_thread_new(NULL, list_installed_routine, daemon);
    if (thread) {
        g_thread_unref(thread);
        thread = NULL;
    }

    return TRUE;
}



static void *
list_uninstalled_routine(void *arg)
{
    Daemon *daemon = (Daemon *)arg;
    if (!g_mutex_trylock(&daemon->priv->lock)) {
        gdbus_isoftapp_emit_finished(GDBUS_ISOFTAPP(daemon),"", STATUS_UNKNOWN);
        return NULL;
    }

    checkDB(daemon);

    char *argv[3];
    argv[0] = (char *)"";
    argv[1] = (char *)"";
    argv[2] = (char *)"";
    listUnInstalled(3, argv,true, emit_list_changed_handle, daemon,false);
    gdbus_isoftapp_emit_finished(GDBUS_ISOFTAPP(daemon),"", STATUS_UNKNOWN);
    g_mutex_unlock(&daemon->priv->lock);
    return NULL;
}

static gboolean
daemon_list_uninstalled(GdbusIsoftapp *object,
               GDBusMethodInvocation *invocation)
{
    Daemon *daemon = (Daemon *)object;
    GThread *thread = NULL;

    thread = g_thread_new(NULL, list_uninstalled_routine, daemon);
    if (thread) {
        g_thread_unref(thread);
        thread = NULL;
    }

    return TRUE;
}


static void *
list_all_routine(void *arg)
{
    Daemon *daemon = (Daemon *)arg;
    if (!g_mutex_trylock(&daemon->priv->lock)) {
        gdbus_isoftapp_emit_finished(GDBUS_ISOFTAPP(daemon),"", STATUS_UNKNOWN);
        return NULL;
    }

    checkDB(daemon);

    char *argv[3];
    argv[0] = (char *)"";
    argv[1] = (char *)"";
    argv[2] = (char *)"listall_123321";
    search(3, argv, emit_search_all_result_handle, daemon,true);
    gdbus_isoftapp_emit_finished(GDBUS_ISOFTAPP(daemon),"", STATUS_UNKNOWN);
    g_mutex_unlock(&daemon->priv->lock);
    return NULL;
}
static gboolean
daemon_list_all(GdbusIsoftapp *object,
                GDBusMethodInvocation *invocation)
{
    Daemon *daemon = (Daemon *)object;
    GThread *thread = NULL;

    thread = g_thread_new(NULL, list_all_routine, daemon);
    if (thread) {
        g_thread_unref(thread);
        thread = NULL;
    }

    return TRUE;
}


static void *
list_update_routine(void *arg)
{
    Daemon *daemon = (Daemon *)arg;
    if (!g_mutex_trylock(&daemon->priv->lock)) {
        gdbus_isoftapp_emit_finished(GDBUS_ISOFTAPP(daemon),"", STATUS_UNKNOWN);
        return NULL;
    }

    checkDB(daemon);

    char *argv[3];
    argv[0] = (char *)"";
    argv[1] = (char *)"";
    argv[2] = (char *)"";
    listUpdate(3, argv, emit_list_update_changed_handle, daemon);
    gdbus_isoftapp_emit_finished(GDBUS_ISOFTAPP(daemon),"", STATUS_UNKNOWN);
    g_mutex_unlock(&daemon->priv->lock);
    return NULL;
}
static gboolean
daemon_list_update(GdbusIsoftapp *object,
                GDBusMethodInvocation *invocation)
{
    Daemon *daemon = (Daemon *)object;
    GThread *thread = NULL;

    thread = g_thread_new(NULL, list_update_routine, daemon);
    if (thread) {
        g_thread_unref(thread);
        thread = NULL;
    }

    return TRUE;
}

extern t_RPMPATHMODE g_rpmPathMode; // isoftapp.cpp

static gboolean
daemon_set_path_mode(GdbusIsoftapp *object,
            GDBusMethodInvocation *invocation,
            const gchar *arg_path,
            const gchar *arg_mode)
{
    char *p = strstr(arg_path,"file://");
    if (p) {
        snprintf(g_rpmPathMode.path,511,"%s",p + strlen("file://"));
    } else {
        snprintf(g_rpmPathMode.path,511,"%s",arg_path);
    }
    g_rpmPathMode.mode = atoi(arg_mode);

    int ret = -1;
    int fd = open(ISOFTAPP_CACHE_RPMPATHMODE_FILE,O_RDWR|O_CREAT,0644);
    if (fd < 1) {
        return FALSE;
    }
    ret = write(fd,&g_rpmPathMode,sizeof(t_RPMPATHMODE));
    close(fd);

    printf("trace:%s,%d,recieved:[%s][%s]act path[%s]\n",__FUNCTION__,__LINE__,arg_path,arg_mode,g_rpmPathMode.path);

    return TRUE;
}

static gboolean
daemon_get_path_mode(GdbusIsoftapp *object,
            GDBusMethodInvocation *invocation)
{
    int ret = -1;
    int fd = open(ISOFTAPP_CACHE_RPMPATHMODE_FILE,O_RDONLY);
    if (fd < 1) {
        return FALSE;
    }
    Daemon *daemon = (Daemon *)object;
    t_RPMPATHMODE rpmPathMode;
    memset(&rpmPathMode,0,sizeof(t_RPMPATHMODE));
    ret = read(fd,&rpmPathMode,sizeof(t_RPMPATHMODE));
    close(fd);
    char path_mode[600]="";
    snprintf(path_mode,600,"%s|||%d",rpmPathMode.path,rpmPathMode.mode);
    printf("trace:%s,%d,get path mode[%s]\n",__FUNCTION__,__LINE__,path_mode);

    gdbus_isoftapp_emit_settings_changed(GDBUS_ISOFTAPP(daemon),
                                         path_mode);

    return TRUE;
}

extern int download_file(const char *src_file,const char *dst_file);
static gboolean daemon_get_icons(GdbusIsoftapp *object,
                                 GDBusMethodInvocation *invocation,
                                 const gchar *arg_name,
                                 const gchar *arg_icon,
                                 const gchar *arg_title,
                                 const gchar *arg_desc,
                                 const gchar *arg_category)
{
    if (arg_name == NULL || arg_icon == NULL )
        return FALSE;

    if (g_mkdir_with_parents(ICON_CACHE_DIR, 0775) < 0) {
        return FALSE;
    }

    char dst_file[512]="";
    snprintf(dst_file,sizeof(dst_file),"%s/%s.png",ICON_CACHE_DIR,arg_name);
    int ret = download_file(arg_icon,dst_file);

    snprintf(dst_file,sizeof(dst_file),"%s/.%s.cfg",ICON_CACHE_DIR,arg_name);
    struct icon_cache_info info;
    memset(&info, 0, sizeof(struct icon_cache_info));
    snprintf(info.name,sizeof(info.name),"%s",arg_name);
    snprintf(info.icon,sizeof(info.icon),"%s/%s.png",ICON_CACHE_DIR,arg_name); // local
    snprintf(info.title,sizeof(info.title),"%s",arg_title);
    snprintf(info.desc,sizeof(info.desc),"%s",arg_desc);
    snprintf(info.cate,sizeof(info.cate),"%s",arg_category);

    int fd = open(dst_file,O_RDWR|O_CREAT,0644);
    if (fd < 1) {
        return FALSE;
    }
    ret = write(fd,&info,sizeof(struct icon_cache_info));
    close(fd);
    fd = -1;
    return TRUE;

}

static gboolean 
daemon_get_desktop_name(GdbusIsoftapp           *object, 
                        GDBusMethodInvocation   *invocation, 
                        gchar *pkgName) 
{
    //return TRUE;
    gchar desktopName[512]="";
    getDesktopName(pkgName,desktopName);
    gdbus_isoftapp_complete_get_desktop_name(object, invocation, 
        desktopName[0]!= 0 ? desktopName : "");
    return TRUE;
}


void emit_list_check_handle(void *arg_data, char *pkgName )
{
    Daemon *daemon = (Daemon *)arg_data;

    gdbus_isoftapp_emit_check_changed(GDBUS_ISOFTAPP(daemon),
                                           pkgName);
}

static void *
check_routine(void *arg)
{
    Daemon *daemon = (Daemon *)arg;
    if (!g_mutex_trylock(&daemon->priv->lock)) {
        gdbus_isoftapp_emit_finished(GDBUS_ISOFTAPP(daemon),"", STATUS_UNKNOWN);
        return NULL;
    }

    checkDB(daemon);

    char *argv[3];
    argv[0] = (char *)"";
    argv[1] = (char *)"";
    argv[2] = (char *)daemon->priv->arg;
    check(2, argv, emit_list_check_handle, daemon);
    gdbus_isoftapp_emit_finished(GDBUS_ISOFTAPP(daemon),"", STATUS_UNKNOWN);
    g_mutex_unlock(&daemon->priv->lock);
    return NULL;
}

static gboolean
daemon_check(GdbusIsoftapp *object,
               GDBusMethodInvocation *invocation)
{
    Daemon *daemon = (Daemon *)object;
    GThread *thread = NULL;

    thread = g_thread_new(NULL, check_routine, daemon);
    if (thread) {
        g_thread_unref(thread);
        thread = NULL;
    }

    return TRUE;
}


static void
daemon_isoftapp_iface_init(GdbusIsoftappIface *iface)
{
    iface->get_daemon_version = daemon_get_daemon_version;
    iface->get_update_duration = daemon_get_update_duration;

    iface->handle_install = daemon_install;
    iface->handle_remove = daemon_remove;
    iface->handle_search = daemon_search;
    iface->handle_update = daemon_update;
    iface->handle_upgrade = daemon_upgrade;
    iface->handle_list_installed = daemon_list_installed;
    iface->handle_list_uninstalled = daemon_list_uninstalled;
    iface->handle_get_desktop_name = daemon_get_desktop_name;
    iface->handle_check = daemon_check;
    iface->handle_list_all = daemon_list_all;
    iface->handle_list_update = daemon_list_update;
    iface->handle_set_path_mode = daemon_set_path_mode;
    iface->handle_get_path_mode = daemon_get_path_mode;
    iface->handle_get_icons = daemon_get_icons;
}

static bool delete_rpm_files()
{
    const gchar *name = NULL;
    GDir *dir = NULL;

    dir = g_dir_open(g_rpmPathMode.path, 0, NULL);
    if (!dir)
        return false;

    while ((name = g_dir_read_name(dir))) {
        if (strstr(name,".rpm") == NULL)
            continue;
        gchar *filename = NULL;
        filename = g_build_filename(g_rpmPathMode.path, name, NULL);
        struct stat st;
        if (stat(filename, &st) < 0) {
            printf("%s, line %d:get file modify time error[%s]!\n", __func__, __LINE__,filename);
            g_free(filename);
            continue;
        }
        time_t now = time(NULL);
        if(((int)now - st.st_mtime ) > (7*24*60*60) ) {
            unlink(filename);
            printf("%s, line %d:will remove rpm file[%s]!\n", __func__, __LINE__,filename);
        } else {
            printf("%s, line %d:do not remove rpm file[%s],[%d]!\n", __func__, __LINE__,
                   filename,(int)now - st.st_mtime);
        }
        g_free(filename);
    }
    g_dir_close(dir);
    return true;
}

static void *
update_routine(void *arg)
{
    Daemon *daemon = (Daemon *)arg;
    if (!g_mutex_trylock(&daemon->priv->lock)) {
        gdbus_isoftapp_emit_finished(GDBUS_ISOFTAPP(daemon),"", STATUS_UNKNOWN);
        return NULL;
    }

    gdbus_isoftapp_emit_error(GDBUS_ISOFTAPP(daemon),
            ERROR_UPDATE,
            g_strdup_printf(_("updating.")),
            ERROR_CODE_UPDATEING);

    update();

    // to clean rpms in g_rpmPathMode.path
    // to deal with xxx.rpm:1-delete now;2-after one week;other-pass
    if (g_rpmPathMode.mode == 2) {
        delete_rpm_files();
    }

    printf("%s, line %d:......!\n", __func__, __LINE__);

    gdbus_isoftapp_emit_finished(GDBUS_ISOFTAPP(daemon),"", STATUS_UPDATED);

    g_mutex_unlock(&daemon->priv->lock);

    return NULL;
}

static bool daemon_update_cache_timer(gpointer user_data)
{
    printf("%s, line %d:will update cache!\n", __func__, __LINE__);
    Daemon *daemon = (Daemon *)user_data;
    GThread *thread = NULL;

    thread = g_thread_new(NULL, update_routine, daemon);

    if (thread) {
        g_thread_unref(thread);
        thread = NULL;
    }

    printf("%s, line %d:update cache! done.\n", __func__, __LINE__);
    return true;
}

static void
daemon_init(Daemon *daemon)
{
    GError *error = NULL;

    daemon->priv = DAEMON_GET_PRIVATE(daemon);

    daemon->priv->extension_ifaces = daemon_read_extension_ifaces();

    daemon->priv->context = NULL;

    daemon->priv->call_by_client = FALSE;

    daemon->priv->task_locked = FALSE;

    daemon->priv->update_duration = CACHE_UPDATE_DURATION; // 24hours

    g_mutex_init(&daemon->priv->lock);

    daemon->priv->logind = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                            G_DBUS_PROXY_FLAGS_NONE,
                                            NULL,
                                            "org.freedesktop.login1",
                                            "/org/freedesktop/login1",
                                            "org.freedesktop.login1.Manager",
                                            NULL,
                                            &error);
    if (error) {
        printf("ERROR: failed to connect systemd logind: %s\n", error->message);
        goto cleanup;
    }

    daemon->priv->nm = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                            G_DBUS_PROXY_FLAGS_NONE,
                                            NULL,
                                            "org.freedesktop.NetworkManager",
                                            "/org/freedesktop/NetworkManager",
                                            "org.freedesktop.NetworkManager",
                                            NULL,
                                            &error);
    if (daemon->priv->nm == NULL || error) {
        printf("ERROR: failed to connect NetworkManager: %s\n", error->message);
    } else {
        g_signal_connect(daemon->priv->nm, "g-signal", G_CALLBACK(nm_signal), daemon);
    }

    daemon->priv->update_timer = g_timeout_add(daemon->priv->update_duration,
                                             (GSourceFunc)daemon_update_cache_timer,
                                             daemon);
    //
    if (needUpdate()) {
        daemon_update((GdbusIsoftapp*)daemon,NULL);
    }

cleanup:
    if (error) {
        g_error_free(error);
        error = NULL;
    }
}
static void
daemon_finalize(GObject *object)
{
    Daemon *daemon;

    g_return_if_fail(IS_DAEMON(object));

    daemon = DAEMON(object);

    if (daemon->priv->bus_connection) {
        g_object_unref(daemon->priv->bus_connection);
        daemon->priv->bus_connection = NULL;
    }


    if (daemon->priv->logind) {
        g_object_unref(daemon->priv->logind);
        daemon->priv->logind = NULL;
    }

    if (daemon->priv->nm) {
        g_object_unref(daemon->priv->nm);
        daemon->priv->nm = NULL;
    }

    G_OBJECT_CLASS(daemon_parent_class)->finalize(object);
}

static void
daemon_get_property(GObject    *object,
                    guint       prop_id,
                    GValue     *value,
                    GParamSpec *pspec)
{
    Daemon *daemon = DAEMON(object);
    switch (prop_id) {
    case PROP_DAEMON_VERSION:
        g_value_set_string(value, PROJECT_VERSION);
        break;

    case PROP_UPDATE_DURATION:
        g_value_set_int64(value, daemon->priv->update_duration);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}


static void
daemon_set_property(GObject      *object,
                    guint         prop_id,
                    const GValue *value,
                    GParamSpec   *pspec)
{
    switch (prop_id) {
    case PROP_DAEMON_VERSION:
        g_assert_not_reached();
        break;

    case PROP_UPDATE_DURATION:
        g_assert_not_reached();
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}
static void
daemon_class_init(DaemonClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->finalize = daemon_finalize;
    object_class->get_property = daemon_get_property;
    object_class->set_property = daemon_set_property;

    g_type_class_add_private(klass, sizeof(DaemonPrivate));

    g_object_class_override_property(object_class,
                                     PROP_DAEMON_VERSION,
                                     "daemon-version");
}

Daemon *
daemon_new()
{
    Daemon *daemon;

    daemon = DAEMON(g_object_new(TYPE_DAEMON, NULL));

    if (!register_isoftapp_daemon(DAEMON(daemon))) {
        g_object_unref(daemon);
        goto error;
    }

    return daemon;

error:
    return NULL;
}


static GMainLoop *loop;

static gboolean
ensure_directory(const char  *path,
                 GError     **error)
{
    if (g_mkdir_with_parents(path, 0775) < 0) {
        g_set_error(error,
                    G_FILE_ERROR,
                    g_file_error_from_errno(errno),
                    "Failed to create directory %s: %m",
                    path);
        return FALSE;
    }
    return TRUE;
}

static void
on_bus_acquired(GDBusConnection  *connection,
                const gchar      *name,
                gpointer          user_data)
{
    Daemon *daemon;
    GError *local_error = NULL;
    GError **error = &local_error;

    daemon = daemon_new();
    if (daemon == NULL) {
        g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED,
                    "Failed to initialize daemon");
        goto out;
    }

    openlog("isoftapp-daemon", LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "started daemon version %s", PROJECT_VERSION);
    closelog();
    openlog("isoftapp-daemon", 0, LOG_AUTHPRIV);

 out:
    if (local_error != NULL) {
        g_printerr ("%s\n", local_error->message);
        g_clear_error (&local_error);
        g_main_loop_quit (loop);
    }
}

static void
on_name_lost(GDBusConnection  *connection,
             const gchar      *name,
             gpointer          user_data)
{
    g_debug("got NameLost, exiting");
    g_main_loop_quit(loop);
}


static gboolean debug;
static void
on_log_debug(const gchar *log_domain,
             GLogLevelFlags log_level,
             const gchar *message,
             gpointer user_data)
{
    GString *string;
    const gchar *progname;
    int ret G_GNUC_UNUSED;

    string = g_string_new(NULL);

    progname = g_get_prgname();

    g_string_append_printf(string, "%s",message ? message : "(NULL) message");
    //write_log(LOG_DEBUG,string->str);

    if (string) {
        g_string_free(string,true);
    }
}

static void
log_handler(const gchar   *domain,
            GLogLevelFlags level,
            const gchar   *message,
            gpointer       data)
{
    if ((level & G_LOG_LEVEL_MASK) == G_LOG_LEVEL_DEBUG && !debug)
        return;

    g_log_default_handler(domain, level, message, data);
}

static gboolean
on_signal_quit(gpointer data)
{
    g_main_loop_quit((GMainLoop*)data);
    return FALSE;
}

int isoftAppDaemon(int argc, char *argv[])
{
    GError *error;
    gint ret;
    GBusNameOwnerFlags flags;
    GOptionContext *context;
    static gboolean replace;
    static gboolean show_version;
    static GOptionEntry entries[] = {
        { "version", 0, 0, G_OPTION_ARG_NONE, &show_version, N_("Output version information and exit"), NULL },
        { "replace", 0, 0, G_OPTION_ARG_NONE, &replace, N_("Replace existing instance"), NULL },
        { "debug", 0, 0, G_OPTION_ARG_NONE, &debug, N_("Enable debugging code"), NULL },

        { NULL }
    };

    ret = 1;
    error = NULL;

    printf("entering main function,pid[%d]",(int)getpid());

    setlocale(LC_ALL, "");
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

#if !GLIB_CHECK_VERSION(2, 35, 3)
    g_type_init();
#endif

    if (!g_setenv("GIO_USE_VFS", "local", TRUE)) {
        printf("Couldn't set GIO_USE_GVFS");
        goto out;
    }

    context = g_option_context_new("");
    g_option_context_set_translation_domain(context, GETTEXT_PACKAGE);
    g_option_context_set_summary(context, _("Provides D-Bus interfaces for querying and manipulating\niSOFT App information."));
    g_option_context_add_main_entries(context, entries, NULL);
    error = NULL;
    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        printf(error->message);
        g_error_free(error);
        goto out;
    }
    g_option_context_free(context);

    if (show_version) {
        g_print("isoftapp-daemon   PROJECT_VERSION  \n");
        ret = 0;
        goto out;
    }

    /* If --debug, then print debug messages even when no G_MESSAGES_DEBUG */
    if (debug && !g_getenv("G_MESSAGES_DEBUG")) {
        g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, on_log_debug, NULL);
        //set_write_log_level(LOG_DEBUG);
    } else {
        //set_write_log_level(LOG_DEBUG);
    }
    g_log_set_default_handler(log_handler, NULL);

    flags = G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT;
    if (replace)
        flags |= G_BUS_NAME_OWNER_FLAGS_REPLACE;
    g_bus_own_name(G_BUS_TYPE_SYSTEM,
                   NAME_TO_CLAIM,
                   flags,
                   on_bus_acquired,
                   NULL,
                   on_name_lost,
                   NULL,
                   NULL);

    loop = g_main_loop_new(NULL, FALSE);

    g_unix_signal_add(SIGINT, on_signal_quit, loop);
    g_unix_signal_add(SIGTERM, on_signal_quit, loop);
    printf("entering main loop.");

    g_main_loop_run(loop);

    printf("exiting.");
    g_main_loop_unref(loop);

    ret = 0;

 out:
    printf("exit main function,pid[%d].",(int)getpid());
    return ret;
}

