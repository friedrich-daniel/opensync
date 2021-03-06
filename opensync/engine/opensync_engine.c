/*
 * libopensync - A synchronization engine for the opensync framework
 * Copyright (C) 2004-2005  Armin Bauer <armin.bauer@opensync.org>
 * Copyright (C) 2007       Daniel Gollub <dgollub@suse.de>
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
 
#include "opensync.h"
#include "opensync_internals.h"

#include "opensync-client.h"
#include "opensync-engine.h"
#include "opensync-group.h"
#include "opensync-format.h"
#include "opensync-data.h"
#include "opensync-plugin.h"
#include "opensync-archive.h"
#include "opensync-merger.h"
#include "opensync-xmlformat.h"

#include "archive/opensync_archive_internals.h"
#include "client/opensync_client_proxy_internals.h"

#include "opensync_status_internals.h"

#include "opensync_engine.h"
#include "opensync_engine_private.h"
#include "opensync_engine_internals.h"

#ifdef OPENSYNC_UNITTESTS
#include "xmlformat/opensync-xmlformat_internals.h"
#endif

static void osync_engine_set_error(OSyncEngine *engine, OSyncError *error)
{
  osync_assert(engine);
  if (engine->error) {
    osync_error_stack(&error, &engine->error);
    osync_error_unref(&engine->error);
  }
	
  engine->error = error;
  if (error)
    osync_error_ref(&error);
}

static void _finalize_callback(OSyncClientProxy *proxy, void *userdata, OSyncError *error)
{
  OSyncEngine *engine = userdata;
  osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, proxy, userdata, error);
	
  if (error) {
    osync_engine_set_error(engine, error);
  }
	
  engine->busy = FALSE;
	
  osync_trace(TRACE_EXIT, "%s", __func__);
}

static gboolean _command_prepare(GSource *source, gint *timeout_)
{
  *timeout_ = 1;
  return FALSE;
}

static gboolean _command_check(GSource *source)
{
  OSyncEngine *engine = *((OSyncEngine **)(source + 1));
  if (g_async_queue_length(engine->command_queue) > 0)
    return TRUE;
	
  return FALSE;
}

void osync_engine_command(OSyncEngine *engine, OSyncEngineCommand *command);

static OSyncObjFormat *_osync_engine_get_internal_format(OSyncEngine *engine, const char *objtype)
{
  char *format = g_hash_table_lookup(engine->internalFormats, objtype);
  if (!format)
    return NULL;
  return osync_format_env_find_objformat(engine->formatenv, format);
}

static int _osync_engine_get_proxy_position(OSyncEngine *engine, OSyncClientProxy *proxy)
{
  int ret = 0;
  osync_assert(engine);
  osync_assert(proxy);

  ret = g_list_index(engine->proxies, proxy);

  osync_assert(ret >= 0);

  return ret;
}

static osync_bool _osync_engine_is_proxy_connected(OSyncEngine *engine, OSyncClientProxy *proxy)
{
  osync_assert(engine);
  osync_assert(proxy);

  return !!(engine->proxy_connects & (1 << _osync_engine_get_proxy_position(engine, proxy)));
}

static int _osync_engine_get_objengine_position(OSyncEngine *engine, OSyncObjEngine *objengine)
{
  int ret = 0;
  osync_assert(engine);
  osync_assert(objengine);

  ret = g_list_index(engine->object_engines, objengine);

  osync_assert(ret >= 0);

  return ret;
}

gboolean foreach_schema(void *key, void *value, void *userdata) {
  osync_xmlformat_schema_unref((OSyncXMLFormatSchema *)value);
  return TRUE;
}

static void _osync_engine_finalize_internal_schemas(OSyncEngine *engine)
{

  if ( engine->internalSchemas != NULL ) {
    g_hash_table_foreach_remove(engine->internalSchemas, foreach_schema, NULL);
  } 

}

static void _osync_engine_set_internal_schema(OSyncEngine *engine, const char *objtype, OSyncError **error) 
{
  OSyncXMLFormat *xmlformat = NULL;
  OSyncXMLFormatSchema *schema = NULL;
  osync_trace(TRACE_INTERNAL, "Setting internal schema for objtype %s", objtype);

  // init OSyncXMLFormatSchemas
  xmlformat = osync_xmlformat_new(objtype, NULL);
#ifdef OPENSYNC_UNITTESTS
  schema = osync_xmlformat_schema_get_instance_with_path(xmlformat, engine->schema_dir, error);
#else
  schema = osync_xmlformat_schema_get_instance(xmlformat, error);
#endif
  osync_xmlformat_unref(xmlformat);
  g_hash_table_insert(engine->internalSchemas, g_strdup(objtype), schema);
}

static void _osync_engine_set_internal_format(OSyncEngine *engine, const char *objtype, OSyncObjFormat *format)
{
  osync_trace(TRACE_INTERNAL, "Setting internal format of %s to %p:%s", objtype, format, osync_objformat_get_name(format));
  if (!format)
    return;
  g_hash_table_insert(engine->internalFormats, g_strdup(objtype), g_strdup(osync_objformat_get_name(format)));
}

static OSyncFormatConverterPath *_osync_engine_get_converter_path(OSyncEngine *engine, const char *member_objtype)
{
  OSyncFormatConverterPath *converter_path = g_hash_table_lookup(engine->converterPathes, member_objtype);
  return converter_path;
}

static void _osync_engine_set_converter_path(OSyncEngine *engine, const char *member_objtype, OSyncFormatConverterPath *converter_path)
{
  osync_trace(TRACE_INTERNAL, "Setting converter_path of %s to %p", member_objtype, converter_path);
  if (!converter_path)
    return;
  g_hash_table_insert(engine->converterPathes, g_strdup(member_objtype), converter_path);
}

static void _osync_engine_converter_path_unref(gpointer data) {
  OSyncFormatConverterPath * converter_path = data;
  osync_converter_path_unref(converter_path);
}

static void _osync_engine_receive_change(OSyncClientProxy *proxy, void *userdata, OSyncChange *change)
{
  OSyncEngine *engine = userdata;
  OSyncError *error = NULL;
  osync_bool found = FALSE;
  OSyncMember *member = NULL;
  long long int memberid = 0;
  const char *uid = NULL;
  OSyncChangeType changetype = 0;
  const char *format = NULL;
  const char *objtype = NULL;
  OSyncObjTypeSink *objtype_sink = NULL;
  char *member_objtype = NULL;
  OSyncData *data = NULL;
  OSyncObjFormat *internalFormat = NULL;
	
  osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, proxy, userdata, change);

  member = osync_client_proxy_get_member(proxy);
  memberid = osync_member_get_id(member);
  uid = osync_change_get_uid(change);		
  changetype = osync_change_get_changetype(change);
  format = osync_objformat_get_name(osync_change_get_objformat(change));
  objtype = osync_change_get_objtype(change);

  objtype_sink = osync_member_find_objtype_sink(member, objtype);

  osync_trace(TRACE_INTERNAL, "Received change %s, changetype %i, format %s, objtype %s from member %lli", uid, changetype, format, objtype, memberid);
  member_objtype = g_strdup_printf("%lli_%s", memberid, objtype); 

  data = osync_change_get_data(change);

  /* Convert the format to the internal format */
  internalFormat = _osync_engine_get_internal_format(engine, osync_change_get_objtype(change));
  osync_trace(TRACE_INTERNAL, "common format %p for objtype %s", internalFormat, osync_change_get_objtype(change));

  /* Only convert if the engine is allowed to convert and if a internal format is available. 
     The reason that the engine isn't allowed to convert could be backup. dumping the changes. 
     Do not convert anything if the chagetype is DELETED. */
  if (internalFormat && osync_group_get_converter_enabled(engine->group) && (osync_change_get_changetype(change) != OSYNC_CHANGE_TYPE_DELETED)) {
    OSyncFormatConverterPath *path = NULL;
    OSyncObjFormatSink *formatsink = NULL;
    osync_trace(TRACE_INTERNAL, "converting to common format %s", osync_objformat_get_name(internalFormat));

    path = _osync_engine_get_converter_path(engine, member_objtype);
    if(!path) {
      path = osync_format_env_find_path_with_detectors(engine->formatenv, osync_change_get_data(change), internalFormat, NULL, &error);
      _osync_engine_set_converter_path(engine, member_objtype, path);
    }

    if (!path)
      goto error;
	
    formatsink = osync_objtype_sink_find_objformat_sink(objtype_sink, internalFormat);
    if (formatsink) {
      const char *config = osync_objformat_sink_get_config(formatsink); 
      osync_converter_path_set_config(path, config);
    }

    if (!osync_format_env_convert(engine->formatenv, path, data, &error)) {
      goto error;
    }
  }
	
  /* Merger - Merge lost information to the change (don't merger anything when changetype is DELETED.) */
  if( osync_group_get_merger_enabled(engine->group) &&
      osync_group_get_converter_enabled(engine->group) &&	
      (osync_change_get_changetype(change) != OSYNC_CHANGE_TYPE_DELETED) &&
      /* only use the merger if the objformat name starts with "xmlformat-" (10 chars) */
      ( !strncmp(osync_objformat_get_name(osync_change_get_objformat(change)), "xmlformat-", 10)))

    {
      char *buffer = NULL;
      unsigned int xmlformat_size = 0, size = 0;
      OSyncXMLFormat *xmlformat = NULL;
      OSyncXMLFormat *xmlformat_entire = NULL;
      const char *objtype = NULL;
      OSyncMerger *merger = NULL;
      osync_trace(TRACE_INTERNAL, "Merge the XMLFormat.");

      objtype = osync_change_get_objtype(change);
		
      member = osync_client_proxy_get_member(proxy);
      merger = osync_member_get_merger(member);
      if(merger) {
        /* TODO: Merger save the archive data with the member so we have to load it only for one time*/
        // osync_archive_load_data() is fetching the mappingid by uid in the db
        int ret = osync_archive_load_data(engine->archive, uid, objtype, &buffer, &size, &error);
        if (ret < 0) {
          goto error; 
        }
			
        if (ret > 0) {
          xmlformat_entire = osync_xmlformat_parse(buffer, size, &error);
          free(buffer);
          if(!xmlformat_entire)
            goto error;
					
          osync_data_get_data(osync_change_get_data(change), (char **) &xmlformat, &xmlformat_size);
          osync_assert(xmlformat_size == osync_xmlformat_size());

          osync_merger_merge(merger, xmlformat, xmlformat_entire);
          osync_xmlformat_unref(xmlformat_entire);
        }
      }
    }
	
  /* Search for the correct objengine */
  {GList * o = NULL;
    for (o = engine->object_engines; o; o = o->next) {
      OSyncObjEngine *objengine = o->data;
      if (!strcmp(osync_change_get_objtype(change), osync_obj_engine_get_objtype(objengine))) {
        found = TRUE;
        if (!osync_obj_engine_receive_change(objengine, proxy, change, &error))
          goto error;
        break;
      }	
    }}
	
  if (!found) {
    osync_error_set(&error, OSYNC_ERROR_GENERIC, "Unable to find engine which can handle objtype %s", osync_change_get_objtype(change));
    goto error;
  }

  g_free(member_objtype);
	
  osync_trace(TRACE_EXIT, "%s", __func__);
  return;

 error:
  g_free(member_objtype);
	
  osync_engine_set_error(engine, error);
  osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_ERROR, NULL, error);
  osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&error));
}

