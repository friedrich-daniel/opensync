/*
 * xmlformat-event - A plugin for parsing vevent objects for the opensync framework
 * Copyright (C) 2004-2005  Armin Bauer <armin.bauer@opensync.org>
 * Copyright (C) 2007  Daniel Gollub <dgollub@suse.de>
 * Copyright (C) 2007  Christopher Stender <cstender@suse.de>
 * Copyright (C) 2007  Jerry Yu <jijun.yu@sun.com>
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

#include "xmlformat-vevent.h"
#include "xmlformat-vevent10.h"
#include "xmlformat-vevent20.h"

void *init_vevent_to_xmlformat(VFormatType target)
{
	osync_trace(TRACE_ENTRY, "%s", __func__);

	OSyncHookTables *hooks = g_malloc0(sizeof(OSyncHookTables));
	
	/*
	hooks->table = g_hash_table_new(g_str_hash, g_str_equal);
	hooks->tztable = g_hash_table_new(g_str_hash, g_str_equal);
	hooks->comptable = g_hash_table_new(g_str_hash, g_str_equal);
	hooks->compparamtable = g_hash_table_new(g_str_hash, g_str_equal);
	hooks->alarmtable = g_hash_table_new(g_str_hash, g_str_equal);
	*/

	hooks->attributes = g_hash_table_new(g_str_hash, g_str_equal);
	hooks->parameters = g_hash_table_new(g_str_hash, g_str_equal);
	
        // [vcal-1.0] vcal (same order as in spec!)
	insert_attr_handler(hooks->attributes, "BEGIN", HANDLE_IGNORE);
	insert_attr_handler(hooks->attributes, "END", HANDLE_IGNORE);

        // [vcal-1.0] calprop (same order as in spec!)
	insert_attr_handler(hooks->attributes, "DAYLIGHT", HANDLE_IGNORE); // TODO
	insert_attr_handler(hooks->attributes, "GEO", handle_geo_attribute);
	insert_attr_handler(hooks->attributes, "PRODID", handle_prodid_attribute);
	insert_attr_handler(hooks->attributes, "TZ", HANDLE_IGNORE);
	insert_attr_handler(hooks->attributes, "VERSION", HANDLE_IGNORE);

        // [vcal-1.0] simprop (same order as in spec!)
	insert_attr_handler(hooks->attributes, "ATTACH", handle_attach_attribute);
	insert_attr_handler(hooks->attributes, "ATTENDEE", handle_attendee_attribute);
	insert_attr_handler(hooks->attributes, "DCREATED", handle_created_attribute);
	insert_attr_handler(hooks->attributes, "COMPLETED", handle_completed_attribute);
	insert_attr_handler(hooks->attributes, "DESCRIPTION", handle_description_attribute);
	insert_attr_handler(hooks->attributes, "DUE", handle_due_attribute);
	insert_attr_handler(hooks->attributes, "DTEND", handle_dtend_attribute);
	insert_attr_handler(hooks->attributes, "EXRULE", handle_exrule_attribute);
	insert_attr_handler(hooks->attributes, "LAST-MODIFIED", handle_last_modified_attribute);
	insert_attr_handler(hooks->attributes, "LOCATION", handle_location_attribute);
	insert_attr_handler(hooks->attributes, "RNUM", HANDLE_IGNORE); // TODO
	insert_attr_handler(hooks->attributes, "PRIORITY", handle_priority_attribute);
	insert_attr_handler(hooks->attributes, "RELATED-TO", handle_related_attribute);
	insert_attr_handler(hooks->attributes, "RRULE", handle_rrule_attribute);
	insert_attr_handler(hooks->attributes, "SEQUENCE", handle_sequence_attribute);
	insert_attr_handler(hooks->attributes, "DTSTART", handle_dtstart_attribute);
	insert_attr_handler(hooks->attributes, "SUMMARY", handle_summary_attribute);
	insert_attr_handler(hooks->attributes, "TRANSP", handle_transp_attribute);
	insert_attr_handler(hooks->attributes, "URL", handle_url_attribute);
	insert_attr_handler(hooks->attributes, "UID", handle_uid_attribute);

        // [vcal-1.0] entprop (same order as in spec!)
	insert_attr_handler(hooks->attributes, "AALARM", handle_aalarm_attribute);
	insert_attr_handler(hooks->attributes, "CATEGORIES", handle_categories_attribute);
	insert_attr_handler(hooks->attributes, "CLASS", handle_class_attribute);
	insert_attr_handler(hooks->attributes, "DALARM", handle_dalarm_attribute);
	insert_attr_handler(hooks->attributes, "EXDATE", handle_exdate_attribute);
	insert_attr_handler(hooks->attributes, "MALARM", HANDLE_IGNORE);  // TODO
	insert_attr_handler(hooks->attributes, "PALARM", HANDLE_IGNORE);  // TODO
	insert_attr_handler(hooks->attributes, "RDATE", handle_rdate_attribute);
	insert_attr_handler(hooks->attributes, "RESOURCES", handle_resources_attribute);
	insert_attr_handler(hooks->attributes, "STATUS", handle_status_attribute);

        // [vcal-1.0] param (same order as in sepc!)
	insert_attr_handler(hooks->parameters, "TYPE", HANDLE_IGNORE); // TODO
	// insert_attr_handler(hooks->parameters, "VALUE", handle_value_parameter);
	insert_attr_handler(hooks->parameters, "ENCODING", HANDLE_IGNORE); // TODO
	insert_attr_handler(hooks->parameters, "CHARSET", HANDLE_IGNORE); // TODO
	insert_attr_handler(hooks->parameters, "LANGUAGE", HANDLE_IGNORE); // TODO
	//insert_attr_handler(hooks->parameters, "ROLE", handle_role_parameter);
	//insert_attr_handler(hooks->parameters, "STATUS", handle_status_parameter);


