#include <opensync/opensync.h>
#include <glib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>

#include "opensync/opensync_internals.h"

typedef struct PluginProcess {
	OSyncEnv *env;
	OSyncMember *member;
	OSyncQueue *incoming;
	OSyncQueue *outgoing;

	/** Does osync_member_initialized() run successfully? */
	osync_bool is_initialized;
} PluginProcess;

typedef struct context {
	PluginProcess *pp;
	OSyncMessage *message;

	/** The change being commited, for commit_change() */
	OSyncChange *change;

	/** A function that may be used to set method-specific data in the reply,
	 *  such as the UID in the in the commit_change reply
	 */
	osync_bool (*add_reply_data)(OSyncMessage*, struct context*, OSyncError**);
} context;


static osync_bool add_commit_change_reply_data(OSyncMessage *reply, context *ctx, OSyncError **error);
static osync_bool add_connect_reply_data(OSyncMessage *reply, context *ctx, OSyncError **error);
static osync_bool add_get_changedata_reply_data(OSyncMessage *reply, context *ctx, OSyncError **error);

void message_handler(OSyncMessage*, void*);
void message_callback(OSyncMember*, context*, OSyncError**);

void process_free(PluginProcess *pp)
{
	if (pp->incoming) {
		osync_queue_disconnect(pp->incoming, NULL);
		osync_queue_remove(pp->incoming, NULL);
		osync_queue_free(pp->incoming);
	}

	if (pp->outgoing) {
		osync_queue_disconnect(pp->incoming, NULL);
		osync_queue_free(pp->outgoing);
	}

	if (pp->env)
		osync_env_free(pp->env);

	g_free(pp);
}

void process_error_shutdown(PluginProcess *pp, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, pp, error);

	OSyncMessage *message = osync_message_new(OSYNC_MESSAGE_ERROR, 0, NULL);
	if (!message)
		goto error;

	osync_marshal_error(message, *error);

	if (!osync_queue_send_message(pp->outgoing, NULL, message, NULL))
		goto error_free_message;

	sleep(1);

	process_free(pp);
	osync_trace(TRACE_EXIT, "%s", __func__);
	exit(1);

error_free_message:
	osync_message_unref(message);
error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	exit(2);
}

void osync_client_sync_alert_sink(OSyncMember *member)
{
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, member);
	PluginProcess *pp = (PluginProcess*)osync_member_get_data(member);

	OSyncError *error = NULL;

	OSyncMessage *message = osync_message_new(OSYNC_MESSAGE_SYNC_ALERT, 0, &error);
	if (!message)
		process_error_shutdown(pp, &error);

	if (!osync_queue_send_message(pp->outgoing, NULL, message, &error))
		process_error_shutdown(pp, &error);

	osync_trace(TRACE_EXIT, "%s", __func__);
}

void osync_client_changes_sink(OSyncMember *member, OSyncChange *change, void *user_data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, member, change, user_data);
	context *ctx = (context *)user_data;
	PluginProcess *pp = ctx->pp;
	OSyncMessage *orig = ctx->message;

	OSyncError *error = NULL;

	if (osync_message_is_answered(orig)) {
		osync_change_free(change);
		osync_trace(TRACE_EXIT, "%s", __func__);
		return;
	}

	OSyncMessage *message = osync_message_new(OSYNC_MESSAGE_NEW_CHANGE, 0, &error);
	if (!message)
		process_error_shutdown(pp, &error);

	osync_marshal_change(message, change);

	osync_message_write_long_long_int(message, osync_member_get_id(member));

	if (!osync_queue_send_message(pp->outgoing, NULL, message, &error))
		process_error_shutdown(pp, &error);

	osync_trace(TRACE_EXIT, "%s", __func__);
}