/* This function is called from the master thread. The function dispatched incoming data from
 * the remote end */
static gboolean _command_dispatch(GSource *source, GSourceFunc callback, gpointer user_data)
{
  OSyncEngine *engine = user_data;
  OSyncEngineCommand *command = NULL;
	
  osync_trace(TRACE_ENTRY, "%s(%p)", __func__, user_data);
	
  while ((command = g_async_queue_try_pop(engine->command_queue))) {
    /* We check if the message is a reply to something */
    osync_trace(TRACE_INTERNAL, "Dispatching %p: %i", command, command->cmd);
		
    osync_engine_command(engine, command);
    g_free(command);
  }
	
  osync_trace(TRACE_EXIT, "%s: Done dispatching", __func__);
  return TRUE;
}

osync_bool osync_engine_mapping_solve(OSyncEngine *engine, OSyncMappingEngine *mapping_engine, OSyncChange *change, OSyncError **error)
{
  OSyncEngineCommand *cmd = NULL;
  osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, mapping_engine, change, error);
	
  cmd = osync_try_malloc0(sizeof(OSyncEngineCommand), error);
  if (!cmd) {
    osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
    return FALSE;
  }
	
  cmd->cmd = OSYNC_ENGINE_COMMAND_SOLVE;
  cmd->mapping_engine = mapping_engine;
  cmd->master = change;
  cmd->solve_type = OSYNC_ENGINE_SOLVE_CHOOSE;
	
  g_async_queue_push(engine->command_queue, cmd);
	
  osync_trace(TRACE_EXIT, "%s", __func__);
  return TRUE;
}

osync_bool osync_engine_mapping_duplicate(OSyncEngine *engine, OSyncMappingEngine *mapping_engine, OSyncError **error)
{
  OSyncEngineCommand *cmd = NULL;
  osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, mapping_engine, error);
	
  cmd = osync_try_malloc0(sizeof(OSyncEngineCommand), error);
  if (!cmd) {
    osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
    return FALSE;
  }
	
  cmd->cmd = OSYNC_ENGINE_COMMAND_SOLVE;
  cmd->mapping_engine = mapping_engine;
  cmd->solve_type = OSYNC_ENGINE_SOLVE_DUPLICATE;
	
  g_async_queue_push(engine->command_queue, cmd);
	
  osync_trace(TRACE_EXIT, "%s", __func__);
  return TRUE;
}

osync_bool osync_engine_mapping_ignore_conflict(OSyncEngine *engine, OSyncMappingEngine *mapping_engine, OSyncError **error)
{
  OSyncEngineCommand *cmd = NULL;
  osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, mapping_engine, error);
	
  cmd = osync_try_malloc0(sizeof(OSyncEngineCommand), error);
  if (!cmd) {
    osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
    return FALSE;
  }
	
  cmd->cmd = OSYNC_ENGINE_COMMAND_SOLVE;
  cmd->mapping_engine = mapping_engine;
  cmd->solve_type = OSYNC_ENGINE_SOLVE_IGNORE;
	
  g_async_queue_push(engine->command_queue, cmd);
	
  osync_trace(TRACE_EXIT, "%s", __func__);
  return TRUE;
}

osync_bool osync_engine_mapping_use_latest(OSyncEngine *engine, OSyncMappingEngine *mapping_engine, OSyncError **error)
{
  OSyncEngineCommand *cmd = NULL;
  osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, mapping_engine, error);
	
  cmd = osync_try_malloc0(sizeof(OSyncEngineCommand), error);
  if (!cmd) {
    osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
    return FALSE;
  }
	
  cmd->cmd = OSYNC_ENGINE_COMMAND_SOLVE;
  cmd->mapping_engine = mapping_engine;
  cmd->solve_type = OSYNC_ENGINE_SOLVE_USE_LATEST;
	
  g_async_queue_push(engine->command_queue, cmd);
	
  osync_trace(TRACE_EXIT, "%s", __func__);
  return TRUE;
}

/*! @brief This will create a new engine for the given group
 * 
 * This will create a new engine for the given group
 * 
 * @param group A pointer to the group, for which you want to create a new engine
 * @param error A pointer to a error struct
 * @returns Pointer to a newly allocated OSyncEngine on success, NULL otherwise
 * 
 */
OSyncEngine *osync_engine_new(OSyncGroup *group, OSyncError **error)
{
  OSyncEngine *engine = NULL;
  OSyncEngine **engineptr = NULL;
  char *enginesdir = NULL;

  osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, group, error);
  g_assert(group);
	
  engine = osync_try_malloc0(sizeof(OSyncEngine), error);
  if (!engine)
    goto error;
  engine->ref_count = 1;

  if (!g_thread_supported ())
    g_thread_init (NULL);
	
  engine->internalFormats = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
  engine->internalSchemas = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
  engine->converterPathes = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, _osync_engine_converter_path_unref);
	
  engine->context = g_main_context_new();
  engine->thread = osync_thread_new(engine->context, error);
  if (!engine->thread)
    goto error_free_engine;
	
  engine->group = group;
  osync_group_ref(group);

  engine->command_queue = g_async_queue_new();

  if (!osync_group_get_configdir(group)) {
    osync_trace(TRACE_INTERNAL, "No config dir found. Making stateless sync");
  } else {
    char *filename = g_strdup_printf("%s%carchive.db", osync_group_get_configdir(group), G_DIR_SEPARATOR);
    engine->archive = osync_archive_new(filename, error);
    g_free(filename);
    if (!engine->archive)
      goto error_free_engine;
  }
	
  /* Now we attach a queue to the engine which handles our commands */
  engine->command_functions = g_malloc0(sizeof(GSourceFuncs));
  engine->command_functions->prepare = _command_prepare;
  engine->command_functions->check = _command_check;
  engine->command_functions->dispatch = _command_dispatch;
  engine->command_functions->finalize = NULL;

  engine->command_source = g_source_new(engine->command_functions, sizeof(GSource) + sizeof(OSyncEngine *));

  /* Overwriting pointer of callback_data of GSource (->command_source) with engine 
     FIXME: Make it work without such dirty hacks to inject pointers to a private struct */ 
  engineptr = (OSyncEngine **)(engine->command_source + 1); 
  *engineptr = engine;

  g_source_set_callback(engine->command_source, NULL, engine, NULL);
  g_source_attach(engine->command_source, engine->context);
  g_main_context_ref(engine->context);

  enginesdir = g_strdup_printf("%s%cengines", osync_group_get_configdir(group), G_DIR_SEPARATOR);
  engine->engine_path = g_strdup_printf("%s%cenginepipe", enginesdir, G_DIR_SEPARATOR);
	
  if (g_mkdir_with_parents(enginesdir, 0755) < 0) {
    osync_error_set(error, OSYNC_ERROR_GENERIC, "Couldn't create engines directory: %s", g_strerror(errno));
    g_free(enginesdir);
    goto error_free_engine;
  }
  g_free(enginesdir);
	
  engine->syncing_mutex = g_mutex_new();
  engine->syncing = g_cond_new();
	
  engine->started_mutex = g_mutex_new();
  engine->started = g_cond_new();
	
  osync_trace(TRACE_EXIT, "%s: %p", __func__, engine);
  return engine;

 error_free_engine:
  osync_engine_unref(engine);
 error:
  osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
  return NULL;
}

