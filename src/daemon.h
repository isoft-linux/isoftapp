/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
*/

#ifndef __DAEMON_H__
#define __DAEMON_H__

#include "isoftapp-generated.h"


G_BEGIN_DECLS

#define TYPE_DAEMON         (daemon_get_type())
#define DAEMON(o)           (G_TYPE_CHECK_INSTANCE_CAST((o), TYPE_DAEMON, Daemon))
#define DAEMON_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), TYPE_DAEMON, DaemonClass))
#define IS_DAEMON(o)        (G_TYPE_CHECK_INSTANCE_TYPE((o), TYPE_DAEMON))
#define IS_DAEMON_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE((k), TYPE_DAEMON))
#define DAEMON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), TYPE_DAEMON, DaemonClass))

typedef struct DaemonClass DaemonClass;
typedef struct DaemonPrivate DaemonPrivate;

struct Daemon {
    GdbusIsoftappSkeleton parent;
    DaemonPrivate *priv;
};
typedef struct Daemon Daemon;
struct DaemonClass {
    GdbusIsoftappSkeletonClass parent_class;
};

typedef enum {
    STATUS_INSTALL,
    STATUS_UPGRADE,
    STATUS_REMOVE,
    STATUS_INSTALLED,
    STATUS_REMOVED,
    STATUS_UPDATED,
    STATUS_UPGRADED,
    STATUS_SEARCHED,
    STATUS_INSTALL_ERROR,
    STATUS_REMOVE_ERROR,
    STATUS_UPGRADE_ERROR,
    STATUS_DOWNLOAD,
    STATUS_UNKNOWN,
} Status;

typedef enum {
    INSTALLED,
    UNINSTALLED,
    SEARCH_UNKNOWN,
} SearchStatus;

typedef enum {
    ERROR_FAILED,
    ERROR_PERMISSION_DENIED,
    ERROR_NOT_SUPPORTED,
    ERROR_SEARCH,
    ERROR_TASK_LOCKED,
    ERROR_CHECK,
    ERROR_INSTALL,
    ERROR_REMOVE,
    ERROR_UPDATE,
    ERROR_PKG_NOT_EXIST,
    ERROR_PKG_NOT_REMOVED,
    ERROR_PKG_NOT_UPGRADED,
    NUM_ERRORS
} Error;

typedef enum {
    ERROR_CODE_NETWORK,
    ERROR_CODE_DOWNLOAD,
    ERROR_CODE_INSTALL,
    ERROR_CODE_REMOVE,
    ERROR_CODE_SEARCH,
    ERROR_CODE_UPDATEING,
    ERROR_CODE_OTHERS
} ErrorCode;

#define ERROR error_quark()

GType error_get_type();
#define TYPE_ERROR (error_get_type())
GQuark error_quark();

GType   daemon_get_type              (void) G_GNUC_CONST;
Daemon *daemon_new                   (void);

/* local methods */

GHashTable * daemon_read_extension_ifaces();
GHashTable * daemon_get_extension_ifaces(Daemon *daemon);


int isoftAppDaemon(int argc, char *argv[]);

#define  ISOFTAPP_CACHE_RPMPATHMODE_FILE  ISOFTAPP_CACHE_DIR"/rpmpathmode.cfg"

typedef struct {
    char path[512]; // default:ISOFTAPP_CACHE_DIR/rpms
    int  mode;      // 1-delete now;2-after one week;other-pass
} t_RPMPATHMODE;

G_END_DECLS

#endif /* __DAEMON_H__ */


