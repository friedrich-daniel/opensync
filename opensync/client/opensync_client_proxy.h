/*
 * libosengine - A synchronization engine for the opensync framework
 * Copyright (C) 2004-2005  Armin Bauer <armin.bauer@opensync.org>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 * 
 */
 
#ifndef OSYNC_CLIENT_PROXY_H_
#define OSYNC_CLIENT_PROXY_H_

typedef void (* proxy_init_cb) (OSyncClientProxy *proxy, void *userdata);

typedef void (* initialize_cb) (OSyncClientProxy *proxy, void *userdata, OSyncError *error);
typedef void (* finalize_cb) (OSyncClientProxy *proxy, void *userdata, OSyncError *error);
typedef void (* discover_cb) (OSyncClientProxy *proxy, void *userdata, OSyncError *error);
typedef void (* connect_cb) (OSyncClientProxy *proxy, void *userdata, OSyncError *error);
typedef void (* disconnect_cb) (OSyncClientProxy *proxy, void *userdata, OSyncError *error);

OSyncClientProxy *osync_client_proxy_new(OSyncError **error);
void osync_client_proxy_ref(OSyncClientProxy *proxy);
void osync_client_proxy_unref(OSyncClientProxy *proxy);

void osync_client_proxy_set_context(OSyncClientProxy *proxy, GMainContext *ctx);

osync_bool osync_client_proxy_spawn(OSyncClientProxy *proxy, OSyncStartType type, const char *path, OSyncError **error);
osync_bool osync_client_proxy_shutdown(OSyncClientProxy *proxy, OSyncError **error);

osync_bool osync_client_proxy_initialize(OSyncClientProxy *proxy, initialize_cb callback, void *userdata, const char *formatdir, const char *plugindir, const char *plugin, OSyncError **error);
osync_bool osync_client_proxy_finalize(OSyncClientProxy *proxy, finalize_cb callback, void *userdata, OSyncError **error);

osync_bool osync_client_proxy_discover(OSyncClientProxy *proxy, discover_cb callback, void *userdata, OSyncError **error);
int osync_client_proxy_num_objtypes(OSyncClientProxy *proxy);
OSyncObjTypeSink *osync_client_proxy_nth_objtype(OSyncClientProxy *proxy, int nth);

osync_bool osync_client_proxy_connect(OSyncClientProxy *proxy, connect_cb callback, void *userdata, const char *objtype, OSyncError **error);
osync_bool osync_client_proxy_disconnect(OSyncClientProxy *proxy, disconnect_cb callback, void *userdata, const char *objtype, OSyncError **error);

#endif /*OSYNC_CLIENT_PROXY_H_*/