OSyncEngine *osync_engine_ref(OSyncEngine *engine)
{
  osync_assert(engine);
	
  g_atomic_int_inc(&(engine->ref_count));

  return engine;
}

void osync_engine_unref(OSyncEngine *engine)
{
  osync_assert(engine);
		
  if (g_atomic_int_dec_and_test(&(engine->ref_count))) {
    osync_trace(TRACE_ENTRY, "%s(%p)", __func__, engine);

    while (engine->object_engines) {
      OSyncObjEngine *objengine = engine->object_engines->data;
      osync_obj_engine_unref(objengine);
      engine->object_engines = g_list_remove(engine->object_engines, engine->object_engines->data);
    }

    if (engine->internalFormats)
      g_hash_table_destroy(engine->internalFormats);

    if (engine->converterPathes)
      g_hash_table_destroy(engine->converterPathes);
		
    if (engine->group)
      osync_group_unref(engine->group);
		
    if (engine->engine_path)
      g_free(engine->engine_path);
			
    if (engine->plugin_dir)
      g_free(engine->plugin_dir);
			
    if (engine->format_dir)
      g_free(engine->format_dir);
		
    if (engine->thread)
      osync_thread_free(engine->thread);
			
    if (engine->context)
      g_main_context_unref(engine->context);
			
    if (engine->syncing)
      g_cond_free(engine->syncing);
			
    if (engine->syncing_mutex)
      g_mutex_free(engine->syncing_mutex);
			
    if (engine->started)
      g_cond_free(engine->started);
			
    if (engine->started_mutex)
      g_mutex_free(engine->started_mutex);
		
    if (engine->command_queue)
      g_async_queue_unref(engine->command_queue);
	
    if (engine->command_source)
      g_source_unref(engine->command_source);
	
    if (engine->command_functions)
      g_free(engine->command_functions);
	
    if (engine->archive)
      osync_archive_unref(engine->archive);
		
    if (engine->error)
      osync_error_unref(&(engine->error));

    if (engine->internalSchemas)
      g_hash_table_destroy(engine->internalSchemas);

#ifdef OPENSYNC_UNITTESTS
    if (engine->schema_dir)
      g_free(engine->schema_dir);
#endif /* OPENSYNC_UNITTESTS */
		
    g_free(engine);
    osync_trace(TRACE_EXIT, "%s", __func__);
  }
}

void osync_engine_set_plugindir(OSyncEngine *engine, const char *dir)
{
  osync_assert(engine);
  if (engine->plugin_dir)
    g_free(engine->plugin_dir);
  engine->plugin_dir = g_strdup(dir);
}

OSyncGroup *osync_engine_get_group(OSyncEngine *engine)
{
  osync_assert(engine);
  return engine->group;
}

OSyncArchive *osync_engine_get_archive(OSyncEngine *engine)
{
  osync_assert(engine);
  return engine->archive;
}

void osync_engine_set_formatdir(OSyncEngine *engine, const char *dir)
{
  osync_assert(engine);
  if (engine->format_dir)
    g_free(engine->format_dir);
  engine->format_dir = g_strdup(dir);
}

static osync_bool _osync_engine_start(OSyncEngine *engine, OSyncError **error)
{
  osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, engine, error);
	
  /* For testing purpose, it's possible to preload a instrumented plugin_env */
  if (!engine->pluginenv) {
    engine->pluginenv = osync_plugin_env_new(error);
    if (!engine->pluginenv)
      goto error;
		
    if (!osync_plugin_env_load(engine->pluginenv, engine->plugin_dir, error))
      goto error;
  }
	
  osync_thread_start(engine->thread);

  osync_engine_ref(engine);
	
  osync_trace(TRACE_EXIT, "%s", __func__);
  return TRUE;
	
 error:
  osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
  return FALSE;
}

static void _osync_engine_stop(OSyncEngine *engine)
{
  osync_trace(TRACE_ENTRY, "%s(%p)", __func__, engine);
	
  if (engine->thread)
    osync_thread_stop(engine->thread);

  osync_engine_unref(engine);
	
  osync_trace(TRACE_EXIT, "%s", __func__);
}

static osync_bool _osync_engine_finalize_member(OSyncEngine *engine, OSyncClientProxy *proxy, OSyncError **error)
{
  unsigned int i = 2000;
  osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, engine, proxy, error);
		
  engine->busy = TRUE;
	
  if (!osync_client_proxy_finalize(proxy, _finalize_callback, engine, error))
    goto error;
	
  //FIXME
  while (engine->busy && i > 0) { g_usleep(1000); g_main_context_iteration(engine->context, FALSE); i--; }
  osync_trace(TRACE_INTERNAL, "Done waiting");
	
  if (!osync_client_proxy_shutdown(proxy, error))
    goto error;
	
  engine->proxies = g_list_remove(engine->proxies, proxy);
	
  osync_client_proxy_unref(proxy);
	
  osync_trace(TRACE_EXIT, "%s", __func__);
  return TRUE;
	
 error:
  osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
  return FALSE;
}

static OSyncClientProxy *_osync_engine_initialize_member(OSyncEngine *engine, OSyncMember *member, OSyncError **error)
{
  OSyncPluginConfig *config = NULL;
  OSyncPlugin *plugin = NULL;
  OSyncClientProxy *proxy = NULL;
	
  osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, engine, member, error);

  plugin = osync_plugin_env_find_plugin(engine->pluginenv, osync_member_get_pluginname(member));
  if (!plugin) {
    osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to find plugin %s", osync_member_get_pluginname(member));
    goto error;
  }
		
  /* If we don't have a config we have to ask the plugin if it needs a config */
  if (!osync_member_has_config(member)) {

    switch (osync_plugin_get_config_type(plugin)) {
    case OSYNC_PLUGIN_NO_CONFIGURATION:
      break;
    case OSYNC_PLUGIN_OPTIONAL_CONFIGURATION:
      config = osync_member_get_config_or_default(member, error);
      if (!config)
        goto error;
      break;
    case OSYNC_PLUGIN_NEEDS_CONFIGURATION:
      config = osync_member_get_config(member, error);
      if (!config)
        goto error;
      break;
    }
  } else {
    config = osync_member_get_config(member, error);
    if (!config)
      goto error;
  }
	
  proxy = osync_client_proxy_new(engine->formatenv, member, error);
  if (!proxy)
    goto error;
		
  osync_client_proxy_set_context(proxy, engine->context);
  osync_client_proxy_set_change_callback(proxy, _osync_engine_receive_change, engine);

  if (!osync_client_proxy_spawn(proxy, osync_plugin_get_start_type(plugin), osync_member_get_configdir(member), error))
    goto error_free_proxy;
	
  engine->busy = TRUE;
	
  if (!osync_client_proxy_initialize(proxy, _finalize_callback, engine, engine->format_dir, engine->plugin_dir, osync_member_get_pluginname(member), osync_group_get_name(engine->group), osync_member_get_configdir(member), config, error))
    goto error_shutdown;
	
  //FIXME
  while (engine->busy) { g_usleep(100); }
	
  engine->proxies = g_list_append(engine->proxies, proxy);
	
  if (engine->error) {
    _osync_engine_finalize_member(engine, proxy, NULL);
    osync_error_set_from_error(error, &(engine->error));
    osync_error_unref(&(engine->error));
    engine->error = NULL;
    osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
    return NULL;
  }
	
  osync_trace(TRACE_EXIT, "%s", __func__);
  return proxy;
	
 error_shutdown:
  osync_client_proxy_shutdown(proxy, NULL);
 error_free_proxy:
  osync_client_proxy_unref(proxy);
 error:
  osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
  return NULL;
}

