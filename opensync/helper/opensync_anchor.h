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
 
#ifndef OPENSYNC_ANCHOR_H_
#define OPENSYNC_ANCHOR_H_

OSYNC_EXPORT osync_bool osync_anchor_compare(const char *anchordb, const char *key, const char *new_anchor);
OSYNC_EXPORT void osync_anchor_update(const char *anchordb, const char *key, const char *new_anchor);
OSYNC_EXPORT char *osync_anchor_retrieve(const char *anchordb, const char *key);

#endif /* OPENSYNC_ANCHOR_H_ */
