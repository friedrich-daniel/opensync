OSyncChange *osync_change_new(void);
OSyncChangeType osync_change_get_changetype(OSyncChange *change);
void osync_change_set_hash(OSyncChange *change, const char *hash);
void osync_change_set_uid(OSyncChange *change, const char *uid);
void osync_change_set_data(OSyncChange *change, char *data, int size, osync_bool has_data);
int osync_change_set_datasize(OSyncChange *change);
void osync_change_set_objformat(OSyncChange *change, OSyncObjFormat *format);
OSyncObjType *osync_change_get_objtype(OSyncChange *change);
void osync_change_set_changetype(OSyncChange *change, OSyncChangeType type);
char *osync_change_get_hash(OSyncChange *change);
char *osync_change_get_uid(OSyncChange *change);
char *osync_change_get_data(OSyncChange *change);
int osync_change_get_datasize(OSyncChange *change);
OSyncObjFormat *osync_change_get_objformat(OSyncChange *change);
void osync_report_change(OSyncContext *context, OSyncChange *change);
OSyncMapping *osync_change_get_mapping(OSyncChange *entry);
void *osync_change_get_engine_data(OSyncChange *change);
void osync_change_set_engine_data(OSyncChange *change, void *engine_data);
OSyncMember *osync_change_get_member(OSyncChange *change);
void osync_change_unmarshal(OSyncMappingTable *, OSyncChange *change, const void *data);
void osync_change_update(OSyncChange *source, OSyncChange *target);
void osync_change_set_objtype(OSyncChange *change, OSyncObjType *type);
void osync_change_set_objtype_string(OSyncChange *change, const char *name);
void osync_change_set_info(OSyncChange *change, char *data, int size);
char *osync_change_get_info(OSyncChange *change);
int osync_change_get_infosize(OSyncChange *change);
void osync_change_append_objformat(OSyncChange *change, OSyncObjFormat *objformat);
void osync_change_set_member(OSyncChange *change, OSyncMember *member);
void osync_change_set_objformat_string(OSyncChange *change, const char *name);
void osync_change_prepend_objformat(OSyncChange *change, OSyncObjFormat *objformat);
long long int osync_change_get_id(OSyncChange *change);
osync_bool osync_change_has_data(OSyncChange *change);
void osync_change_free(OSyncChange *change);
void osync_change_reset(OSyncChange *change);
