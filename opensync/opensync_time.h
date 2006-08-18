#ifndef _OPENSYNC_FORMAT_H_
#define _OPENSYNC_FORMAT_H_

/* Timeformat helper */
char *osync_time_timestamp(const char *vtime);
char *osync_time_datestamp(const char *vtime); 
osync_bool osync_time_isdate(const char *vformat);
//char *osync_time_set_vtime(const char *vtime, const char *time, osync_bool is_utc);

/* Timetype helper */
struct tm *osync_time_vtime2tm(const char *vtime);
char *osync_time_tm2vtime(const struct tm *time, osync_bool is_utc);
time_t osync_time_vtime2unix(const char *vtime);
char *osync_time_unix2vtime(const time_t *timestamp);
time_t osync_time_tm2unix(struct tm *tmtime);
struct tm *osync_time_unix2tm(const time_t *timestamp);

/* Timezone helper */
int osync_time_timezone_diff(void);
struct tm *osync_time_tm2utc(const struct tm *ltime);
struct tm *osync_time_tm2localtime(const struct tm *utime);
char *osync_time_vtime2utc(const char* localtime);
char *osync_time_vtime2localtime(const char* utc);

#endif // _OPENSYNC_FORMAT_H_
