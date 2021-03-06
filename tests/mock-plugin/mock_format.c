/*
 * opensync - A plugin for file objects for the opensync framework
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
 
#include <opensync/opensync.h>
#include <opensync/opensync_internals.h>
#include <opensync/opensync-support.h>
#include <opensync/opensync-serializer.h>
#include <opensync/opensync-format.h>
#include <glib.h>
#include "mock_format.h"


static osync_bool mock_format_get_error(const char *domain)
{
        const char *env = g_getenv(domain);

        if (!env)
                return FALSE;

        char *chancestr = g_strdup_printf("%s_PROB", domain);
        const char *chance = g_getenv(chancestr);
        g_free(chancestr);
        if (!chance)
                return TRUE;
        int prob = atoi(chance);
        if (prob >= g_random_int_range(0, 100))
                return TRUE;

        return FALSE;
}

static OSyncConvCmpResult compare_file(const char *leftdata, unsigned int leftsize, const char *rightdata, unsigned int rightsize)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %i, %p, %i)", __func__, leftdata, leftsize, rightdata, rightsize);
	osync_assert(leftdata);
	osync_assert(rightdata);
	
	OSyncFileFormat *leftfile = (OSyncFileFormat *)leftdata;
	OSyncFileFormat *rightfile = (OSyncFileFormat *)rightdata;
	
	osync_assert(rightfile->path);
	osync_assert(leftfile->path);
	
	osync_trace(TRACE_INTERNAL, "Comparing %s and %s", leftfile->path, rightfile->path);
			
	
	if (mock_format_get_error("MOCK_FORMAT_PATH_COMPARE_NO") || !strcmp(leftfile->path, rightfile->path)) {
		if (leftfile->size == rightfile->size) {
			if (leftfile->size == 0 || !memcmp(leftfile->data, rightfile->data, rightfile->size)) {
				osync_trace(TRACE_EXIT, "%s: Same", __func__);
				return OSYNC_CONV_DATA_SAME;
			}
		}
		
		osync_trace(TRACE_EXIT, "%s: Similar", __func__);
		return OSYNC_CONV_DATA_SIMILAR;
	}
	
	osync_trace(TRACE_EXIT, "%s: Mismatch", __func__);
	return OSYNC_CONV_DATA_MISMATCH;
}

static osync_bool conv_file_to_plain(char *input, unsigned int inpsize, char **output, unsigned int *outpsize, osync_bool *free_input, const char *config, void *userdata, OSyncError **error)
{
	osync_trace(TRACE_INTERNAL, "Converting file to plain");
	
	*free_input = TRUE;
	OSyncFileFormat *file = (OSyncFileFormat *)input;

	/* Add a \0 to make a usable plain (text) format. input gets freed by destroy_func() */
	char *plaindata = osync_try_malloc0(file->size + 1, error);
	memcpy(plaindata, file->data, file->size);

	*output = plaindata; 
	*outpsize = file->size + 1;

	return TRUE;
}

static osync_bool conv_plain_to_file(char *input, unsigned int inpsize, char **output, unsigned int *outpsize, osync_bool *free_input, const char *config, void *userdata, OSyncError **error)
{
	osync_trace(TRACE_INTERNAL, "Converting plain to file");
	
	*free_input = FALSE;
	OSyncFileFormat *file = osync_try_malloc0(sizeof(OSyncFileFormat), error);
	osync_assert(file);

	file->path = osync_rand_str(g_random_int_range(1, 100));
	
	file->data = input;
	file->size = inpsize - 1;
	
	*output = (char *)file;
	*outpsize = sizeof(OSyncFileFormat);
	return TRUE;
}

static void destroy_file(char *input, unsigned int inpsize)
{
	OSyncFileFormat *file = (OSyncFileFormat *)input;
	
	if (file->data)
		g_free(file->data);
		
	if (file->path)
		g_free(file->path);
	
	g_free(file);
}

static osync_bool duplicate_file(const char *uid, const char *input, unsigned int insize, char **newuid, char **output, unsigned int *outsize, osync_bool *dirty, OSyncError **error)
{
	OSyncFileFormat *file = (OSyncFileFormat *)input;
	
	char *newpath = g_strdup_printf ("%s-dupe", file->path);
	g_free(file->path);
	file->path = newpath;
	*newuid = g_strdup(file->path);
	*dirty = TRUE;
	return TRUE;
}

static osync_bool copy_file(const char *input, unsigned int inpsize, char **output, unsigned int *outpsize, OSyncError **error)
{
	OSyncFileFormat *inpfile = (OSyncFileFormat *)input;
	
	OSyncFileFormat *outfile = osync_try_malloc0(sizeof(OSyncFileFormat), error);
	osync_assert(outfile);
	
	if (inpfile->data) {
		outfile->data = g_malloc0(inpfile->size);
		memcpy(outfile->data, inpfile->data, inpfile->size);
		outfile->size = inpfile->size;
	}
	
	outfile->path = g_strdup(inpfile->path);
	
	*output = (char *)outfile;
	*outpsize = sizeof(OSyncFileFormat);
	return TRUE;
}