// XXX: not quite sure if RSVP is part of vcal-1.0 .. some traces about in the spec...
//	insert_attr_handler(hooks->parameters, "RSVP", handle_rsvp_parameter);

	osync_trace(TRACE_EXIT, "%s: %p", __func__, hooks);
	return (void *)hooks;
}


static OSyncHookTables *init_xmlformat_to_vevent(void)
{
	osync_trace(TRACE_ENTRY, "%s", __func__);
	
	OSyncHookTables *hooks = g_malloc0(sizeof(OSyncHookTables));

	
	/*
	hooks->table = g_hash_table_new(g_str_hash, g_str_equal);
	hooks->tztable = g_hash_table_new(g_str_hash, g_str_equal);
	hooks->comptable = g_hash_table_new(g_str_hash, g_str_equal);
	hooks->alarmtable = g_hash_table_new(g_str_hash, g_str_equal);
	*/

	hooks->attributes = g_hash_table_new(g_str_hash, g_str_equal);
	hooks->parameters = g_hash_table_new(g_str_hash, g_str_equal);

	//todo attributes
	insert_xml_attr_handler(hooks->attributes, "Uid", handle_xml_uid_attribute);
	insert_xml_attr_handler(hooks->attributes, "DateCalendarCreated", handle_xml_dtstamp_attribute);
	insert_xml_attr_handler(hooks->attributes, "Description", handle_xml_description_attribute);
	insert_xml_attr_handler(hooks->attributes, "Summary", handle_xml_summary_attribute);
	insert_xml_attr_handler(hooks->attributes, "DateDue", handle_xml_due_attribute);
	insert_xml_attr_handler(hooks->attributes, "DateStarted", handle_xml_dtstart_attribute);
	insert_xml_attr_handler(hooks->attributes, "PercentComplete", handle_xml_percent_complete_attribute);
	insert_xml_attr_handler(hooks->attributes, "Class", handle_xml_class_attribute);
	insert_xml_attr_handler(hooks->attributes, "Categories", handle_xml_categories_attribute);
	insert_xml_attr_handler(hooks->attributes, "Priority", handle_xml_priority_attribute);
	insert_xml_attr_handler(hooks->attributes, "Url", handle_xml_url_attribute);
	insert_xml_attr_handler(hooks->attributes, "Sequence", handle_xml_sequence_attribute);
	insert_xml_attr_handler(hooks->attributes, "LastModified", handle_xml_last_modified_attribute);
	insert_xml_attr_handler(hooks->attributes, "DateCreated", handle_xml_created_attribute);
	insert_xml_attr_handler(hooks->attributes, "RecurrenceRule", handle_xml_rrule_attribute);
	insert_xml_attr_handler(hooks->attributes, "RecurrenceDate", handle_xml_rdate_attribute);
	insert_xml_attr_handler(hooks->attributes, "Location", handle_xml_location_attribute);
	insert_xml_attr_handler(hooks->attributes, "Geo", handle_xml_geo_attribute);
	insert_xml_attr_handler(hooks->attributes, "Completed", handle_xml_completed_attribute);
	insert_xml_attr_handler(hooks->attributes, "Organizer", handle_xml_organizer_attribute);
	insert_xml_attr_handler(hooks->attributes, "RecurrenceID", handle_xml_recurid_attribute);
	insert_xml_attr_handler(hooks->attributes, "Status", handle_xml_status_attribute);
	insert_xml_attr_handler(hooks->attributes, "Duration", handle_xml_duration_attribute);
	insert_xml_attr_handler(hooks->attributes, "Attach", handle_xml_attach_attribute);
	insert_xml_attr_handler(hooks->attributes, "Attendee", handle_xml_attendee_attribute);
	insert_xml_attr_handler(hooks->attributes, "Contact", handle_xml_event_attribute);
	insert_xml_attr_handler(hooks->attributes, "ExclusionDate", handle_xml_exdate_attribute);
	insert_xml_attr_handler(hooks->attributes, "ExclusionRule", handle_xml_exrule_attribute);
	insert_xml_attr_handler(hooks->attributes, "RStatus", handle_xml_rstatus_attribute);
	insert_xml_attr_handler(hooks->attributes, "Related", handle_xml_related_attribute);
	insert_xml_attr_handler(hooks->attributes, "Resources", handle_xml_resources_attribute);
	insert_xml_attr_handler(hooks->attributes, "DateEnd", handle_xml_dtend_attribute);
	insert_xml_attr_handler(hooks->attributes, "TimeTransparency", handle_xml_transp_attribute);


	/*
	insert_xml_attr_handler(hooks->parameters, "Category", handle_xml_category_parameter);
	insert_xml_attr_handler(hooks->parameters, "Rule", handle_xml_rule_parameter);
	insert_xml_attr_handler(hooks->parameters, "Value", handle_xml_value_parameter);
	insert_xml_attr_handler(hooks->parameters, "AlternateRep", handle_xml_altrep_parameter);
	insert_xml_attr_handler(hooks->parameters, "CommonName", handle_xml_cn_parameter);
	insert_xml_attr_handler(hooks->parameters, "DelegatedFrom", handle_xml_delegated_from_parameter);
	insert_xml_attr_handler(hooks->parameters, "DelegatedTo", handle_xml_delegated_to_parameter);
	insert_xml_attr_handler(hooks->parameters, "Directory", handle_xml_dir_parameter);
	insert_xml_attr_handler(hooks->parameters, "FormaType", handle_xml_format_type_parameter);
	insert_xml_attr_handler(hooks->parameters, "FreeBusyType", handle_xml_fb_type_parameter);
	insert_xml_attr_handler(hooks->parameters, "Member", handle_xml_member_parameter);
	insert_xml_attr_handler(hooks->parameters, "PartStat", handle_xml_partstat_parameter);
	insert_xml_attr_handler(hooks->parameters, "Range", handle_xml_range_parameter);
	insert_xml_attr_handler(hooks->parameters, "Related", handle_xml_related_parameter);
	insert_xml_attr_handler(hooks->parameters, "RelationType", handle_xml_reltype_parameter);
	insert_xml_attr_handler(hooks->parameters, "Role", handle_xml_role_parameter);
	insert_xml_attr_handler(hooks->parameters, "RSVP", handle_xml_rsvp_parameter);
	insert_xml_attr_handler(hooks->parameters, "SentBy", handle_xml_sent_by_parameter);
	*/
	
	//vcal attributes
	insert_xml_attr_handler(hooks->attributes, "CalendarScale", handle_xml_calscale_attribute);
	insert_xml_attr_handler(hooks->attributes, "ProductID", handle_xml_prodid_attribute);
	insert_xml_attr_handler(hooks->attributes, "Method", handle_xml_method_attribute);
//	insert_xml_attr_handler(hooks->attributes, "UnknownNode", xml_handle_unknown_attribute);
//	insert_xml_attr_handler(hooks->attributes, "UnknownParameter", xml_handle_unknown_parameter);
	
	/*
	//Timezone
	insert_xml_attr_handler(hooks->attributes, "TimezoneID", handle_xml_tzid_attribute);
	insert_xml_attr_handler(hooks->attributes, "Location", handle_xml_tz_location_attribute);
	insert_xml_attr_handler(hooks->attributes, "TZOffsetFrom", handle_xml_tzoffsetfrom_location_attribute);
	insert_xml_attr_handler(hooks->attributes, "TZOffsetTo", handle_xml_tzoffsetto_location_attribute);
	insert_xml_attr_handler(hooks->attributes, "TimezoneName", handle_xml_tzname_attribute);
	insert_xml_attr_handler(hooks->attributes, "DateStarted", handle_xml_tzdtstart_attribute);
	insert_xml_attr_handler(hooks->attributes, "RecurrenceRule", handle_xml_tzrrule_attribute);
	insert_xml_attr_handler(hooks->attributes, "LastModified", handle_xml_tz_last_modified_attribute);
	insert_xml_attr_handler(hooks->attributes, "TimezoneUrl", handle_xml_tzurl_attribute);
	insert_xml_attr_handler(hooks->attributes, "RecurrenceDate", handle_xml_tzrdate_attribute);
	*/

	/*
//	insert_xml_attr_handler(hooks->parameters, "Category", handle_xml_category_parameter);
//	insert_xml_attr_handler(hooks->parameters, "Rule", handle_xml_rule_parameter);
	insert_xml_attr_handler(hooks->parameters, "Value", handle_xml_value_parameter);
	insert_xml_attr_handler(hooks->parameters, "AlternateRep", handle_xml_altrep_parameter);
	insert_xml_attr_handler(hooks->parameters, "CommonName", handle_xml_cn_parameter);
	insert_xml_attr_handler(hooks->parameters, "DelegatedFrom", handle_xml_delegated_from_parameter);
	insert_xml_attr_handler(hooks->parameters, "DelegatedTo", handle_xml_delegated_to_parameter);
	insert_xml_attr_handler(hooks->parameters, "Directory", handle_xml_dir_parameter);
	insert_xml_attr_handler(hooks->parameters, "FormaType", handle_xml_format_type_parameter);
	insert_xml_attr_handler(hooks->parameters, "FreeBusyType", handle_xml_fb_type_parameter);
	insert_xml_attr_handler(hooks->parameters, "Member", handle_xml_member_parameter);
	insert_xml_attr_handler(hooks->parameters, "PartStat", handle_xml_partstat_parameter);
	insert_xml_attr_handler(hooks->parameters, "Range", handle_xml_range_parameter);
	insert_xml_attr_handler(hooks->parameters, "Related", handle_xml_related_parameter);
	insert_xml_attr_handler(hooks->parameters, "RelationType", handle_xml_reltype_parameter);
	insert_xml_attr_handler(hooks->parameters, "Role", handle_xml_role_parameter);
	insert_xml_attr_handler(hooks->parameters, "RSVP", handle_xml_rsvp_parameter);
	insert_xml_attr_handler(hooks->parameters, "SentBy", handle_xml_sent_by_parameter);
	*/
	
	//VAlarm component
	insert_xml_attr_handler(hooks->attributes, "AlarmTrigger", handle_xml_atrigger_attribute);
	insert_xml_attr_handler(hooks->attributes, "AlarmRepeat", handle_xml_arepeat_attribute);
	insert_xml_attr_handler(hooks->attributes, "AlarmDuration", handle_xml_aduration_attribute);
	insert_xml_attr_handler(hooks->attributes, "AlarmAction", handle_xml_aaction_attribute);
	insert_xml_attr_handler(hooks->attributes, "AlarmAttach", handle_xml_aattach_attribute);
	insert_xml_attr_handler(hooks->attributes, "AlarmDescription", handle_xml_adescription_attribute);
	insert_xml_attr_handler(hooks->attributes, "AlarmAttendee", handle_xml_aattendee_attribute);
	insert_xml_attr_handler(hooks->attributes, "AlarmSummary", handle_xml_asummary_attribute);

	/*
	//FIXME: The functions below shouldn't be on alarmtable, but on other hash table
//	insert_xml_attr_handler(hooks->parameters, "Category", handle_xml_category_parameter);
	insert_xml_attr_handler(hooks->parameters, "Rule", handle_xml_rule_parameter);
	insert_xml_attr_handler(hooks->parameters, "Value", handle_xml_value_parameter);
	insert_xml_attr_handler(hooks->parameters, "AlternateRep", handle_xml_altrep_parameter);
	insert_xml_attr_handler(hooks->parameters, "CommonName", handle_xml_cn_parameter);
	insert_xml_attr_handler(hooks->parameters, "DelegatedFrom", handle_xml_delegated_from_parameter);
	insert_xml_attr_handler(hooks->parameters, "DelegatedTo", handle_xml_delegated_to_parameter);
	insert_xml_attr_handler(hooks->parameters, "Directory", handle_xml_dir_parameter);
	insert_xml_attr_handler(hooks->parameters, "FormaType", handle_xml_format_type_parameter);
	insert_xml_attr_handler(hooks->parameters, "FreeBusyType", handle_xml_fb_type_parameter);
	insert_xml_attr_handler(hooks->parameters, "Member", handle_xml_member_parameter);
	insert_xml_attr_handler(hooks->parameters, "PartStat", handle_xml_partstat_parameter);
	insert_xml_attr_handler(hooks->parameters, "Range", handle_xml_range_parameter);
	insert_xml_attr_handler(hooks->parameters, "Related", handle_xml_related_parameter);
	insert_xml_attr_handler(hooks->parameters, "RelationType", handle_xml_reltype_parameter);
	insert_xml_attr_handler(hooks->parameters, "Role", handle_xml_role_parameter);
	insert_xml_attr_handler(hooks->parameters, "RSVP", handle_xml_rsvp_parameter);
	insert_xml_attr_handler(hooks->parameters, "SentBy", handle_xml_sent_by_parameter);
	*/
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, hooks);
	return (void *)hooks;
}


