
/*! @brief A member of a group which represent a single device */
struct OSyncMember {
#ifndef DOXYGEN_SHOULD_SKIP_THIS
	long long int id;
	char *configdir;
	char *configdata;
	int configsize;
	OSyncPlugin *plugin;
	OSyncMemberFunctions *memberfunctions;
	OSyncGroup *group;
	
	void *enginedata;
	void *plugindata;
	
	GList *format_sinks;
	GList *objtype_sinks;
	char *pluginname;
	
	//For the filters
	GList *accepted_objtypes;
	GList *filters;

	char *extension;
#endif
};

OSyncObjTypeSink *osync_member_find_objtype_sink(OSyncMember *member, const char *objtypestr);
void osync_member_select_format(OSyncMember *member, OSyncObjTypeSink *objsink);
osync_bool osync_member_instance_default_plugin(OSyncMember *member, OSyncError **error);
