
/*
 * libopensync - A synchronization framework
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

#ifndef OPENSYNC_CLIENT_INTERNALS_H_
#define OPENSYNC_CLIENT_INTERNALS_H_

OSYNC_TEST_EXPORT void osync_client_shutdown(OSyncClient *client);
OSYNC_TEST_EXPORT void osync_client_error_shutdown(OSyncClient *client, OSyncError *error);
void osync_client_disconnect(OSyncClient *client);

OSYNC_TEST_EXPORT osync_bool osync_client_run_external(OSyncClient *client, char *pipe_path, OSyncPlugin *plugin, OSyncError **error);

#endif /*OPENSYNC_CLIENT_INTERNALS_H_*/