int main( int argc, char **argv )
{
	osync_trace(TRACE_ENTRY, "%s(%i, %p)", __func__, argc, argv);
	GMainLoop *syncloop;
	GMainContext *context;
	OSyncError *error = NULL;
	PluginProcess pp;
	assert(argc == 3);

	memset(&pp, 0, sizeof(pp));

	char *group_path = argv[ 1 ];
	int member_id = atoi( argv[ 2 ] );

	context = g_main_context_new();
	syncloop = g_main_loop_new(context, TRUE);

	/** Create environment **/
	OSyncEnv *env = osync_env_new();
	/* Don't load groups. We will load the group manually using osync_group_load() */
	osync_env_set_option(env, "LOAD_GROUPS", "no");

	/* Don't load plugins automatically if OSYNC_MODULE_LIST is set */
	char *module_list = getenv("OSYNC_MODULE_LIST");
	if (module_list) {
		osync_env_set_option(env, "LOAD_PLUGINS", "no");
		osync_env_set_option(env, "LOAD_FORMATS", "no");

		osync_trace(TRACE_INTERNAL, "OSYNC_MODULE_LIST variable: %s", module_list);

		char *str, *saveptr;
		for (str = module_list; ; str = NULL) {
			char *path = strtok_r(str, ":", &saveptr);
			if (!path)
				break;

			osync_trace(TRACE_INTERNAL, "Module to be loaded: %s", path);
			if (!osync_module_load(env, path, &error)) {
				fprintf(stderr, "Unable to load plugin %s: %s\n", path, osync_error_print(&error));
				osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&error));
				return 1;
			}
		}
	}

	if (!osync_env_initialize(env, &error)) {
		fprintf(stderr, "Unable to initialize environment: %s\n", osync_error_print(&error));
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&error));
		osync_error_free(&error);
		return 1;
	}

	/** Find group **/
	OSyncGroup *group = osync_group_load(env, group_path, &error);
	if (!group) {
		fprintf(stderr, "Unable to load group from path: %s\n", group_path);
		osync_trace(TRACE_EXIT_ERROR, "%s: Unable to load group from path: %s", __func__, group_path);
		return 2;
	}

	/** Find member **/
	int i;
	for ( i = 0; i < osync_group_num_members(group); ++i ) {
		pp.member = osync_group_nth_member(group, i);
		if (member_id == osync_member_get_id(pp.member))
			break;
		else
			pp.member = NULL;
	}
	if ( !pp.member ) {
		fprintf(stderr, "Unable to find member with id %d\n", member_id);
		osync_trace(TRACE_EXIT_ERROR, "%s: Unable to find member with id %d", __func__, member_id);
		return 3;
	}
	osync_trace(TRACE_INTERNAL, "+++++++++ This is the client #%d (%s plugin) of group %s", member_id, pp.member->pluginname, osync_group_get_name(group));

	/** Create connection pipes **/
	char *pipe_path = g_strdup_printf( "%s/pluginpipe", osync_member_get_configdir( pp.member ) );
	pp.incoming = osync_queue_new( pipe_path, &error );
	pp.outgoing = NULL;
	g_free( pipe_path );

	osync_queue_create( pp.incoming, &error );
	if ( osync_error_is_set( &error ) )
		osync_error_free( &error );

	/** Idle until the syncengine connects to (and reads from) our pipe **/
	if (!osync_queue_connect( pp.incoming, OSYNC_QUEUE_RECEIVER, 0 )) {
		fprintf(stderr, "Unable to connect\n");
		osync_trace(TRACE_EXIT_ERROR, "%s: Unable to connect", __func__);
		exit(1);
	}

	
	osync_member_set_data(pp.member, &pp);

	/** Set callback functions **/
	OSyncMemberFunctions *functions = osync_member_get_memberfunctions(pp.member);
	functions->rf_change = osync_client_changes_sink;
	//functions->rf_message = osync_client_message_sink;
	functions->rf_sync_alert = osync_client_sync_alert_sink;

	/** Start loop **/
	osync_trace(TRACE_INTERNAL, "plugin setting up mainloop");
	osync_queue_set_message_handler(pp.incoming, message_handler, &pp);
	osync_queue_setup_with_gmainloop(pp.incoming, context);
	osync_member_set_loop(pp.member, context);

	osync_trace(TRACE_INTERNAL, "running loop");
	g_main_loop_run(syncloop);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return 0;
}

