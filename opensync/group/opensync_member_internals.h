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

#ifndef _OPENSYNC_MEMBER_INTERNALS_H_
#define _OPENSYNC_MEMBER_INTERNALS_H_

/*! @brief A member of a group which represent a single device */
struct OSyncMember {
	long long int id;
	char *configdir;
	
	OSyncPluginConfig *config;
	
	//OSyncGroup *group;

	char *name;
	char *pluginname;
	
	GList *objtypes; /* OSyncObjTypeSink */
	OSyncObjTypeSink *main_sink;

	//For the filters
	GList *filters;
	int ref_count;
	
	OSyncCapabilities *capabilities;
	OSyncMerger *merger;

#ifdef OPENSYNC_UNITTESTS
	char *schemadir;
#endif /* OPENSYNC_UNITTESTS */
};

#ifdef OPENSYNC_UNITTESTS
OSYNC_TEST_EXPORT void osync_member_set_schemadir(OSyncMember *member, const char *schemadir);
#endif

#endif /* _OPENSYNC_MEMBER_INTERNALS_H_ */
