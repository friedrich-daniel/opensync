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

#ifndef _OPENSYNC_DATA_PRIVATE_H_
#define _OPENSYNC_DATA_PRIVATE_H_

/**
 * @defgroup OSyncDataPrivate OpenSync Data Module Private API
 * @ingroup OSyncPrivate
 * @defgroup OSyncDataPrivateAPI OpenSync Data Private
 * @ingroup OSyncDataPrivate
 * @brief The private part of the OSyncData API
 * 
 */
/*@{*/

/** @brief A data object */
struct OSyncData {
	/** The data reported from the plugin */
	char *data;
	/** The size of the data from the plugin */
	int size;
	/** The name of the object type */
	char *objtype;
	/** The name of the format */
	OSyncObjFormat *objformat;
	int ref_count;
};

/*@}*/

#endif /* _OPENSYNC_DATA_PRIVATE_H_ */