static osync_bool _osync_engine_generate_connected_event(OSyncEngine *engine)
{
  OSyncError *locerror = NULL;

  if (osync_bitcount(engine->proxy_errors | engine->proxy_connects) != g_list_length(engine->proxies))
    return FALSE;
	
  if (osync_bitcount(engine->obj_errors | engine->obj_connects) == g_list_length(engine->object_engines)) {
    if (osync_bitcount(engine->obj_errors) == g_list_length(engine->object_engines)) {
      osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "No objtypes left without error. Aborting");
      osync_trace(TRACE_ERROR, "%s", osync_error_print(&locerror));
      osync_engine_set_error(engine, locerror);
      osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_ERROR, locerror);
      osync_engine_event(engine, OSYNC_ENGINE_EVENT_ERROR);
      osync_error_unref(&locerror);
    } else if (osync_bitcount(engine->proxy_errors) || osync_bitcount(engine->obj_errors)) {
      osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "At least one object engine failed while connecting. Aborting");
      osync_trace(TRACE_ERROR, "%s", osync_error_print(&locerror));
      osync_engine_set_error(engine, locerror);
      osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_ERROR, locerror);
      osync_engine_event(engine, OSYNC_ENGINE_EVENT_ERROR);
      osync_error_unref(&locerror);
    } else {
      osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_CONNECTED, NULL);
      osync_engine_event(engine, OSYNC_ENGINE_EVENT_CONNECTED);
    }

    return TRUE;
  }
	
  return FALSE;
}

osync_bool osync_engine_check_get_changes(OSyncEngine *engine)
{
  if (osync_bitcount(engine->proxy_errors | engine->proxy_get_changes) != g_list_length(engine->proxies)) {
    osync_trace(TRACE_INTERNAL, "Not yet. main sinks still need to read: %i", osync_bitcount(engine->proxy_errors | engine->proxy_get_changes), g_list_length(engine->proxies));
    return FALSE;
  }
	
  if (osync_bitcount(engine->obj_errors | engine->obj_get_changes) == g_list_length(engine->object_engines))
    return TRUE;
		
  osync_trace(TRACE_INTERNAL, "Not yet. Obj Engines still need to read: %i", osync_bitcount(engine->obj_errors | engine->obj_get_changes));
  return FALSE;
}

static void _osync_engine_generate_get_changes_event(OSyncEngine *engine)
{
  if (!osync_engine_check_get_changes(engine))
    return;
		
  if (osync_bitcount(engine->obj_errors)) {
    OSyncError *locerror = NULL;
    osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "At least one object engine failed while getting changes. Aborting");
    osync_trace(TRACE_ERROR, "%s", osync_error_print(&locerror));
    osync_engine_set_error(engine, locerror);
    osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_ERROR, locerror);
    osync_engine_event(engine, OSYNC_ENGINE_EVENT_ERROR);
  } else {
    osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_READ, NULL);
    osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_END_CONFLICTS, NULL);
		
    osync_engine_event(engine, OSYNC_ENGINE_EVENT_READ);
  }
}

static void _osync_engine_generate_written_event(OSyncEngine *engine)
{
  if (osync_bitcount(engine->proxy_errors | engine->proxy_written) != g_list_length(engine->proxies))
    return;
	
  if (osync_bitcount(engine->obj_errors | engine->obj_written) == g_list_length(engine->object_engines)) {
    if (osync_bitcount(engine->obj_errors)) {
      OSyncError *locerror = NULL;
      osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "At least one object engine failed while writting changes. Aborting");
      osync_trace(TRACE_ERROR, "%s", osync_error_print(&locerror));
      osync_engine_set_error(engine, locerror);
      osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_ERROR, locerror);
      osync_engine_event(engine, OSYNC_ENGINE_EVENT_ERROR);
      //osync_error_unref(&locerror);
    } else {
      osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_WRITTEN, NULL);
      osync_engine_event(engine, OSYNC_ENGINE_EVENT_WRITTEN);
    }
  } else
    osync_trace(TRACE_INTERNAL, "Not yet: %i", osync_bitcount(engine->obj_errors | engine->obj_written));

}

static void _osync_engine_generate_sync_done_event(OSyncEngine *engine)
{
  if (osync_bitcount(engine->proxy_errors | engine->proxy_sync_done) != g_list_length(engine->proxies))
    return;
	
  if (osync_bitcount(engine->obj_errors | engine->obj_sync_done) == g_list_length(engine->object_engines)) {
    if (osync_bitcount(engine->obj_errors)) {
      OSyncError *locerror = NULL;
      osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "At least one object engine failed within sync_done. Aborting");
      osync_engine_set_error(engine, locerror);
      osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_ERROR, locerror);
      osync_engine_event(engine, OSYNC_ENGINE_EVENT_ERROR);
      //osync_error_unref(&locerror);
    } else {
      osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_SYNC_DONE, NULL);
      osync_engine_event(engine, OSYNC_ENGINE_EVENT_SYNC_DONE);
    }
  } else
    osync_trace(TRACE_INTERNAL, "Not yet: %i", osync_bitcount(engine->obj_errors | engine->obj_sync_done));
}

static osync_bool _osync_engine_generate_disconnected_event(OSyncEngine *engine)
{
  if (osync_bitcount(engine->proxy_errors | engine->proxy_disconnects) != g_list_length(engine->proxies))
    return FALSE;
	
  if (osync_bitcount(engine->obj_errors | engine->obj_disconnects) == g_list_length(engine->object_engines)) {

    /* Error handling in this case is quite special. We have to call OSYNC_ENGINE_EVENT_DISCONNECTED,
       even on errors. Since OSYNC_ENGINE_EVENT_ERROR would emit this DISCONNECTED event again - deadlock! */
    if (!osync_bitcount(engine->obj_errors))
      osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_DISCONNECTED, NULL);

    osync_engine_event(engine, OSYNC_ENGINE_EVENT_DISCONNECTED);
    return TRUE;
  }
	
  osync_trace(TRACE_INTERNAL, "Not yet: %i", osync_bitcount(engine->obj_errors | engine->obj_disconnects));
  return FALSE;
}

static void _osync_engine_connect_callback(OSyncClientProxy *proxy, void *userdata, osync_bool slowsync, OSyncError *error)
{
  GList *o = NULL;
  OSyncEngine *engine = NULL;
  int position = 0;

  osync_trace(TRACE_ENTRY, "%s(%p, %p, %i, %p)", __func__, proxy, userdata, slowsync, error);

  engine = userdata;
  position = _osync_engine_get_proxy_position(engine, proxy);
	
  if (error) {
    osync_engine_set_error(engine, error);
    engine->proxy_errors = engine->proxy_errors | (0x1 << position);
    osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_ERROR, NULL, error);
  } else {
    engine->proxy_connects = engine->proxy_connects | (0x1 << position);
    osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_CONNECTED, NULL, NULL);
  }

  /* If MainSink request a SlowSync, flag all objengines with SlowSync */
  if (slowsync) {
    for (o = engine->object_engines; o; o = o->next) {
      OSyncObjEngine *objengine = o->data;
      osync_obj_engine_set_slowsync(objengine, TRUE);
    }
  }
	
  _osync_engine_generate_connected_event(engine);
	
  osync_trace(TRACE_EXIT, "%s", __func__);
}

static void _osync_engine_disconnect_callback(OSyncClientProxy *proxy, void *userdata, OSyncError *error)
{
  OSyncEngine *engine = userdata;
  int position = 0;
  osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, proxy, userdata, error);
	
  position = _osync_engine_get_proxy_position(engine, proxy);
	
  if (error) {
    osync_engine_set_error(engine, error);
    engine->proxy_errors = engine->proxy_errors | (0x1 << position);
    osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_ERROR, NULL, error);
  } else {
    engine->proxy_disconnects = engine->proxy_disconnects | (0x1 << position);
    osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_DISCONNECTED, NULL, NULL);
  }
	
  _osync_engine_generate_disconnected_event(engine);
	
  osync_trace(TRACE_EXIT, "%s", __func__);
}

static void _osync_engine_get_changes_callback(OSyncClientProxy *proxy, void *userdata, OSyncError *error)
{
  OSyncEngine *engine = userdata;
  int position = 0;
  osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, proxy, userdata, error);
	
  position = _osync_engine_get_proxy_position(engine, proxy);
	
  if (error) {
    osync_engine_set_error(engine, error);
    engine->proxy_errors = engine->proxy_errors | (0x1 << position);
    osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_ERROR, NULL, error);
  } else {
    engine->proxy_get_changes = engine->proxy_get_changes | (0x1 << position);
    osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_READ, NULL, NULL);
  }
	
  _osync_engine_generate_get_changes_event(engine);
	
  osync_trace(TRACE_EXIT, "%s", __func__);
}