void message_handler(OSyncMessage *message, void *user_data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	PluginProcess *pp = user_data;

	OSyncMessage *reply = NULL;
	OSyncError *error = NULL;
	//OSyncChange *change = 0;
	OSyncMember *member = pp->member;
	char *enginepipe = NULL;
   	context *ctx = NULL;

	osync_trace(TRACE_INTERNAL, "plugin received command %i", osync_message_get_command( message ));

	switch ( osync_message_get_command( message ) ) {
	case OSYNC_MESSAGE_NOOP:
		break;

	case OSYNC_MESSAGE_INITIALIZE:
		osync_trace(TRACE_INTERNAL, "init.");
		osync_message_read_string(message, &enginepipe);

		osync_trace(TRACE_INTERNAL, "enginepipe %s", enginepipe);
		pp->outgoing = osync_queue_new(enginepipe, NULL);
		if (!pp->outgoing) {
			fprintf(stderr, "Unable to make new queue\n");
			osync_trace(TRACE_EXIT_ERROR, "%s: Unable to make new queue", __func__);
			exit(1);
		}
		osync_trace(TRACE_INTERNAL, "connecting to engine");
		if (!osync_queue_connect(pp->outgoing, OSYNC_QUEUE_SENDER, 0 )) {
			fprintf(stderr, "Unable to connect queue\n");
			osync_trace(TRACE_EXIT_ERROR, "%s: Unable to connect queue", __func__);
			exit(1);
		}

		osync_trace(TRACE_INTERNAL, "done connecting to engine");
		/** Instanciate plugin **/
		if (!osync_member_instance_default_plugin(pp->member, &error))
			goto error;

		/** Initialize plugin **/
		if (!osync_member_initialize(pp->member, &error))
			goto error;

		pp->is_initialized = TRUE;

		osync_trace(TRACE_INTERNAL, "sending reply to engine");
		reply = osync_message_new_reply(message, NULL);
		if (!reply) {
			fprintf(stderr, "Unable to make new reply\n");
			osync_trace(TRACE_EXIT_ERROR, "%s: Unable to make new reply", __func__);
			exit(1);
		}

		if (!osync_queue_send_message(pp->outgoing, NULL, reply, NULL)) {
			fprintf(stderr, "Unable to send reply\n");
			osync_trace(TRACE_EXIT_ERROR, "%s: Unable to send reply", __func__);
			exit(1);
		}

		osync_trace(TRACE_INTERNAL, "done sending to engine");
		break;

	case OSYNC_MESSAGE_FINALIZE:
		if (pp->is_initialized)
			osync_member_finalize(pp->member);

		reply = osync_message_new_reply(message, NULL);
		if (!reply) {
			fprintf(stderr, "Unable to make new reply\n");
			osync_trace(TRACE_EXIT_ERROR, "%s: Unable to make new reply", __func__);
			exit(1);
		}

		if (!osync_queue_send_message(pp->outgoing, NULL, reply, NULL)) {
			fprintf(stderr, "Unable to send reply\n");
			osync_trace(TRACE_EXIT_ERROR, "%s: Unable to send reply", __func__);
			exit(1);
		}

		/*FIXME: how to wait for a message to be sent?
		 * We need to wait for the reply to be sent before exiting
		 */

		osync_trace(TRACE_EXIT, "%s", __func__);
		exit(0);
		break;

	case OSYNC_MESSAGE_CONNECT:
		osync_member_read_sink_info_full(member, message);

		ctx = g_malloc0(sizeof(context));
		ctx->pp = pp;
		ctx->message = message;
		osync_message_ref(message);

		/* connect() needs to tell the engine if it must perform a
		 * slow-sync, use add_reply_data() method for this
		 */
		ctx->add_reply_data = add_connect_reply_data;
		
		osync_member_connect(member, (OSyncEngCallback)message_callback, ctx);
		break;

	case OSYNC_MESSAGE_GET_CHANGES:
		osync_member_read_sink_info_full(member, message);

		ctx = g_malloc0(sizeof(context));
		ctx->pp = pp;
		ctx->message = message;
		osync_message_ref(message);
  		osync_member_get_changeinfo(member, (OSyncEngCallback)message_callback, ctx);
		break;

	case OSYNC_MESSAGE_COMMIT_CHANGE:
		ctx = g_malloc0(sizeof(context));
		ctx->pp = pp;
		ctx->message = message;
		osync_message_ref(message);
		OSyncChange *change;
  		osync_demarshal_change(message, member->group->conv_env, &change);
		osync_change_set_member(change, member);

		/* commit_change() needs to return some data back to the engine,
		 * use the add_reply_data() method for this
		 */
		ctx->change = change;
		ctx->add_reply_data = add_commit_change_reply_data;

	  	osync_member_commit_change(member, change, (OSyncEngCallback)message_callback, ctx);
		break;

	case OSYNC_MESSAGE_SYNC_DONE:
		ctx = g_malloc0(sizeof(context));
		ctx->pp = pp;
		ctx->message = message;
		osync_message_ref(message);
  		osync_member_sync_done(member, (OSyncEngCallback)message_callback, ctx);
		break;

	case OSYNC_MESSAGE_DISCONNECT:
		ctx = g_malloc0(sizeof(context));
		ctx->pp = pp;
		ctx->message = message;
		osync_message_ref(message);
  		osync_member_disconnect(member, (OSyncEngCallback)message_callback, ctx);
		break;

	case OSYNC_MESSAGE_REPLY:
		break;

  	case OSYNC_MESSAGE_ERRORREPLY:
		break;

	case OSYNC_MESSAGE_GET_CHANGEDATA:
		ctx = g_malloc0(sizeof(context));
		ctx->pp = pp;
		ctx->message = message;
		osync_message_ref(message);

  		osync_demarshal_change(message, member->group->conv_env, &change);
		osync_change_set_member(change, member);

		/* get_changedata needs to return the data from the change object back */
		ctx->change = change;
		ctx->add_reply_data = add_get_changedata_reply_data;

		osync_member_get_change_data(member, change, (OSyncEngCallback)message_callback, ctx);
		osync_trace(TRACE_EXIT, "message_handler");
		break;

  	case OSYNC_MESSAGE_COMMITTED_ALL:
		ctx = g_malloc0(sizeof(context));
		ctx->pp = pp;
		ctx->message = message;
		osync_message_ref(message);
  		osync_member_committed_all(member, (OSyncEngCallback)message_callback, ctx);
		break;

	/*case OSYNC_MESSAGE_READ_CHANGE:
		osync_demarshal_change( queue, &change, &error );
		osync_member_read_change(client->member, change, (OSyncEngCallback)message_callback, message);
		osync_trace(TRACE_EXIT, "message_handler");
		break;
	*/

	case OSYNC_MESSAGE_CALL_PLUGIN:
		/*
		char *function = itm_message_get_data(message, "function");
		void *data = itm_message_get_data(message, "data");
		OSyncError *error = NULL;
		void *replydata = osync_member_call_plugin(client->member, function, data, &error);

		if (itm_message_get_data(message, "want_reply")) {
			ITMessage *reply = NULL;
			if (!osync_error_is_set(&error)) {
				reply = itm_message_new_methodreply(client, message);
				itm_message_set_data(message, "reply", replydata);
			} else {
				reply = itm_message_new_errorreply(client, message);
				itm_message_set_error(reply, error);
			}

			itm_message_send_reply(reply);
		}
		*/
		break;
	case OSYNC_MESSAGE_QUEUE_HUP:
		osync_trace(TRACE_INTERNAL, "%s: ERROR: Queue hangup", __func__);
		fprintf(stderr, "Pipe closed! Exiting.\n");
		exit(1);
		break;
	default:
		osync_trace(TRACE_INTERNAL, "%s: ERROR: Unknown message", __func__);
		g_assert_not_reached();
		break;
	}

	if (reply)
		osync_message_unref(reply);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return;

error:;

	OSyncMessage *errorreply = osync_message_new_errorreply(message, NULL);
	if (!errorreply) {
		fprintf(stderr, "Unable to make new reply\n");
		osync_trace(TRACE_EXIT_ERROR, "%s", __func__);
		exit(1);
	}

	osync_marshal_error(errorreply, error);

	if (!osync_queue_send_message(pp->outgoing, NULL, errorreply, NULL)) {
		fprintf(stderr, "Unable to send error\n");
		osync_trace(TRACE_EXIT_ERROR, "%s", __func__);
		exit(1);
	}

	osync_message_unref(errorreply);

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&error));
	osync_error_free(&error);
}

