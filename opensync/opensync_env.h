OSyncEnv *osync_env_new	(void);
void osync_env_free(OSyncEnv *env);

osync_bool osync_env_initialize(OSyncEnv *env, OSyncError **error);
osync_bool osync_env_finalize(OSyncEnv *env, OSyncError **error);

int osync_env_num_plugins (OSyncEnv *osstruct);
OSyncPlugin *osync_env_nth_plugin(OSyncEnv *osstruct, int nth);
osync_bool osync_env_load_plugins(OSyncEnv *env, const char *path, OSyncError **oserror);
OSyncPlugin *osync_env_find_plugin(OSyncEnv *env, const char *name);

void osync_env_remove_group(OSyncEnv *osstruct, OSyncGroup *group);
OSyncGroup *osync_env_find_group(OSyncEnv *env, const char *name);
int osync_env_num_groups(OSyncEnv *env);
void osync_env_append_group(OSyncEnv *os_env, OSyncGroup *group);
OSyncGroup *osync_env_nth_group(OSyncEnv *osinfo, int nth);

const char *osync_env_get_configdir(OSyncEnv *osinfo);
void osync_env_set_configdir(OSyncEnv *osinfo, const char *path);
void *osync_get_data(OSyncEnv *osync);
void osync_set_data(OSyncEnv *osync, void *data);

osync_bool osync_env_load_groups(OSyncEnv *osyncinfo, const char *path, OSyncError **error);

osync_bool osync_file_write(const char *filename, const char *data, int size, int mode, OSyncError **error);
osync_bool osync_file_read(const char *filename, char **data, int *size, OSyncError **error);