static void _osync_engine_written_callback(OSyncClientProxy *proxy, void *userdata, OSyncError *error)
{
  OSyncEngine *engine = userdata;
  int position = 0;
  osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, proxy, userdata, error);
	
  position = _osync_engine_get_proxy_position(engine, proxy);
	
  if (error) {
    osync_engine_set_error(engine, error);
    engine->proxy_errors = engine->proxy_errors | (0x1 << position);
    osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_ERROR, NULL, error);
  } else {
    engine->proxy_written = engine->proxy_written | (0x1 << position);
    osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_WRITTEN, NULL, NULL);
  }
	
  _osync_engine_generate_written_event(engine);
	
  osync_trace(TRACE_EXIT, "%s", __func__);
}

static void _osync_engine_sync_done_callback(OSyncClientProxy *proxy, void *userdata, OSyncError *error)
{
  OSyncEngine *engine = userdata;
  int position = 0;
  osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, proxy, userdata, error);
	
  position = _osync_engine_get_proxy_position(engine, proxy);
	
  if (error) {
    osync_engine_set_error(engine, error);
    engine->proxy_errors = engine->proxy_errors | (0x1 << position);
    osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_ERROR, NULL, error);
  } else {
    engine->proxy_sync_done = engine->proxy_sync_done | (0x1 << position);
    osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_SYNC_DONE, NULL, NULL);
  }
	
  _osync_engine_generate_sync_done_event(engine);
	
  osync_trace(TRACE_EXIT, "%s", __func__);
}

static void _osync_engine_get_objengine_error(OSyncEngine *engine, OSyncObjEngine *objengine, int position, OSyncError *error)
{
  engine->obj_errors = engine->obj_errors | (0x1 << position);
  osync_engine_set_error(engine, error);
}

static void _osync_engine_get_objengine_event(OSyncEngine *engine, OSyncObjEngine *objengine, int position, OSyncEngineEvent event)
{
  switch (event) {
  case OSYNC_ENGINE_EVENT_CONNECTED:
    engine->obj_connects = engine->obj_connects | (0x1 << position);
    break;
  case OSYNC_ENGINE_EVENT_ERROR:
    /* ObjEngine don't emit this signal. To determine which actual event fail,
       the ObjEngine emits regular event type and pass an OSyncError object.
       See _osync_engine_generate_event() and _osync_engine_get_obj_engine_error(). */
    break;
  case OSYNC_ENGINE_EVENT_READ:
    engine->obj_get_changes = engine->obj_get_changes | (0x1 << position);
    break;
  case OSYNC_ENGINE_EVENT_WRITTEN:
    engine->obj_written = engine->obj_written | (0x1 << position);
    break;
  case OSYNC_ENGINE_EVENT_SYNC_DONE:
    engine->obj_sync_done = engine->obj_sync_done | (0x1 << position);
    break;
  case OSYNC_ENGINE_EVENT_DISCONNECTED:
    engine->obj_disconnects = engine->obj_disconnects | (0x1 << position);
    break;
  case OSYNC_ENGINE_EVENT_SUCCESSFUL:
  case OSYNC_ENGINE_EVENT_END_CONFLICTS:
  case OSYNC_ENGINE_EVENT_PREV_UNCLEAN:
    break;
  }
}

static void _osync_engine_generate_event(OSyncEngine *engine, OSyncEngineEvent event)
{
  switch (event) {
  case OSYNC_ENGINE_EVENT_CONNECTED:
    _osync_engine_generate_connected_event(engine);
    break;
  case OSYNC_ENGINE_EVENT_READ:
    _osync_engine_generate_get_changes_event(engine);
    break;
  case OSYNC_ENGINE_EVENT_WRITTEN:
    _osync_engine_generate_written_event(engine);
    break;
  case OSYNC_ENGINE_EVENT_SYNC_DONE:
    _osync_engine_generate_sync_done_event(engine);
    break;
  case OSYNC_ENGINE_EVENT_DISCONNECTED:
    _osync_engine_generate_disconnected_event(engine);
    break;
  case OSYNC_ENGINE_EVENT_ERROR:
  case OSYNC_ENGINE_EVENT_SUCCESSFUL:
  case OSYNC_ENGINE_EVENT_END_CONFLICTS:
  case OSYNC_ENGINE_EVENT_PREV_UNCLEAN:
    break;
  }
}

static void _osync_engine_event_callback(OSyncObjEngine *objengine, OSyncEngineEvent event, OSyncError *error, void *userdata)
{
  OSyncEngine *engine = userdata;
  int position = 0;
  osync_trace(TRACE_ENTRY, "%s(%p, %i, %p, %p)", __func__, objengine, event, error, userdata);

  position = _osync_engine_get_objengine_position(engine, objengine);
	
  if (error)
    _osync_engine_get_objengine_error(engine, objengine, position, error);
  else
    _osync_engine_get_objengine_event(engine, objengine, position, event);

  _osync_engine_generate_event(engine, event);

  osync_trace(TRACE_EXIT, "%s", __func__);
}

static void _osync_engine_discover_callback(OSyncClientProxy *proxy, void *userdata, OSyncError *error)
{
  OSyncEngine *engine = userdata;
  osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, proxy, userdata, error);
	
  if (error) {
    osync_engine_set_error(engine, error);
    osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_ERROR, NULL, error);
  } else {
    osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_DISCOVERED, NULL, NULL);
  }
	
  g_mutex_lock(engine->syncing_mutex);
  g_cond_signal(engine->syncing);
  g_mutex_unlock(engine->syncing_mutex);
			
  osync_trace(TRACE_EXIT, "%s", __func__);
}


/*! @brief Initialize format environment and "intenral formats" for the engine 
 *
 * FIXME: Drop internal schema initilization once xmlformat plugin does this in the fomrat-init function. 
 * 
 * @param engine A pointer to the engine, which to initialize the formatenv.
 * @param error A pointer to a error struct
 * @returns TRUE on success, FALSE otherwise.
 * 
 */
static osync_bool _osync_engine_initialize_formats(OSyncEngine *engine, OSyncError **error)
{
  engine->formatenv = osync_format_env_new(error);
  if (!engine->formatenv)
    goto error;
	
  if (!osync_format_env_load_plugins(engine->formatenv, engine->format_dir, error))
    goto error_free;
	
  /* XXX The internal formats XXX */
  _osync_engine_set_internal_format(engine, "contact", osync_format_env_find_objformat(engine->formatenv, "xmlformat-contact"));
  _osync_engine_set_internal_format(engine, "event", osync_format_env_find_objformat(engine->formatenv, "xmlformat-event"));
  _osync_engine_set_internal_format(engine, "todo", osync_format_env_find_objformat(engine->formatenv, "xmlformat-todo"));
  _osync_engine_set_internal_format(engine, "note", osync_format_env_find_objformat(engine->formatenv, "xmlformat-note"));
  /* init schemas */
  _osync_engine_set_internal_schema(engine, "contact", error);
  _osync_engine_set_internal_schema(engine, "event", error);
  _osync_engine_set_internal_schema(engine, "todo", error);
  _osync_engine_set_internal_schema(engine, "note", error);
	
  return TRUE;

 error_free:
  osync_format_env_free(engine->formatenv);
  engine->formatenv = NULL;

 error:
  return FALSE;
}