osync_bool conv_xmlformat_to_vevent(char *input, unsigned int inpsize, char **output, unsigned int *outpsize, osync_bool *free_input, const char *config, OSyncError **error, int target)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %i, %p, %p, %p, %p)", __func__, input, inpsize, output, outpsize, free_input, error);

	OSyncHookTables *hooks = init_xmlformat_to_vevent();

	int i = 0;
	if (config) {
		gchar** config_array = g_strsplit_set(config, "=;", 0);
		for(i=0; config_array[i]; i+=2)
		{
			if(!config_array[i+1]) {
				osync_trace(TRACE_ERROR, "Error in the converter configuration.");
				g_hash_table_destroy(hooks->attributes);
				g_hash_table_destroy(hooks->parameters);
				g_free(hooks);
				g_strfreev(config_array);
				return FALSE;
			}
			
			if(strcmp(config_array[i], "VCAL_EXTENSION") == 0) {

				/*
				if(strcmp(config_array[i+1], "KDE") == 0)
					init_xmlformat_to_kde(hooks);
				else if(strcmp(config_array[i+1], "Evolution") == 0)
					init_xmlformat_to_evolution(hooks);
				*/	
					
			}else if(strcmp(config_array[i], "VCAL_ENCODING")) {
				
				if(strcmp(config_array[i+1], "UTF-16") == 0)
					;
				/* TODO: what to do? :) */
			}
		}
		g_strfreev(config_array);
	}

	OSyncXMLFormat *xmlformat = (OSyncXMLFormat *)input;
	unsigned int size;
	char *str;
	osync_xmlformat_assemble(xmlformat, &str, &size);
	osync_trace(TRACE_INTERNAL, "Input XMLFormat is:\n%s", str);
	g_free(str);

	//Make the new vcal
	VFormat *ical = vformat_new();
	
	osync_trace(TRACE_INTERNAL, "parsing xml attributes");
	const char *std_encoding = NULL;
	if (target == VFORMAT_EVENT_10)
		std_encoding = "QUOTED-PRINTABLE";
	else
		std_encoding = "B";
	
	OSyncXMLField *xmlfield = osync_xmlformat_get_first_field(xmlformat);
	for(; xmlfield != NULL; xmlfield = osync_xmlfield_get_next(xmlfield)) {
		xml_handle_attribute(hooks, ical, xmlfield, std_encoding);
	}
	
	g_hash_table_destroy(hooks->attributes);
	g_hash_table_destroy(hooks->parameters);
	g_free(hooks);

	*free_input = TRUE;
	*output = vformat_to_string(ical, target);
	*outpsize = strlen(*output) + 1;

	vformat_free(ical);

	osync_trace(TRACE_INTERNAL, "Output icalendar is: \n%s", *output);

	osync_trace(TRACE_EXIT, "%s", __func__);
	
	return TRUE;
}


