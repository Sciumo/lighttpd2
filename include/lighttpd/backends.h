#ifndef _LIGHTTPD_BACKENDS_H_
#define _LIGHTTPD_BACKENDS_H_

#include <lighttpd/base.h>

typedef struct liGenericBackendCallbacks liGenericBackendCallbacks;
typedef struct liGenericBackend liGenericBackend;
typedef struct liBackendPool liBackendPool;

struct liGenericBackendCallbacks {
};

/* members marked with "[pool]" are protected by the pool lock */
struct liGenericBackend {
	gint refcount;
	gint ndx; /* [pool] */
	gboolean active; /* [pool] */
	ev_tstamp ts_idle; /* [pool] */

	gpointer data;
	const liGenericBackendCallbacks *callbacks;

	liWorker *worker, *worker_next, *worker_dest; /* [pool] */
};

struct liBackendPool {
	GMutex *lock;
	liWorker *wrk; /* "master" worker for pool watchers */

	GPtrArray *backends;
	GArray *pending_connections;

	/* waiting vrequests */
	GQueue vr_wait_queue, idle_queue;
	ev_tstamp ts_vr_wait, ts_idle, ts_connect, ts_disabled;
	ev_tstamp ts_timeout; /* new timeout */

	int cur_connect_fd;
	ev_io cur_connect_watcher;

	GPtrArray *update_backends;

	ev_async update_watcher;
	ev_periodic timeout_watcher;

	gboolean shutdown;


	/* READ ONLY CONFIGURATION DATA */

	liSocketAddress sock_addr;

	/* >0: real limit for current connections + pendings connects
	 * <0: unlimited connections, absolute value limits the number of pending connects
	 * =0: no limit */
	int max_connections;

	/* how long we wait on keep-alive connections. 0: no keep-alive */
	guint idle_timeout;

	/* how long we wait for connect to succeed, must be > 0; when connect fails the pool gets "disabled". */
	guint connect_timeout;

	 /* how long a vrequest is allowed to wait for a connect  before we return an error. if pool gets disabled all requests fail.
	  * if a pending connect is assigned to a vrequest wait_timeout is not active.
	  */
	guint wait_timeout;

	/* how long the pool stays disabled. even if this is 0, all vrequests will receive an error on disable */
	guint disable_time;

	/* max requests per connection. -1: unlimited */
	int max_requests;
};

LI_API void li_backend_;

#endif
