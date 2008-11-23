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

#ifndef _OPENSYNC_CHANGE_INTERNALS_H_
#define _OPENSYNC_CHANGE_INTERNALS_H_

OSyncChange *osync_change_clone(OSyncChange *source, OSyncError **error);
OSyncConvCmpResult osync_change_compare(OSyncChange *leftchange, OSyncChange *rightchange);
osync_bool osync_change_duplicate(OSyncChange *change, osync_bool *dirty, OSyncError **error);

#endif /*_OPENSYNC_CHANGE_INTERNALS_H_*/