void get_conversion_info(OSyncFormatEnv *env)
{
	OSyncFormatConverter *conv;
	OSyncError *error = NULL;
	
	OSyncObjFormat *xmlformat = osync_format_env_find_objformat(env, "xmlformat-event");
	OSyncObjFormat *vcal = osync_format_env_find_objformat(env, "vevent10");
	OSyncObjFormat *ical = osync_format_env_find_objformat(env, "vevent20");
	
	conv = osync_converter_new(OSYNC_CONVERTER_CONV, xmlformat, vcal, conv_xmlformat_to_vevent10, &error);
	if (!conv) {
		osync_trace(TRACE_ERROR, "Unable to register format converter: %s", osync_error_print(&error));
		osync_error_unref(&error);
		return;
	}
	osync_format_env_register_converter(env, conv);
	osync_converter_unref(conv);
	
	conv = osync_converter_new(OSYNC_CONVERTER_CONV, vcal, xmlformat, conv_vevent10_to_xmlformat, &error);
	if (!conv) {
		osync_trace(TRACE_ERROR, "Unable to register format converter: %s", osync_error_print(&error));
		osync_error_unref(&error);
		return;
	}
	osync_format_env_register_converter(env, conv);
	osync_converter_unref(conv);
	
	conv = osync_converter_new(OSYNC_CONVERTER_CONV, xmlformat, ical, conv_xmlformat_to_vevent20, &error);
	if (!conv) {
		osync_trace(TRACE_ERROR, "Unable to register format converter: %s", osync_error_print(&error));
		osync_error_unref(&error);
		return;
	}
	osync_format_env_register_converter(env, conv);
	osync_converter_unref(conv);
	
	conv = osync_converter_new(OSYNC_CONVERTER_CONV, ical, xmlformat, conv_vevent20_to_xmlformat, &error);
	if (!conv) {
		osync_trace(TRACE_ERROR, "Unable to register format converter: %s", osync_error_print(&error));
		osync_error_unref(&error);
		return;
	}
	osync_format_env_register_converter(env, conv);
	osync_converter_unref(conv);
}

int get_version(void)
{
	return 1;
}