static void create_file(char **buffer, unsigned int *size)
{
	OSyncFileFormat *outfile = osync_try_malloc0(sizeof(OSyncFileFormat), NULL);
	
	outfile->path = osync_rand_str(g_random_int_range(1, 100));
	
	outfile->data = osync_rand_str(g_random_int_range(1, 100));
	outfile->size = strlen(outfile->data);
	
	*buffer = (char *)outfile;
	*size = sizeof(OSyncFileFormat);
}

static time_t revision_file(const char *input, unsigned int inpsize, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %i, %p)", __func__, input, inpsize, error);
	
	OSyncFileFormat *file = (OSyncFileFormat *)input;
	time_t lastmod = file->last_mod;
	
	osync_trace(TRACE_EXIT, "%s: %i", __func__, lastmod);
	return lastmod;
}

static char *print_file(const char *data, unsigned int size)
{
	OSyncFileFormat *file = (OSyncFileFormat *)data;
	
	char *printable = g_strdup_printf ("File %s: size: %i", file->path, file->size);
	return printable;
}

static osync_bool marshal_file(const char *input, unsigned int inpsize, OSyncMessage *message, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %i, %p, %p)", __func__, input, inpsize, message, error);
	
	OSyncFileFormat *file = (OSyncFileFormat *)input;
	
	osync_message_write_string(message, file->path);
	osync_message_write_buffer(message, file->data, file->size);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
}

static osync_bool demarshal_file(OSyncMessage *message, char **output, unsigned int *outpsize, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %p)", __func__, message, output, outpsize, error);
	
	OSyncFileFormat *file = osync_try_malloc0(sizeof(OSyncFileFormat), error);
	osync_assert(file);
	
	osync_message_read_string(message, &(file->path));
	osync_message_read_buffer(message, (void *)&(file->data), (int *)&(file->size));
	
	*output = (char *)file;
	*outpsize = sizeof(OSyncFileFormat);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
}

static void _format_set_functions(OSyncObjFormat *format)
{
	osync_objformat_set_compare_func(format, compare_file);
	osync_objformat_set_destroy_func(format, destroy_file);
	osync_objformat_set_duplicate_func(format, duplicate_file);
	osync_objformat_set_print_func(format, print_file);
	osync_objformat_set_revision_func(format, revision_file);
	osync_objformat_set_copy_func(format, copy_file);
	osync_objformat_set_create_func(format, create_file);
	
	osync_objformat_set_marshal_func(format, marshal_file);
	osync_objformat_set_demarshal_func(format, demarshal_file);
}

osync_bool get_format_info(OSyncFormatEnv *env, OSyncError **error)
{
	OSyncObjFormat *format = NULL;

	/* mockformat1 */
	format = osync_objformat_new("mockformat1", "mockobjtype1", error);
	osync_assert(format);

	_format_set_functions(format);

	osync_format_env_register_objformat(env, format);
	osync_objformat_unref(format);

	/* mockformat2 */
	format = osync_objformat_new("mockformat2", "mockobjtype2", error);
	osync_assert(format);

	_format_set_functions(format);

	osync_format_env_register_objformat(env, format);
	osync_objformat_unref(format);

	/* mockformat3 */
	format = osync_objformat_new("mockformat3", "mockobjtype3", error);
	osync_assert(format);

	_format_set_functions(format);

	osync_format_env_register_objformat(env, format);
	osync_objformat_unref(format);

	return TRUE;
}

osync_bool get_conversion_info(OSyncFormatEnv *env, OSyncError **error)
{
	OSyncFormatConverter *conv = NULL;

	/* mock converter */
	OSyncObjFormat *mockformat1 = osync_format_env_find_objformat(env, "mockformat1");
	osync_assert(mockformat1);
	
	OSyncObjFormat *mockformat2 = osync_format_env_find_objformat(env, "mockformat2");
	osync_assert(mockformat2);
	
	conv = osync_converter_new(OSYNC_CONVERTER_DECAP, mockformat1, mockformat2, conv_file_to_plain, error);
	osync_assert(conv);
	
	osync_format_env_register_converter(env, conv);
	osync_converter_unref(conv);
	
	conv = osync_converter_new(OSYNC_CONVERTER_ENCAP, mockformat2, mockformat1, conv_plain_to_file, error);
	osync_assert(conv);
	
	osync_format_env_register_converter(env, conv);
	osync_converter_unref(conv);


	return TRUE;
}

int get_version(void)
{
	return 1;
}
