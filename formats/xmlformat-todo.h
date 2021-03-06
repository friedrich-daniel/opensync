/*
 * xmlformat - registration of xml object formats 
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

#ifndef XMLFORMAT_TODO_H
#define XMLFORMAT_TODO_H

#include "xmlformat-common.h"
#include "xmlformat-calendar.h"

#define XMLFORMAT_TODO_ROOT				"todo"
#define XMLFORMAT_TODO_VERSION				"version"

// Alarm
#define XMLFORMAT_TODO_ALARM				"Alarm"
#define XMLFORMAT_TODO_ALARM_ACTION			XMLFORMAT_CALENDAR_ALARM_ACTION
#define XMLFORMAT_TODO_ALARM_ACTION_AUDIO		XMLFORMAT_CALENDAR_ALARM_ACTION_AUDIO	
#define XMLFORMAT_TODO_ALARM_ACTION_DISPLAY		XMLFORMAT_CALENDAR_ALARM_ACTION_DISPLAY	
#define XMLFORMAT_TODO_ALARM_ACTION_EMAIL		XMLFORMAT_CALENDAR_ALARM_ACTION_EMAIL	
#define XMLFORMAT_TODO_ALARM_ACTION_PROCEDURE		XMLFORMAT_CALENDAR_ALARM_ACTION_PROCEDURE	
#define XMLFORMAT_TODO_ALARM_ATTACH			XMLFORMAT_CALENDAR_ALARM_ATTACH	
#define XMLFORMAT_TODO_ALARM_ATTENDEE			XMLFORMAT_CALENDAR_ALARM_ATTENDEE
#define XMLFORMAT_TODO_ALARM_DESCRIPTION		XMLFORMAT_CALENDAR_ALARM_DESCRIPTION
#define XMLFORMAT_TODO_ALARM_REPEAT			XMLFORMAT_CALENDAR_ALARM_REPEAT	
#define XMLFORMAT_TODO_ALARM_REPEATDURATION		XMLFORMAT_CALENDAR_ALARM_REPEATDURATION
#define XMLFORMAT_TODO_ALARM_SUMMARY			XMLFORMAT_CALENDAR_ALARM_SUMMARY
#define XMLFORMAT_TODO_ALARM_TRIGGER			XMLFORMAT_CALENDAR_ALARM_TRIGGER
#define XMLFORMAT_TODO_ALARM_VALUE			XMLFORMAT_CALENDAR_ALARM_VALUE
#define XMLFORMAT_TODO_ALARM_VALUE_DATETIME		XMLFORMAT_CALENDAR_TRIGGERTYPE_DATETIME
#define XMLFORMAT_TODO_ALARM_VALUE_DURATION		XMLFORMAT_CALENDAR_TRIGGERTYPE_DURATION
#define XMLFORMAT_TODO_ALARM_RELATEDTYPE		XMLFORMAT_CALENDAR_ALARM_RELATEDTYPE
#define XMLFORMAT_TODO_ALARM_RELATEDTYPE_START		XMLFORMAT_CALENDAR_RELATEDTYPE_START
#define XMLFORMAT_TODO_ALARM_RELATEDTYPE_END		XMLFORMAT_CALENDAR_RELATEDTYPE_END
#define XMLFORMAT_TODO_ALARM_ALTERNATIVETEXTREP		XMLFORMAT_CALENDAR_ALARM_ALTERNATIVETEXTREP
#define XMLFORMAT_TODO_ALARM_LANGUAGE			XMLFORMAT_CALENDAR_ALARM_LANGUAGE
#define XMLFORMAT_TODO_ALARM_ATTACHVALUE		XMLFORMAT_CALENDAR_ALARM_ATTACHVALUE
#define XMLFORMAT_TODO_ALARM_ATTACHVALUE_URI		XMLFORMAT_CALENDAR_ATTACHMENTTYPE_URI
#define XMLFORMAT_TODO_ALARM_ATTACHVALUE_BINARY		XMLFORMAT_CALENDAR_ATTACHMENTTYPE_BINARY
#define XMLFORMAT_TODO_ALARM_FORMATTYPE			XMLFORMAT_CALENDAR_ALARM_FORMATTYPE
#define XMLFORMAT_TODO_ALARM_ENCODING			XMLFORMAT_CALENDAR_ALARM_ENCODING

// Attach
#define XMLFORMAT_TODO_ATTACH				"Attach"
#define XMLFORMAT_TODO_ATTACH_CONTENT			XMLFORMAT_CALENDAR_ATTACHMENT_CONTENT
#define XMLFORMAT_TODO_ATTACH_VALUE			XMLFORMAT_CALENDAR_ATTACHMENT_VALUE
#define XMLFORMAT_TODO_ATTACH_VALUE_URI			XMLFORMAT_CALENDAR_ATTACHMENTTYPE_URI	
#define XMLFORMAT_TODO_ATTACH_VALUE_BINARY		XMLFORMAT_CALENDAR_ATTACHMENTTYPE_BINARY
#define XMLFORMAT_TODO_ATTACH_FORMATTYPE		XMLFORMAT_CALENDAR_ATTACHMENT_FORMATTYPE
#define XMLFORMAT_TODO_ATTACH_ENCODING			XMLFORMAT_CALENDAR_ATTACHMENT_ENCODING

// Attendee
#define XMLFORMAT_TODO_ATTENDEE				"Attendee"
#define XMLFORMAT_NOTE_ATTENDEE_CONTENT			XMLFORMAT_CALENDAR_ATTENDEE_CONTENT
#define XMLFORMAT_NOTE_ATTENDEE_CUTYPE			XMLFORMAT_CALENDAR_ATTENDEE_CUTYPE
#define XMLFORMAT_NOTE_ATTENDEE_CUTYPE_INDIVIDUAL	XMLFORMAT_CALENDAR_CALENDARUSERTYPE_INDIVIDUAL
#define XMLFORMAT_NOTE_ATTENDEE_CUTYPE_GROUP		XMLFORMAT_CALENDAR_CALENDARUSERTYPE_GROUP
#define XMLFORMAT_NOTE_ATTENDEE_CUTYPE_RESOURCE		XMLFORMAT_CALENDAR_CALENDARUSERTYPE_RESOURCE
#define XMLFORMAT_NOTE_ATTENDEE_CUTYPE_ROOM		XMLFORMAT_CALENDAR_CALENDARUSERTYPE_ROOM	
#define XMLFORMAT_NOTE_ATTENDEE_CUTYPE_UNKNOWN		XMLFORMAT_CALENDAR_CALENDARUSERTYPE_UNKNOWN
#define XMLFORMAT_NOTE_ATTENDEE_MEMBER			XMLFORMAT_CALENDAR_ATTENDEE_MEMBER
#define XMLFORMAT_NOTE_ATTENDEE_ROLE			XMLFORMAT_CALENDAR_ATTENDEE_ROLE
#define XMLFORMAT_NOTE_ATTENDEE_ROLE_NEEDSACTION	XMLFORMAT_CALENDAR_ROLETYPE_NEEDSACTION
#define XMLFORMAT_NOTE_ATTENDEE_ROLE_ACCEPTED		XMLFORMAT_CALENDAR_ROLETYPE_ACCEPTED
#define XMLFORMAT_NOTE_ATTENDEE_ROLE_DECLINED		XMLFORMAT_CALENDAR_ROLETYPE_DECLINED
#define XMLFORMAT_NOTE_ATTENDEE_ROLE_TENTATIVE		XMLFORMAT_CALENDAR_ROLETYPE_TENTATIVE
#define XMLFORMAT_NOTE_ATTENDEE_ROLE_DELEGATED		XMLFORMAT_CALENDAR_ROLETYPE_DELEGATED
#define XMLFORMAT_NOTE_ATTENDEE_PARTSTAT		XMLFORMAT_CALENDAR_ATTENDEE_PARTSTAT
#define XMLFORMAT_NOTE_ATTENDEE_RSVP			XMLFORMAT_CALENDAR_ATTENDEE_RSVP
#define XMLFORMAT_NOTE_ATTENDEE_DELEGATEDFROM		XMLFORMAT_CALENDAR_ATTENDEE_DELEGATEDFROM
#define XMLFORMAT_NOTE_ATTENDEE_SENTBY			XMLFORMAT_CALENDAR_ATTENDEE_SENTBY
#define XMLFORMAT_NOTE_ATTENDEE_COMMONNAME		XMLFORMAT_CALENDAR_ATTENDEE_COMMONNAME
#define XMLFORMAT_NOTE_ATTENDEE_DIRECTORY		XMLFORMAT_CALENDAR_ATTENDEE_DIRECTORY
#define XMLFORMAT_NOTE_ATTENDEE_LANGUAGE		XMLFORMAT_CALENDAR_ATTENDEE_LANGUAGE

// CalendarScale
#define XMLFORMAT_TODO_CALENDARSCALE			"CalendarScale"
#define XMLFORMAT_TODO_CALENDARSCALE_CONTENT		XMLFORMAT_COMMON_STRINGCONTENT_CONTENT

// Categories
#define XMLFORMAT_TODO_CATEGORIES			"Categories"
#define XMLFORMAT_TODO_CATEGORIES_CATEGORY		XMLFORMAT_COMMON_CATEGORIES_CATEGORY

// Class
#define XMLFORMAT_TODO_CLASS				"Class"
#define XMLFORMAT_TODO_CLASS_CONTENT			XMLFORMAT_COMMON_CLASS_CONTENT
#define XMLFORMAT_TODO_CLASS_CONTENT_PUBLIC		XMLFORMAT_COMMON_CLASS_CONTENT_PUBLIC
#define XMLFORMAT_TODO_CLASS_CONTENT_PRIVATE		XMLFORMAT_COMMON_CLASS_CONTENT_PRIVATE
#define XMLFORMAT_TODO_CLASS_CONTENT_CONFIDENTIAL	XMLFORMAT_COMMON_CLASS_CONTENT_CONFIDENTIAL

// Comment
#define XMLFORMAT_TODO_COMMENT				"Comment"
#define XMLFORMAT_TODO_COMMENT_CONTENT			XMLFORMAT_COMMON_MULTITEXT_CONTENT
#define XMLFORMAT_TODO_COMMENT_ALTERNATIVETEXTREP	XMLFORMAT_COMMON_MULTITEXT_ALTERNATIVETEXTREP
#define XMLFORMAT_TODO_COMMENT_LANGUAGE			XMLFORMAT_COMMON_MULTITEXT_LANGUAGE

// Completed
#define XMLFORMAT_TODO_COMPLETED			"Completed"
#define XMLFORMAT_TODO_COMPLETED_CONTENT		XMLFORMAT_COMMON_DATETIMECONTENT_CONTENT
#define XMLFORMAT_TODO_COMPLETED_VALUE			XMLFORMAT_COMMON_DATETIMECONTENT_VALUE
#define XMLFORMAT_TODO_COMPLETED_VALUE_DATE		XMLFORMAT_COMMON_DATEVALUETYPE_DATE
#define XMLFORMAT_TODO_COMPLETED_VALUE_DATETIME		XMLFORMAT_COMMON_DATEVALUETYPE_DATETIME
#define XMLFORMAT_TODO_COMPLETED_TIMEZONEID		XMLFORMAT_COMMON_DATETIMECONTENT_TIMEZONEID

// Contact
#define XMLFORMAT_TODO_CONTACT				"Contact"
#define XMLFORMAT_TODO_CONTACT_CONTENT			XMLFORMAT_COMMON_MULTITEXT_CONTENT
#define XMLFORMAT_TODO_CONTACT_ALTERNATIVETEXTREP	XMLFORMAT_COMMON_MULTITEXT_ALTERNATIVETEXTREP
#define XMLFORMAT_TODO_CONTACT_LANGUAGE			XMLFORMAT_COMMON_MULTITEXT_LANGUAGE

// Created
#define XMLFORMAT_TODO_CREATED				"Created"
#define XMLFORMAT_TODO_CREATED_CONTENT			XMLFORMAT_COMMON_DATETIMECONTENT_CONTENT
#define XMLFORMAT_TODO_CREATED_VALUE			XMLFORMAT_COMMON_DATETIMECONTENT_VALUE
#define XMLFORMAT_TODO_CREATED_VALUE_DATE		XMLFORMAT_COMMON_DATEVALUETYPE_DATE
#define XMLFORMAT_TODO_CREATED_VALUE_DATETIME		XMLFORMAT_COMMON_DATEVALUETYPE_DATETIME
#define XMLFORMAT_TODO_CREATED_TIMEZONEID		XMLFORMAT_COMMON_DATETIMECONTENT_TIMEZONEID

// DateCalendarCreated
#define XMLFORMAT_TODO_DATECALENDARCREATED		"DateCalendarCreated"
#define XMLFORMAT_TODO_DATECALENDARCREATED_CONTENT	XMLFORMAT_COMMON_DATETIMECONTENT_CONTENT
#define XMLFORMAT_TODO_DATECALENDARCREATED_VALUE	XMLFORMAT_COMMON_DATETIMECONTENT_VALUE
#define XMLFORMAT_TODO_DATECALENDARCREATED_VALUE_DATE	XMLFORMAT_COMMON_DATEVALUETYPE_DATE
#define XMLFORMAT_TODO_DATECALENDARCREATED_VALUE_DATETIME	XMLFORMAT_COMMON_DATEVALUETYPE_DATETIME
#define XMLFORMAT_TODO_DATECALENDARCREATED_TIMEZONEID	XMLFORMAT_COMMON_DATETIMECONTENT_TIMEZONEID

// DateStarted
#define XMLFORMAT_TODO_DATESTARTED			"DateStarted"
#define XMLFORMAT_TODO_DATESTARTED_CONTENT		XMLFORMAT_COMMON_DATETIMECONTENT_CONTENT
#define XMLFORMAT_TODO_DATESTARTED_VALUE		XMLFORMAT_COMMON_DATETIMECONTENT_VALUE
#define XMLFORMAT_TODO_DATESTARTED_VALUE_DATE		XMLFORMAT_COMMON_DATEVALUETYPE_DATE
#define XMLFORMAT_TODO_DATESTARTED_VALUE_DATETIME	XMLFORMAT_COMMON_DATEVALUETYPE_DATETIME
#define XMLFORMAT_TODO_DATESTARTED_TIMEZONEID		XMLFORMAT_COMMON_DATETIMECONTENT_TIMEZONEID

// Description
#define XMLFORMAT_TODO_DESCRIPTION			"Description"
#define XMLFORMAT_TODO_DESCRIPTION_CONTENT		XMLFORMAT_COMMON_MULTITEXT_CONTENT
#define XMLFORMAT_TODO_DESCRIPTION_ALTERNATIVETEXTREP	XMLFORMAT_COMMON_MULTITEXT_ALTERNATIVETEXTREP
#define XMLFORMAT_TODO_DESCRIPTION_LANGUAGE		XMLFORMAT_COMMON_MULTITEXT_LANGUAGE

// Due
#define XMLFORMAT_TODO_DUE				"Due"
#define XMLFORMAT_TODO_DUE_CONTENT			XMLFORMAT_COMMON_DATETIMECONTENT_CONTENT
#define XMLFORMAT_TODO_DUE_VALUE			XMLFORMAT_COMMON_DATETIMECONTENT_VALUE
#define XMLFORMAT_TODO_DUE_VALUE_DATE			XMLFORMAT_COMMON_DATEVALUETYPE_DATE
#define XMLFORMAT_TODO_DUE_VALUE_DATETIME		XMLFORMAT_COMMON_DATEVALUETYPE_DATETIME
#define XMLFORMAT_TODO_DUE_TIMEZONEID			XMLFORMAT_COMMON_DATETIMECONTENT_TIMEZONEID

// Duration
#define XMLFORMAT_TODO_DURATION				"Duration"
#define XMLFORMAT_TODO_DURATION_INADVANCE		XMLFORMAT_CALENDAR_DURATION_INADVANCE
#define XMLFORMAT_TODO_DURATION_WEEKS			XMLFORMAT_CALENDAR_DURATION_WEEKS
#define XMLFORMAT_TODO_DURATION_DAYS			XMLFORMAT_CALENDAR_DURATION_DAYS
#define XMLFORMAT_TODO_DURATION_HOURS			XMLFORMAT_CALENDAR_DURATION_HOURS
#define XMLFORMAT_TODO_DURATION_MINUTES			XMLFORMAT_CALENDAR_DURATION_MINUTES
#define XMLFORMAT_TODO_DURATION_SECONDS			XMLFORMAT_CALENDAR_DURATION_SECONDS

// ExceptionDateTime
#define XMLFORMAT_TODO_EXCEPTIONDATETIME		"ExceptionDateTime"
#define XMLFORMAT_TODO_EXCEPTIONDATETIME_CONTENT	XMLFORMAT_COMMON_DATETIMECONTENT_CONTENT
#define XMLFORMAT_TODO_EXCEPTIONDATETIME_VALUE		XMLFORMAT_COMMON_DATETIMECONTENT_VALUE
#define XMLFORMAT_TODO_EXCEPTIONDATETIME_VALUE_DATE	XMLFORMAT_COMMON_DATEVALUETYPE_DATE
#define XMLFORMAT_TODO_EXCEPTIONDATETIME_VALUE_DATETIME	XMLFORMAT_COMMON_DATEVALUETYPE_DATETIME
#define XMLFORMAT_TODO_EXCEPTIONDATETIME_TIMEZONEID	XMLFORMAT_COMMON_DATETIMECONTENT_TIMEZONEID

// ExceptionRule
#define XMLFORMAT_TODO_EXCEPTIONRULE			"ExceptionRule"
#define XMLFORMAT_TODO_EXCEPTIONRULE_FREQUENCY		XMLFORMAT_CALENDAR_RECURRENCERULE_FREQUENCY
#define XMLFORMAT_TODO_EXCEPTIONRULE_FREQUENCY_DAILY	XMLFORMAT_CALENDAR_BASICRECURRENCEFREQ_DAILY
#define XMLFORMAT_TODO_EXCEPTIONRULE_FREQUENCY_WEEKLY	XMLFORMAT_CALENDAR_BASICRECURRENCEFREQ_WEEKLY
#define XMLFORMAT_TODO_EXCEPTIONRULE_FREQUENCY_MONTHLY	XMLFORMAT_CALENDAR_BASICRECURRENCEFREQ_MONTHLY
#define XMLFORMAT_TODO_EXCEPTIONRULE_FREQUENCY_YEARLY	XMLFORMAT_CALENDAR_BASICRECURRENCEFREQ_YEARLY
#define XMLFORMAT_TODO_EXCEPTIONRULE_UNTIL		XMLFORMAT_CALENDAR_RECURRENCERULE_UNTIL
#define XMLFORMAT_TODO_EXCEPTIONRULE_COUNT		XMLFORMAT_CALENDAR_RECURRENCERULE_COUNT
#define XMLFORMAT_TODO_EXCEPTIONRULE_INTERVAL		XMLFORMAT_CALENDAR_RECURRENCERULE_INTERVAL
#define XMLFORMAT_TODO_EXCEPTIONRULE_BYDAY		XMLFORMAT_CALENDAR_RECURRENCERULE_BYDAY
#define XMLFORMAT_TODO_EXCEPTIONRULE_BYMONTHDAY		XMLFORMAT_CALENDAR_RECURRENCERULE_BYMONTHDAY
#define XMLFORMAT_TODO_EXCEPTIONRULE_BYYEARDAY		XMLFORMAT_CALENDAR_RECURRENCERULE_BYYEARDAY
#define XMLFORMAT_TODO_EXCEPTIONRULE_BYMONTH		XMLFORMAT_CALENDAR_RECURRENCERULE_BYMONTH
#define XMLFORMAT_TODO_EXCEPTIONRULE_TIMEZONEID		XMLFORMAT_CALENDAR_RECURRENCERULE_TIMEZONEID
#define XMLFORMAT_TODO_EXCEPTIONRULE_TZCOMPONENT	XMLFORMAT_CALENDAR_RECURRENCERULE_TZCOMPONENT

// Geo //TODO where is GeoValue defined?
#define XMLFORMAT_TODO_GEO				"Geo"

// LastModified
#define XMLFORMAT_TODO_LASTMODIFIED			"LastModified"
#define XMLFORMAT_TODO_LASTMODIFIED_CONTENT		XMLFORMAT_COMMON_DATETIMECONTENT_CONTENT
#define XMLFORMAT_TODO_LASTMODIFIED_VALUE		XMLFORMAT_COMMON_DATETIMECONTENT_VALUE
#define XMLFORMAT_TODO_LASTMODIFIED_TIMEZONEID		XMLFORMAT_COMMON_DATETIMECONTENT_TIMEZONEID

// Location
#define XMLFORMAT_TODO_LOCATION				"Location"
#define XMLFORMAT_TODO_LOCATION_CONTENT			XMLFORMAT_COMMON_MULTITEXT_CONTENT
#define XMLFORMAT_TODO_LOCATION_ALTERNATIVETEXTREP	XMLFORMAT_COMMON_MULTITEXT_ALTERNATIVETEXTREP
#define XMLFORMAT_TODO_LOCATION_LANGUAGE		XMLFORMAT_COMMON_MULTITEXT_LANGUAGE

// Method
#define XMLFORMAT_TODO_METHOD				"Method"
#define XMLFORMAT_TODO_METHOD_CONTENT			XMLFORMAT_COMMON_STRINGCONTENT_CONTENT

// Organizer
#define XMLFORMAT_TODO_ORGANIZER			"Organizer"
#define XMLFORMAT_TODO_ORGANIZER_CONTENT		XMLFORMAT_CALENDAR_ORGANIZER_CONTENT
#define XMLFORMAT_TODO_ORGANIZER_COMMONNAME		XMLFORMAT_CALENDAR_ORGANIZER_COMMONNAME	
#define XMLFORMAT_TODO_ORGANIZER_DIRECTORY		XMLFORMAT_CALENDAR_ORGANIZER_DIRECTORY
#define XMLFORMAT_TODO_ORGANIZER_SENTBY			XMLFORMAT_CALENDAR_ORGANIZER_SENTBY
#define XMLFORMAT_TODO_ORGANIZER_LANGUAGE		XMLFORMAT_CALENDAR_ORGANIZER_LANGUAGE

// PercentComplete
#define XMLFORMAT_TODO_PERCENTCOMPLETE			"PercentComplete"
#define XMLFORMAT_TODO_PERCENTCOMPLETE_CONTENT		"Content"
#define XMLFORMAT_TODO_PERCENTCOMPLETE_CONTENT_MAX	"100"
#define XMLFORMAT_TODO_PERCENTCOMPLETE_CONTENT_MIN	"0"

// Priority
#define XMLFORMAT_TODO_PRIORITY				"Priority"
#define XMLFORMAT_TODO_PRIORITY_CONTENT			XMLFORMAT_CALENDAR_PRIORITY_CONTENT
#define XMLFORMAT_TODO_PRIORITY_CONTENT_MAX		XMLFORMAT_CALENDAR_PRIORITY_CONTENT_MAX
#define XMLFORMAT_TODO_PRIORITY_CONTENT_MIN		XMLFORMAT_CALENDAR_PRIORITY_CONTENT_MIN

// ProductID
#define XMLFORMAT_TODO_PRODUCTID			"ProductID"
#define XMLFORMAT_TODO_PRODUCTID_CONTENT		XMLFORMAT_COMMON_STRINGCONTENT_CONTENT

// RecurrenceDateTime
#define XMLFORMAT_TODO_RECURRENCEDATETIME		"RecurrenceDateTime"
#define XMLFORMAT_TODO_RECURRENCEDATETIME_CONTENT	 XMLFORMAT_CALENDAR_RECURRENCEDATETIMELIST_CONTENT
#define XMLFORMAT_TODO_RECURRENCEDATETIME_TIMEZONEID	XMLFORMAT_CALENDAR_RECURRENCEDATETIMELIST_TIMEZONEID
#define XMLFORMAT_TODO_RECURRENCEDATETIME_VALUE		XMLFORMAT_CALENDAR_RECURRENCEDATETIMELIST_VALUE				// Attribute type RecurrenceDateTimeList
#define XMLFORMAT_TODO_RECURRENCEDATETIME_VALUE_DATETIME	XMLFORMAT_CALENDAR_RECURRENCEDATETIMELIST_VALUE_DATETIME
#define XMLFORMAT_TODO_RECURRENCEDATETIME_VALUE_DATE	XMLFORMAT_CALENDAR_RECURRENCEDATETIMELIST_VALUE_DATE
#define XMLFORMAT_TODO_RECURRENCEDATETIME_VALUE_PERIOD	XMLFORMAT_CALENDAR_RECURRENCEDATETIMELIST_VALUE_PERIOD

// RecurrenceId
#define XMLFORMAT_TODO_RECURRENCEID			"RecurrenceId"
#define XMLFORMAT_TODO_RECURRENCEID_CONTENT		XMLFORMAT_CALENDAR_RECURRENCEID_CONTENT	
#define XMLFORMAT_TODO_RECURRENCEID_TIMEZONEID		XMLFORMAT_CALENDAR_RECURRENCEID_TIMEZONEID
#define XMLFORMAT_TODO_RECURRENCEID_RANGE		XMLFORMAT_CALENDAR_RECURRENCEID_RANGE
#define XMLFORMAT_TODO_RECURRENCEID_RANGE_THISANDPRIOR	XMLFORMAT_CALENDAR_RECURRENCEID_RANGE_THISANDPRIOR
#define XMLFORMAT_TODO_RECURRENCEID_RANGE_THISANDFUTURE	XMLFORMAT_CALENDAR_RECURRENCEID_RANGE_THISANDFUTURE
#define XMLFORMAT_TODO_RECURRENCEID_VALUE		XMLFORMAT_CALENDAR_RECURRENCEID_VALUE				
#define XMLFORMAT_TODO_RECURRENCEID_VALUE_DATE		XMLFORMAT_COMMON_DATEVALUETYPE_DATE
#define XMLFORMAT_TODO_RECURRENCEID_VALUE_DATETIME	XMLFORMAT_COMMON_DATEVALUETYPE_DATETIME

// Related
#define XMLFORMAT_TODO_RELATED				"Related"
#define XMLFORMAT_TODO_RELATED_CONTENT			XMLFORMAT_CALENDAR_RELATEDTO_CONTENT
#define XMLFORMAT_TODO_RELATED_TIMEZONEID		XMLFORMAT_CALENDAR_RELATEDTO_TIMEZONEID	
#define XMLFORMAT_TODO_RELATED_RELATIONSHIPTYPE		XMLFORMAT_CALENDAR_RELATEDTO_RELATIONSHIPTYPE
#define XMLFORMAT_TODO_RELATED_RELATIONSHIPTYPE_PARENT	XMLFORMAT_CALENDAR_RELATIONSHIPTYPE_PARENT
#define XMLFORMAT_TODO_RELATED_RELATIONSHIPTYPE_CHILD	XMLFORMAT_CALENDAR_RELATIONSHIPTYPE_CHILD
#define XMLFORMAT_TODO_RELATED_RELATIONSHIPTYPE_SIBLING	XMLFORMAT_CALENDAR_RELATIONSHIPTYPE_SIBLING

// Resources
#define XMLFORMAT_TODO_RESOURCES			"Resources"
#define XMLFORMAT_TODO_RESOURCES_CONTENT		XMLFORMAT_COMMON_MULTITEXT_CONTENT
#define XMLFORMAT_TODO_RESOURCES_ALTERNATIVETEXTREP	XMLFORMAT_COMMON_MULTITEXT_ALTERNATIVETEXTREP
#define XMLFORMAT_TODO_RESOURCES_LANGUAGE		XMLFORMAT_COMMON_MULTITEXT_LANGUAGE

// RecurrenceRule
#define XMLFORMAT_TODO_RECURRENCERULE			"RecurrenceRule"
#define XMLFORMAT_TODO_RECURRENCERULE_FREQUENCY		XMLFORMAT_CALENDAR_RECURRENCERULE_FREQUENCY
#define XMLFORMAT_TODO_RECURRENCERULE_FREQUENCY_DAILY	XMLFORMAT_CALENDAR_BASICRECURRENCEFREQ_DAILY
#define XMLFORMAT_TODO_RECURRENCERULE_FREQUENCY_WEEKLY	XMLFORMAT_CALENDAR_BASICRECURRENCEFREQ_WEEKLY
#define XMLFORMAT_TODO_RECURRENCERULE_FREQUENCY_MONTHLY	XMLFORMAT_CALENDAR_BASICRECURRENCEFREQ_MONTHLY
#define XMLFORMAT_TODO_RECURRENCERULE_FREQUENCY_YEARLY	XMLFORMAT_CALENDAR_BASICRECURRENCEFREQ_YEARLY
#define XMLFORMAT_TODO_RECURRENCERULE_UNTIL		XMLFORMAT_CALENDAR_RECURRENCERULE_UNTIL
#define XMLFORMAT_TODO_RECURRENCERULE_COUNT		XMLFORMAT_CALENDAR_RECURRENCERULE_COUNT
#define XMLFORMAT_TODO_RECURRENCERULE_INTERVAL		XMLFORMAT_CALENDAR_RECURRENCERULE_INTERVAL
#define XMLFORMAT_TODO_RECURRENCERULE_BYDAY		XMLFORMAT_CALENDAR_RECURRENCERULE_BYDAY
#define XMLFORMAT_TODO_RECURRENCERULE_BYMONTHDAY	XMLFORMAT_CALENDAR_RECURRENCERULE_BYMONTHDAY
#define XMLFORMAT_TODO_RECURRENCERULE_BYYEARDAY		XMLFORMAT_CALENDAR_RECURRENCERULE_BYYEARDAY
#define XMLFORMAT_TODO_RECURRENCERULE_BYMONTH		XMLFORMAT_CALENDAR_RECURRENCERULE_BYMONTH
#define XMLFORMAT_TODO_RECURRENCERULE_TIMEZONEID	XMLFORMAT_CALENDAR_RECURRENCERULE_TIMEZONEID
#define XMLFORMAT_TODO_RECURRENCERULE_TZCOMPONENT	XMLFORMAT_CALENDAR_RECURRENCERULE_TZCOMPONENT

// RStatus
#define XMLFORMAT_TODO_RSTATUS				"RStatus"
#define XMLFORMAT_TODO_RSTATUS_STATUSCODE		XMLFORMAT_CALENDAR_REQUESTSTATUS_STATUSCODE
#define XMLFORMAT_TODO_RSTATUS_STATUSDESCRIPTION	XMLFORMAT_CALENDAR_REQUESTSTATUS_STATUSDESCRIPTION
#define XMLFORMAT_TODO_RSTATUS_EXCEPTIONDATA		XMLFORMAT_CALENDAR_REQUESTSTATUS_EXCEPTIONDATA
#define XMLFORMAT_TODO_RSTATUS_TIMEZONEID		XMLFORMAT_CALENDAR_REQUESTSTATUS_TIMEZONEID
#define XMLFORMAT_TODO_RSTATUS_RELATIONSHIPTYPE		XMLFORMAT_CALENDAR_REQUESTSTATUS_RELATIONSHIPTYPE
#define XMLFORMAT_TODO_RSTATUS_RELATIONSHIPTYPE_PARENT	XMLFORMAT_CALENDAR_RELATIONSHIPTYPE_PARENT
#define XMLFORMAT_TODO_RSTATUS_RELATIONSHIPTYPE_CHILD	XMLFORMAT_CALENDAR_RELATIONSHIPTYPE_CHILD
#define XMLFORMAT_TODO_RSTATUS_RELATIONSHIPTYPE_SIBLING	XMLFORMAT_CALENDAR_RELATIONSHIPTYPE_SIBLING

// Sequence
#define XMLFORMAT_TODO_SEQUENCE				"Sequence"
#define XMLFORMAT_TODO_SEQUENCE_CONTENT			XMLFORMAT_CALENDAR_INTEGERCONTENT_CONTENT

// Status
#define XMLFORMAT_TODO_STATUS				"Status"
#define XMLFORMAT_TODO_STATUS_NEEDSACTION		"NEEDS-ACTION"
#define XMLFORMAT_TODO_STATUS_COMPLETED			"COMPLETED"
#define XMLFORMAT_TODO_STATUS_INPROCESS			"IN-PROCESS"
#define XMLFORMAT_TODO_STATUS_CANCELLED			"CANCELLED"

// Summary
#define XMLFORMAT_TODO_SUMMARY				"Summary"
#define XMLFORMAT_TODO_SUMMARY_CONTENT			XMLFORMAT_COMMON_MULTITEXT_CONTENT
#define XMLFORMAT_TODO_SUMMARY_ALTERNATIVETEXTREP	XMLFORMAT_COMMON_MULTITEXT_ALTERNATIVETEXTREP
#define XMLFORMAT_TODO_SUMMARY_LANGUAGE			XMLFORMAT_COMMON_MULTITEXT_LANGUAGE

// Uid
#define XMLFORMAT_TODO_UID				"Uid"
#define XMLFORMAT_TODO_UID_CONTENT			XMLFORMAT_COMMON_STRINGCONTENT_CONTENT

// Url
#define XMLFORMAT_TODO_URL				"Url"
#define XMLFORMAT_TODO_URL_CONTENT			XMLFORMAT_CALENDAR_URLCONTENT_CONTENT

// Version
#define XMLFORMAT_TODO_Version				"Version"
#define XMLFORMAT_TODO_Version_CONTENT			XMLFORMAT_COMMON_STRINGCONTENT_CONTENT

#endif /* XMLFORMAT_TODO_H */