osync_bool osync_engine_initialize(OSyncEngine *engine, OSyncError **error)
{
  osync_bool prev_sync_unclean = FALSE;
  OSyncGroup *group = NULL;
  int i = 0, num = 0;
  osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, engine, error);

  if (engine->state != OSYNC_ENGINE_STATE_UNINITIALIZED) {
    osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "This engine was not uninitialized: %i", engine->state);
    goto error;
  }
	
  group = engine->group;

  if (osync_group_num_members(group) < 2) {
    //Not enough members!
    osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "You only configured %i members, but at least 2 are needed", osync_group_num_members(group));
    goto error;
  }
	
  if (osync_group_num_objtypes(engine->group) == 0) {
    osync_error_set(error, OSYNC_ERROR_GENERIC, "No synchronizable objtype");
    goto error;
  }
	
  switch (osync_group_lock(group)) {
  case OSYNC_LOCKED:
    osync_error_set(error, OSYNC_ERROR_LOCKED, "Group is locked");
    goto error;
  case OSYNC_LOCK_STALE:
    osync_trace(TRACE_INTERNAL, "Detected stale lock file. Slow-syncing");
    osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_PREV_UNCLEAN, NULL);
    prev_sync_unclean = TRUE;
    break;
  case OSYNC_LOCK_OK:
    break;
  }

  if (!_osync_engine_initialize_formats(engine, error))
    goto error;
	
  osync_trace(TRACE_INTERNAL, "Running the main loop");
  if (!_osync_engine_start(engine, error))
    goto error_finalize;
		
  osync_trace(TRACE_INTERNAL, "Spawning clients");
  for (i = 0; i < osync_group_num_members(group); i++) {
    OSyncMember *member = osync_group_nth_member(group, i);
    if (!_osync_engine_initialize_member(engine, member, error))
      goto error_finalize;
  }
	
  /* Lets see which objtypes are synchronizable in this group */
  num = osync_group_num_objtypes(engine->group);
  if (num == 0) {
    osync_error_set(error, OSYNC_ERROR_GENERIC, "No synchronizable objtype");
    goto error;
  }

  for (i = 0; i < num; i++) {
    const char *objtype = osync_group_nth_objtype(engine->group, i);
    OSyncObjEngine *objengine = NULL;

    /* Respect if the object type is disabled */
    if (!osync_group_objtype_enabled(engine->group, objtype))
      continue;

    objengine = osync_obj_engine_new(engine, objtype, engine->formatenv, error);
    if (!objengine)
      goto error;

    osync_obj_engine_set_callback(objengine, _osync_engine_event_callback, engine);
    engine->object_engines = g_list_append(engine->object_engines, objengine);

    /* If previous sync was unclean, then trigger SlowSync for all ObjEngines */
    if (prev_sync_unclean)
      osync_obj_engine_set_slowsync(objengine, TRUE);
  }

  engine->state = OSYNC_ENGINE_STATE_INITIALIZED;

  osync_trace(TRACE_EXIT, "%s", __func__);
  return TRUE;
	
 error_finalize:
  osync_engine_finalize(engine, NULL);
  osync_group_unlock(engine->group);
 error:
  osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
  return FALSE;
}

osync_bool osync_engine_finalize(OSyncEngine *engine, OSyncError **error)
{
  OSyncClientProxy *proxy = NULL;
  osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, engine, error);
	
  if (engine->state != OSYNC_ENGINE_STATE_INITIALIZED) {
    osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "This engine was not in state initialized: %i", engine->state);
    goto error;
  }
	
  engine->state = OSYNC_ENGINE_STATE_UNINITIALIZED;

  while (engine->object_engines) {
    OSyncObjEngine *objengine = engine->object_engines->data;
    osync_obj_engine_unref(objengine);
    engine->object_engines = g_list_remove(engine->object_engines, engine->object_engines->data);
  }
	
  while (engine->proxies) {
    proxy = engine->proxies->data;
    if (!_osync_engine_finalize_member(engine, proxy, error))
      goto error;
  }
	
  _osync_engine_stop(engine);
	
  if (engine->formatenv) {
    osync_format_env_free(engine->formatenv);
    engine->formatenv = NULL;
  }
	
  if (engine->pluginenv) {
    osync_plugin_env_unref(engine->pluginenv);
    engine->pluginenv = NULL;
  }
	
  /* free internal schemas */
  _osync_engine_finalize_internal_schemas(engine);

  if (!engine->error)
    osync_group_unlock(engine->group);

  osync_error_unref(&(engine->error));
	
  osync_trace(TRACE_EXIT, "%s", __func__);
  return TRUE;
	
 error:
  osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
  return FALSE;
}

void osync_engine_command(OSyncEngine *engine, OSyncEngineCommand *command)
{
  GList *o = NULL;
  GList *p = NULL;
  OSyncError *locerror = NULL;
  OSyncClientProxy *proxy = NULL;
			
  osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, engine, command);
  osync_assert(engine);
	
  switch (command->cmd) {
  case OSYNC_ENGINE_COMMAND_CONNECT:

    /* We first tell all object engines to connect */
    for (o = engine->object_engines; o; o = o->next) {
      OSyncObjEngine *objengine = o->data;

      if (!osync_obj_engine_initialize(objengine, &locerror))
        goto error;

      if (!osync_obj_engine_command(objengine, OSYNC_ENGINE_COMMAND_CONNECT, &locerror))
        goto error;
    }
			
    /* Then we connect the main sinks */
    for (o = engine->proxies; o; o = o->next) {
      OSyncClientProxy *proxy = o->data;
      if (!osync_client_proxy_connect(proxy, _osync_engine_connect_callback, engine, NULL, FALSE, &locerror))
        goto error;
    }
    break;
  case OSYNC_ENGINE_COMMAND_READ:
  case OSYNC_ENGINE_COMMAND_WRITE:
  case OSYNC_ENGINE_COMMAND_DISCONNECT:
  case OSYNC_ENGINE_COMMAND_SYNC_DONE:
    break;
  case OSYNC_ENGINE_COMMAND_SOLVE:
    switch (command->solve_type) {
    case OSYNC_ENGINE_SOLVE_CHOOSE:
      if (!osync_mapping_engine_solve(command->mapping_engine, command->master, &locerror))
        goto error;
      break;
    case OSYNC_ENGINE_SOLVE_DUPLICATE:
      if (!osync_mapping_engine_duplicate(command->mapping_engine, &locerror))
        goto error;
      break;
    case OSYNC_ENGINE_SOLVE_IGNORE:
      if (!osync_mapping_engine_ignore(command->mapping_engine, &locerror))
        goto error;
      break;
    case OSYNC_ENGINE_SOLVE_USE_LATEST:
      if (!osync_mapping_engine_use_latest(command->mapping_engine, &locerror))
        goto error;
      break;
    }
    break;
  case OSYNC_ENGINE_COMMAND_DISCOVER:
    for (p = engine->proxies; p; p = p->next) {
      proxy = p->data;
      if (osync_client_proxy_get_member(proxy) == command->member)
        break;
      proxy = NULL;
    }
			
    if (!proxy) {
      osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Unable to find member");
      goto error;
    }
		
    if (!osync_client_proxy_discover(proxy, _osync_engine_discover_callback, engine, &locerror))
      goto error;
		
    break;
  case OSYNC_ENGINE_COMMAND_ABORT:
    /* For nwo Command Aborting is just trigger ENGINE_EVENT_ERROR.
       Which is basically just calling the disconnect functions and not setting
       the synchrnoization as a successful one. */
    osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Synchronization got aborted by user!");

    osync_engine_set_error(engine, locerror);
    osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_ERROR, locerror);

    osync_error_unref(&locerror);

    osync_engine_event(engine, OSYNC_ENGINE_EVENT_ERROR);
    break;

  }
	
  osync_trace(TRACE_EXIT, "%s", __func__);
  return;
	
 error:
  osync_engine_set_error(engine, locerror);

  g_mutex_lock(engine->syncing_mutex);
  g_cond_signal(engine->syncing);
  g_mutex_unlock(engine->syncing_mutex);

  osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&locerror));
}