/** add get_changedat-specific data to the get_changedata reply */
static osync_bool add_get_changedata_reply_data(OSyncMessage *reply, context *ctx, OSyncError **error)
{
	OSyncChange *change = ctx->change;

	assert(change);

	osync_marshal_changedata(reply, change);

	return TRUE;
}

/** Add commit_change-specific data to the commit_change reply */
static osync_bool add_commit_change_reply_data(OSyncMessage *reply, context *ctx, OSyncError **error)
{
	OSyncChange *change = ctx->change;

	assert(change);

	osync_message_write_string(reply, osync_change_get_uid(change));

	return TRUE;
}

/** Add connect-specific data to the connect reply */
static osync_bool add_connect_reply_data(OSyncMessage *reply, context *ctx, OSyncError **error)
{
	OSyncMember *member = ctx->pp->member;

	assert(member);

	osync_member_write_sink_info(member, reply);
	
	return TRUE;
}

void message_callback(OSyncMember *member, context *ctx, OSyncError **error)
{
	/*FIXME: handle errors in this function */

	OSyncError *myerror = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, member, ctx, error);

	OSyncMessage *message = ctx->message;
	PluginProcess *pp = ctx->pp;

	OSyncMessage *reply = NULL;

	if (osync_message_is_answered(message) == TRUE) {
		osync_message_unref(message);
		osync_trace(TRACE_EXIT, "%s", __func__);
		return;
	}

	if (!osync_error_is_set(error)) {
		reply = osync_message_new_reply(message, error);
		osync_debug("CLI", 4, "Member is replying with message %p to message %p:\"%lli-%i\" with no error", reply, message, message->id1, message->id2);
		/* Set method-specific data, if needed */
		if (ctx->add_reply_data)
			ctx->add_reply_data(reply, ctx, error);
	} else {
		reply = osync_message_new_errorreply(message, &myerror);
		osync_marshal_error(reply, *error);
		osync_debug("CLI", 1, "Member is replying with message %p to message %p:\"%lli-%i\" with error %i: %s", reply, message, message->id1, message->id2, osync_error_get_type(error), osync_error_print(error));
	}

	g_free(ctx);

	osync_queue_send_message(pp->outgoing, NULL, reply, NULL);
	osync_message_set_answered(message);

	osync_message_unref(message);
	osync_message_unref(reply);

	osync_trace(TRACE_EXIT, "%s", __func__);
}

void *osync_client_message_sink(OSyncMember *member, const char *name, void *data, osync_bool synchronous)
{
	/*TODO: Implement support for PLUGIN_MESSAGE */
/*
	OSyncClient *client = osync_member_get_data(member);
	OSyncEngine *engine = client->engine;
	if (!synchronous) {

		ITMessage *message = itm_message_new_signal(client, "PLUGIN_MESSAGE");
		osync_debug("CLI", 3, "Sending message %p PLUGIN_MESSAGE for message %s", message, name);
		itm_message_set_data(message, "data", data);
		itm_message_set_data(message, "name", g_strdup(name));
		itm_queue_send(engine->incoming, message);

		return NULL;
	} else {
		return engine->plgmsg_callback(engine, client, name, data, engine->plgmsg_userdata);
	}
*/
  return NULL;
}