void osync_engine_event(OSyncEngine *engine, OSyncEngineEvent event)
{
  GList *o = NULL;
  OSyncError *locerror = NULL;
	
  osync_trace(TRACE_ENTRY, "%s(%p, %i)", __func__, engine, event);
  osync_assert(engine);
	

  switch (event) {
  case OSYNC_ENGINE_EVENT_CONNECTED:
    /* Now that we are connected, we read the changes */
    for (o = engine->object_engines; o; o = o->next) {
      OSyncObjEngine *objengine = o->data;
      if (!osync_obj_engine_command(objengine, OSYNC_ENGINE_COMMAND_READ, &locerror))
        goto error;
    }
			
    /* Now we read the main sink */
    for (o = engine->proxies; o; o = o->next) {
      OSyncClientProxy *proxy = o->data;
      if (!osync_client_proxy_get_changes(proxy, _osync_engine_get_changes_callback, engine, NULL, FALSE, &locerror))
        goto error;
    }

    break;
  case OSYNC_ENGINE_EVENT_READ:
    /* Now that we are connected, we write the changes */
    for (o = engine->object_engines; o; o = o->next) {
      OSyncObjEngine *objengine = o->data;
      if (!osync_obj_engine_command(objengine, OSYNC_ENGINE_COMMAND_WRITE, &locerror))
        goto error;
    }

    /* Now we write the main sink */
    for (o = engine->proxies; o; o = o->next) {
      OSyncClientProxy *proxy = o->data;
      if (!osync_client_proxy_committed_all(proxy, _osync_engine_written_callback, engine, NULL, &locerror))
        goto error;
    }

    break;
  case OSYNC_ENGINE_EVENT_WRITTEN:
    /* Lets call sync done */
    for (o = engine->object_engines; o; o = o->next) {
      OSyncObjEngine *objengine = o->data;
      if (!osync_obj_engine_command(objengine, OSYNC_ENGINE_COMMAND_SYNC_DONE, &locerror))
        goto error;
    }
			
    /* Now we call sync done on the main sink */
    for (o = engine->proxies; o; o = o->next) {
      OSyncClientProxy *proxy = o->data;
      if (!osync_client_proxy_sync_done(proxy, _osync_engine_sync_done_callback, engine, NULL, &locerror))
        goto error;
    }

    break;
  case OSYNC_ENGINE_EVENT_ERROR:
    osync_trace(TRACE_ERROR, "Engine aborting due to an error: %s", osync_error_print(&(engine->error)));
    /* Fall through! - To emit disconnect commands for clean connection termination, in error condition */
  case OSYNC_ENGINE_EVENT_SYNC_DONE:
    /* Lets disconnect */
    for (o = engine->object_engines; o; o = o->next) {
      OSyncObjEngine *objengine = o->data;
      if (!osync_obj_engine_command(objengine, OSYNC_ENGINE_COMMAND_DISCONNECT, &locerror))
        goto error;
    }

    /* Now we disconnect the main sink */
    for (o = engine->proxies; o; o = o->next) {
      OSyncClientProxy *proxy = o->data;

      if (!_osync_engine_is_proxy_connected(engine, proxy)) {
        _osync_engine_disconnect_callback(proxy, engine, NULL);
        continue;
      }

      if (!osync_client_proxy_disconnect(proxy, _osync_engine_disconnect_callback, engine, NULL, &locerror))
        goto error;
    }
			
    if (!engine->error)
      osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_SUCCESSFUL, NULL);

    break;
  case OSYNC_ENGINE_EVENT_DISCONNECTED:

    for (o = engine->object_engines; o; o = o->next) {
      OSyncObjEngine *objengine = o->data;
      osync_obj_engine_finalize(objengine);
    }

    engine->proxy_connects = 0;
    engine->proxy_disconnects = 0;
    engine->proxy_get_changes = 0;
    engine->proxy_written = 0;
    engine->proxy_errors = 0;
    engine->proxy_sync_done = 0;
			
    engine->obj_errors = 0;
    engine->obj_connects = 0;
    engine->obj_disconnects = 0;
    engine->obj_get_changes = 0;
    engine->obj_written = 0;
    engine->obj_sync_done = 0;
			
    g_mutex_lock(engine->syncing_mutex);
    g_cond_signal(engine->syncing);
    g_mutex_unlock(engine->syncing_mutex);
    break;
  case OSYNC_ENGINE_EVENT_SUCCESSFUL:
  case OSYNC_ENGINE_EVENT_END_CONFLICTS:
  case OSYNC_ENGINE_EVENT_PREV_UNCLEAN:
    break;
  }
	
  osync_trace(TRACE_EXIT, "%s", __func__);
  return;
	
 error:
  osync_engine_set_error(engine, locerror);

  g_mutex_lock(engine->syncing_mutex);
  g_cond_signal(engine->syncing);
  g_mutex_unlock(engine->syncing_mutex);

  osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&locerror));
}

/*! @brief Starts to synchronize the given OSyncEngine
 *
 * This function synchronizes a given engine. The Engine has to be created
 * from a OSyncGroup before by using osync_engine_new(). This function will not block
 * 
 * @param engine A pointer to the engine, which will be used to sync
 * @param error A pointer to a error struct
 * @returns TRUE on success, FALSE otherwise. Check the error on FALSE. Note that this just says if the sync has been started successfully, not if the sync itself was successful
 * 
 */
osync_bool osync_engine_synchronize(OSyncEngine *engine, OSyncError **error)
{
  OSyncEngineCommand *cmd = NULL;
  osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, engine, error);
  osync_assert(engine);
	
  if (engine->state != OSYNC_ENGINE_STATE_INITIALIZED) {
    osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "This engine was not in state initialized: %i", engine->state);
    goto error;
  }

  cmd = osync_try_malloc0(sizeof(OSyncEngineCommand), error);
  if (!cmd)
    goto error;
  cmd->cmd = OSYNC_ENGINE_COMMAND_CONNECT;
	
  g_async_queue_push(engine->command_queue, cmd);
	
  osync_trace(TRACE_EXIT, "%s", __func__);
  return TRUE;

 error:
  osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
  return FALSE;
}

/*! @brief This function will synchronize once and block until the sync has finished
 *
 * This can be used to sync a group and wait for the synchronization end. DO NOT USE
 * osync_engine_wait_sync_end for this as this might introduce a race condition.
 * 
 * @param engine A pointer to the engine, which to sync and wait for the sync end
 * @param member A pointer to the member, which to discover
 * @param error A pointer to a error struct
 * @returns TRUE on success, FALSE otherwise.
 * 
 */
osync_bool osync_engine_synchronize_and_block(OSyncEngine *engine, OSyncError **error)
{
  osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, engine, error);
	
  g_mutex_lock(engine->syncing_mutex);
	
  if (!osync_engine_synchronize(engine, error)) {
    g_mutex_unlock(engine->syncing_mutex);
    goto error;
  }

  g_cond_wait(engine->syncing, engine->syncing_mutex);
  g_mutex_unlock(engine->syncing_mutex);
	
  if (engine->error) {
    char *msg = osync_error_print_stack(&(engine->error));
    osync_trace(TRACE_ERROR, "error while synchronizing: %s", msg);
    g_free(msg);
    osync_error_set_from_error(error, &(engine->error));
    goto error;
  }
	
  osync_trace(TRACE_EXIT, "%s", __func__);
  return TRUE;

 error:
  osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
  return FALSE;
}

/*! @brief This function will block until a synchronization has ended
 *
 * This can be used to wait until the synchronization has ended. Note that this function will always
 * block until 1 sync has ended. It can be used before the sync has started, to wait for one auto-sync
 * to end
 * 
 * @param engine A pointer to the engine, for which to wait for the sync end
 * @param error Return location for the error if the sync was not successful
 * @returns TRUE on success, FALSE otherwise.
 */
osync_bool osync_engine_wait_sync_end(OSyncEngine *engine, OSyncError **error)
{
  g_mutex_lock(engine->syncing_mutex);
  g_cond_wait(engine->syncing, engine->syncing_mutex);
  g_mutex_unlock(engine->syncing_mutex);
	
  if (engine->error) {
    osync_error_set_from_error(error, &(engine->error));
    osync_error_unref(&(engine->error));
    engine->error = NULL;
    return FALSE;
  }
  return TRUE;
}

/*! @brief This function will discover the capabilities of a member
 *
 * This function discover a member of a given engine. The Engine has to be created
 * from a OSyncGroup before by using osync_engine_new(). This function will not block
 * The Engine MUST NOT be initialized by osync_engine_initilize(), but MUST finalized with
 * osync_engine_finalize().
 *
 * FIXME: Automatically finalize the engine after discovery of member is finished. This
 *        is needed by the frontend to allow easy use of non-blocking discovery.
 * 
 * @param engine A pointer to the engine, which to discover the member and wait for the discover end
 * @param member A pointer to the member, which to discover
 * @param error A pointer to a error struct
 * @returns TRUE on success, FALSE otherwise.
 * 
 */
osync_bool osync_engine_discover(OSyncEngine *engine, OSyncMember *member, OSyncError **error)
{
  OSyncClientProxy *proxy = NULL;
  OSyncEngineCommand *cmd = NULL;
  osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, engine, member, error);
  osync_assert(engine);
	
  if (engine->state == OSYNC_ENGINE_STATE_INITIALIZED) {
    osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "This engine was in state initialized: %i", engine->state);
    goto error;
  }
	
  if (!_osync_engine_start(engine, error))
    goto error;
	
  engine->state = OSYNC_ENGINE_STATE_INITIALIZED;

  /* Initialize formats before members! 
   * Since we check if the formats claimed by the members are available */
  if (!_osync_engine_initialize_formats(engine, error))
    goto error;
	
  proxy = _osync_engine_initialize_member(engine, member, error);
  if (!proxy)
    goto error;

  cmd = osync_try_malloc0(sizeof(OSyncEngineCommand), error);
  if (!cmd)
    goto error;

  /* Flush all object types of member before discovering.
     Otherwise "old" object types get discovered again. */
  osync_member_flush_objtypes(member);

  cmd->cmd = OSYNC_ENGINE_COMMAND_DISCOVER;
  cmd->member = member;
	
  g_async_queue_push(engine->command_queue, cmd);
	
  osync_trace(TRACE_EXIT, "%s", __func__);
  return TRUE;

 error:
  osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
  return FALSE;
}

/*! @brief This function will discover the member and block until the discovery has finished
 *
 * This can be used to discover a member and wait for the discovery end. 
 * The engine MUST NOT be initialized or finalized.
 * 
 * @param engine A pointer to the engine, which to discover the member and wait for the discover end
 * @param member A pointer to the member, which to discover
 * @param error A pointer to a error struct
 * @returns TRUE on success, FALSE otherwise.
 * 
 */
osync_bool osync_engine_discover_and_block(OSyncEngine *engine, OSyncMember *member, OSyncError **error)
{
  osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, engine, member, error);
	
  g_mutex_lock(engine->syncing_mutex);
	
  if (!osync_engine_discover(engine, member, error)) {
    g_mutex_unlock(engine->syncing_mutex);
    goto error_finalize;
  }
	
  g_cond_wait(engine->syncing, engine->syncing_mutex);
  g_mutex_unlock(engine->syncing_mutex);
	
  if (!osync_engine_finalize(engine, error))
    goto error;
	
  if (engine->error) {
    osync_error_set_from_error(error, &(engine->error));
    osync_error_unref(&(engine->error));
    engine->error = NULL;
    goto error;
  }
	
  osync_trace(TRACE_EXIT, "%s", __func__);
  return TRUE;

 error_finalize:
  osync_engine_finalize(engine, NULL);
 error:
  osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
  return FALSE;
}

int osync_engine_num_proxies(OSyncEngine *engine)
{
  osync_assert(engine);
  return g_list_length(engine->proxies);
}

OSyncClientProxy *osync_engine_nth_proxy(OSyncEngine *engine, int nth)
{
  osync_assert(engine);
  return g_list_nth_data(engine->proxies, nth);
}

OSyncClientProxy *osync_engine_find_proxy(OSyncEngine *engine, OSyncMember *member)
{
  GList *p = NULL;
  OSyncClientProxy *proxy = NULL;
	
  osync_assert(engine);
	
  for (p = engine->proxies; p; p = p->next) {
    proxy = p->data;
    if (osync_client_proxy_get_member(proxy) == member)
      return proxy;
  }
	
  return NULL;
}

int osync_engine_num_objengine(OSyncEngine *engine)
{
  osync_assert(engine);
  return g_list_length(engine->object_engines);
}

OSyncObjEngine *osync_engine_nth_objengine(OSyncEngine *engine, int nth)
{
  osync_assert(engine);
  return g_list_nth_data(engine->object_engines, nth);
}

OSyncObjEngine *osync_engine_find_objengine(OSyncEngine *engine, const char *objtype)
{
  GList *p = NULL;
  OSyncObjEngine *objengine = NULL;
	
  osync_assert(engine);

  for (p = engine->object_engines; p; p = p->next) {
    objengine = p->data;
    if (!strcmp(osync_obj_engine_get_objtype(objengine), objtype))
      return objengine;
  }
	
  return NULL;
}

/*! @brief This will set the conflict handler for the given engine
 * 
 * The conflict handler will be called every time a conflict occurs
 * 
 * @param engine A pointer to the engine, for which to set the callback
 * @param function A pointer to a function which will receive the conflict
 * @param user_data Pointer to some data that will get passed to the status function as the last argument
 * 
 */
void osync_engine_set_conflict_callback(OSyncEngine *engine, osync_conflict_cb callback, void *user_data)
{
  engine->conflict_callback = callback;
  engine->conflict_userdata = user_data;
}

/*! @brief This will set the change status handler for the given engine
 * 
 * The change status handler will be called every time a new change is received, written etc
 * 
 * @param engine A pointer to the engine, for which to set the callback
 * @param function A pointer to a function which will receive the change status
 * @param user_data Pointer to some data that will get passed to the status function as the last argument
 * 
 */
void osync_engine_set_changestatus_callback(OSyncEngine *engine, osync_status_change_cb callback, void *user_data)
{
  engine->changestat_callback = callback;
  engine->changestat_userdata = user_data;
}

/*! @brief This will set the mapping status handler for the given engine
 * 
 * The mapping status handler will be called every time a mapping is updated
 * 
 * @param engine A pointer to the engine, for which to set the callback
 * @param function A pointer to a function which will receive the mapping status
 * @param user_data Pointer to some data that will get passed to the status function as the last argument
 * 
 */
void osync_engine_set_mappingstatus_callback(OSyncEngine *engine, osync_status_mapping_cb callback, void *user_data)
{
  engine->mapstat_callback = callback;
  engine->mapstat_userdata = user_data;
}

/*! @brief This will set the engine status handler for the given engine
 * 
 * The engine status handler will be called every time the engine is updated (started, stopped etc)
 * 
 * @param engine A pointer to the engine, for which to set the callback
 * @param function A pointer to a function which will receive the engine status
 * @param user_data Pointer to some data that will get passed to the status function as the last argument
 * 
 */
void osync_engine_set_enginestatus_callback(OSyncEngine *engine, osync_status_engine_cb callback, void *user_data)
{
  engine->engstat_callback = callback;
  engine->engstat_userdata = user_data;
}

/*! @brief This will set the member status handler for the given engine
 * 
 * The member status handler will be called every time a member is updated (connects, disconnects etc)
 * 
 * @param engine A pointer to the engine, for which to set the callback
 * @param function A pointer to a function which will receive the member status
 * @param user_data Pointer to some data that will get passed to the status function as the last argument
 * 
 */
void osync_engine_set_memberstatus_callback(OSyncEngine *engine, osync_status_member_cb callback, void *user_data)
{
  engine->mebstat_callback = callback;
  engine->mebstat_userdata = user_data;
}

#if 0
/*! @brief This will set the callback handler for a custom message
 * 
 * A custom message can be used to communicate with a plugin directly
 * 
 * @param engine A pointer to the engine, for which to set the callback
 * @param function A pointer to a function which will receive the member status
 * @param user_data A pointer to some user data that the callback function will get passed
 * 
 */
void osync_engine_set_message_callback(OSyncEngine *engine, void *(* function) (OSyncEngine *, OSyncClient *, const char *, void *, void *), void *user_data)
{
  engine->plgmsg_callback = function;
  engine->plgmsg_userdata = user_data;
}
#endif

/*! @brief Aborts running synchronization
 * 
 * This is aborting the current synchronization while flushing the pending
 * commands in the engine command queue and pushing the abort command on this
 * queue. The abort command will send the disconnect command to the client/plugins.
 * This could be also used within a conflict handler function which aborts the
 * synchronization instead of resolving the conflicts.
 *
 * FIXME: Currently aborting of the current synchronization is not yet perfect! It
 *        will not preempt already running commands. For example the batch_commit
 *        will not be preempted and the engine will abort after the batch_commit is done.
 *
 * TODO: Review XMPM Benq patches for abort hander. Is sigaction really sane way
 *       to abort? It's very important that the plugins get called with the disconnect
 *       functions, since plugins/devices rely on clean termination of connections.
 *
 * TODO: Introduce plugin abort function for protocol specific abort implementations
 *       (SyncML?, OBEX-based?, ...?)
 * 
 * @param engine A pointer to the engine with a running synchronization which gets aborted. 
 * @param error A pointer to a error struct
 * @returns TRUE on success, FALSE otherwise.
 * 
 */
osync_bool osync_engine_abort(OSyncEngine *engine, OSyncError **error)
{
  OSyncEngineCommand *pending_command, *cmd;
  osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, engine, error);

  if (engine->state != OSYNC_ENGINE_STATE_INITIALIZED) {
    osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "This engine was not in state initialized: %i", engine->state);
    goto error;
  }

  cmd = osync_try_malloc0(sizeof(OSyncEngineCommand), error);
  if (!cmd)
    goto error;
	
  cmd->cmd = OSYNC_ENGINE_COMMAND_ABORT;
	
  /* Lock the engine command queue ... */
  g_async_queue_lock(engine->command_queue);

  /* ...and flush all pending commands.
     To make sure the abort command will be the next and last command. */
  while ((pending_command = g_async_queue_try_pop_unlocked(engine->command_queue)))
    g_free(pending_command);

  /* Push the abort command on the empty queue. */
  g_async_queue_push_unlocked(engine->command_queue, cmd);

  /* Done. Unlock the command queue again. */
  g_async_queue_unlock(engine->command_queue);
	
  osync_trace(TRACE_EXIT, "%s", __func__);
  return TRUE;

 error:	
  osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
  return FALSE;
}

#ifdef OPENSYNC_UNITTESTS
/** @brief Set the schemadir for schema validation to a custom directory.
 *  This is actually only inteded for UNITTESTS to run tests without 
 *  having OpenSync installed.
 * 
 * @param engine Pointer to engine
 * @param schemadir Custom schemadir path
 * 
 */
void osync_engine_set_schemadir(OSyncEngine *engine, const char *schema_dir)
{
  osync_assert(engine);
  osync_assert(schema_dir);

  if (engine->schema_dir)
    g_free(engine->schema_dir);

  engine->schema_dir = g_strdup(schema_dir); 
}
#endif /* OPENSYNC_UNITTESTS */

