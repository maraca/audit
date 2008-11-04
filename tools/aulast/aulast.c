/*
 * aulast.c - A last program based on audit logs 
 * Copyright (c) 2008 Red Hat Inc., Durham, North Carolina.
 * All Rights Reserved.
 *
 * This software may be freely redistributed and/or modified under the
 * terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Authors:
 *   Steve Grubb <sgrubb@redhat.com>
 */

#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <sys/utsname.h>
#include "libaudit.h"
#include "auparse.h"
#include "aulast-llist.h"


static	llist l;

/* command line params */
static int cuid = -1, bad = 0;
static char *cterm = NULL;

void usage(void)
{
	fprintf(stderr, "usage: aulast [-f file] [user name] [tty]\n");
}

/* This outputs a line of text reporting the login/out times */
static void report_session(lnode* cur)
{
	int notime = 0;

	// Don't list failed logins
	if (cur == NULL)
		return;

	if (cur->result != bad)
		return;

	if (cur->name) {
		// This is a reboot record
		printf("%-8.8s ", cur->name);
		if (cur->end == 0) {
			cur->end = time(NULL);
			notime = 1;
		}
	} else {
		struct passwd *p = getpwuid(cur->auid);
		printf("%-8.8s ", p->pw_name);
	}
	if (strncmp("/dev/", cur->term, 5) == 0)
		printf("%-12.12s ", cur->term+5);
	else
		printf("%-12.12s ", cur->term);
	printf("%-16.16s ", cur->host ? cur->host : "?");
	printf("%-16.16s ", ctime(&cur->start));
	if (cur->end > 0) {
		time_t secs;
		int mins, hours, days;
		if (notime)
			printf("- %-7.5s", " ");
		else
			printf("- %-7.5s", ctime(&cur->end) + 11);
		secs = cur->end - cur->start;
		mins  = (secs / 60) % 60;
		hours = (secs / 3600) % 24;
		days  = secs / 86400;
		if (days)
			printf("(%d+%02d:%02d)\n", days, hours, mins);
		else
			printf("(%02d:%02d)\n", hours, mins);
	} else {
		switch(cur->status)
		{
			case SESSION_START:
				printf("  still logged in\n");
				break;
			case DOWN:
				printf("- down\n");
				break;
			case CRASH:
				printf("- crash\n");
				break;
			case GONE:
				printf("  gone - no logout\n");
				break;
			default:
				printf("\n");
				break;
		}
	}
}

static void create_new_session(auparse_state_t *au)
{
	const char *tpid, *tses, *tauid;
	int pid = -1, auid = -1, ses = -1;
	lnode *cur;

	// Get pid
	tpid = auparse_find_field(au, "pid");
	if (tpid)
		pid = auparse_get_field_int(au);

	// Get second auid field
	auparse_find_field(au, "auid");
	auparse_next_field(au);
	tauid = auparse_find_field(au, "auid");
	if (tauid)
		auid = auparse_get_field_int(au);

	// Get second ses field
	auparse_find_field(au, "ses"); 
	auparse_next_field(au);
	tses = auparse_find_field(au, "ses");
	if (tses)
		ses = auparse_get_field_int(au);

	// Check that they are valid
	if (pid == -1 || auid ==-1 || ses == -1)
		return;

	// See if this session is already open
	cur = list_find_auid(&l, auid, pid, ses);
	if (cur) {
		// This means we have an open session close it out
		cur->status = GONE;
		report_session(cur);
		list_delete_cur(&l);
	}

	// If this is supposed to be limited to a specific
	// uid and we don't have that record, skip creating it
	if (cuid != -1 && cuid != auid)
		return;

	list_create_session(&l, auid, pid, ses);
}

static void update_session_login(auparse_state_t *au)
{
	const char *tpid, *tses, *tuid, *tacct, *host, *term, *tres;
	int pid = -1, uid = -1, ses = -1, result = -1;
	time_t start;
	lnode *cur;

	// Get pid
	tpid = auparse_find_field(au, "pid");
	if (tpid)
		pid = auparse_get_field_int(au);

	// Get ses field
	tses = auparse_find_field(au, "ses");
	if (tses)
		ses = auparse_get_field_int(au);

	// Get second uid field - we should be positioned past the first one
	tuid = auparse_find_field(au, "uid");
	if (tuid)
		uid = auparse_get_field_int(au);
	else
		auparse_first_record(au);

	start = auparse_get_time(au);

	host = auparse_find_field(au, "hostname");
	if (host && strcmp(host, "?") == 0)
		host = auparse_find_field(au, "addr");

	term = auparse_find_field(au, "terminal");
	tres = auparse_find_field(au, "res");
	if (tres)
		tres = auparse_interpret_field(au);
	if (tres) {
		if (strcmp(tres, "success") == 0)
			result = 0;
		else
			result = 1;
	}
	// We only get tacct when its a bad login
	if (result == 1) {
		auparse_first_record(au);
		tacct = auparse_find_field(au, "acct");
		if (tacct)
			tacct = auparse_interpret_field(au);
	} else {
		// Check that they are valid
		if (pid == -1 || uid ==-1 || ses == -1) 
			return;
	}

	// See if this session is already open
	if (result == 0)
		cur = list_find_auid(&l, uid, pid, ses);
	else
		cur = NULL;
	if (cur) {
		// If we are limited to a specific terminal and
		// we find out the session is not associated with
		// the terminal of interest, delete the current node
		if (cterm && strstr(term, cterm) == NULL) {
			list_delete_cur(&l);
			return;
		}

		// This means we have an open session - update it
		list_update_start(&l, start, host, term, result);

		// If the results were failed, we can close it out
		if (result) {
			report_session(cur);
			list_delete_cur(&l);
		} 
	} else if (bad == 1 && result == 1) {
		// If it were a bad login and we are wanting bad logins
		// create the record and report it.
		lnode n;

		n.auid = uid;
		n.start = start;
		n.end = start;
		n.name = tacct;
		n.host = host;
		n.term = term;
		n.result = result;
		n.status = SESSION_START;
		report_session(&n); 
	}
}

static void update_session_logout(auparse_state_t *au)
{
	const char *tses, *tauid, *tpid;
	int pid = -1, auid = -1, ses = -1;
	lnode *cur;

	// Get pid field
	tpid = auparse_find_field(au, "pid");
	if (tpid)
		pid = auparse_get_field_int(au);

	// Get auid field
	tauid = auparse_find_field(au, "auid");
	if (tauid)
		auid = auparse_get_field_int(au);

	// Get ses field
	tses = auparse_find_field(au, "ses");
	if (tses)
		ses = auparse_get_field_int(au);

	// Check that they are valid
	if (pid == -1 || auid ==-1 || ses == -1)
		return;

	// See if this session is already open
	cur = list_find_auid(&l, auid, pid, ses);
	if (cur) {
		// if time never got updated, this must be a cron or su 
		// session...so we will just delete it.
		if (cur->start) {
			// This means we have an open session close it out
			time_t end = auparse_get_time(au);
			list_update_logout(&l, end);
			report_session(cur);
		}
		list_delete_cur(&l);
	}
}

static void process_bootup(auparse_state_t *au)
{
	lnode *cur;
	int start;
	struct utsname ubuf;

	// See if we have unclosed boot up and make into CRASH record
	list_first(&l);
	cur = list_get_cur(&l);
	while(cur) {
		if (cur->name) {
			cur->status = CRASH;
			report_session(cur);
		}
		cur = list_next(&l);
	}

	// Logout and process anyone still left in the machine
	list_first(&l);
	cur = list_get_cur(&l);
	while(cur) {
		if (cur->status != CRASH) {
			cur->status = DOWN;
			report_session(cur);
		}
		cur = list_next(&l);
	}
	list_clear(&l);
	list_create(&l);

	// make reboot record - user:reboot, tty:system boot, host: uname -r 
	uname(&ubuf);
	start = auparse_get_time(au);
	list_create_session(&l, 0, 0, 0);
	cur = list_get_cur(&l);
	cur->start = start;
	cur->name = strdup("reboot");
	cur->term = strdup("system boot");
	cur->host = strdup(ubuf.release);
	cur->result = 0;
}

static void process_shutdown(auparse_state_t *au)
{
	lnode *cur;

	// Find reboot record
	list_first(&l);
	cur = list_get_cur(&l);
	while(cur) {
		if (cur->name) {
			// Found it - close it out and display it
			time_t end = auparse_get_time(au);
			list_update_logout(&l, end);
			report_session(cur);
			list_delete_cur(&l);
			return;
		}
		cur = list_next(&l);
	}
}

int main(int argc, char *argv[])
{
	int i, use_stdin = 0;
	char *user = NULL, *file = NULL;
	struct passwd *p;
        auparse_state_t *au;

	for (i=1; i<argc; i++) {
		if (argv[i][0] != '-') {
			//take input and lookup as if it were a user name
			//if that fails assume its a tty
			if (user == NULL) {
				p = getpwnam(argv[i]);
				if (p) {
					cuid = p->pw_uid;
					user = argv[i];
					continue;
				}
			}
			if (cterm == NULL) {
				cterm = argv[i];
			} else {
				usage();
				return 1;
			}
		} else {
			if (strcmp(argv[i], "-f") == 0) {
				if (use_stdin == 0) {
					i++;
					file = argv[i];
				} else {
					fprintf(stderr,"stdin already given\n");
					return 1;
				}
			} else if (strcmp(argv[i], "--bad") == 0) {
				bad = 1;
			} else if (strcmp(argv[i], "--stdin") == 0) {
				if (file == NULL)
					use_stdin = 1;
				else {
					fprintf(stderr, "file already given\n");
					return 1;
				}
			} else {
				usage();
				return 1;
			}
		}
	}

        setlocale (LC_ALL, "");
	list_create(&l);

	// Search for successful user logins
	if (file)
		au = auparse_init(AUSOURCE_FILE, file);
	else if (use_stdin)
		au = auparse_init(AUSOURCE_FILE_POINTER, stdin);
	else
		au = auparse_init(AUSOURCE_LOGS, NULL);
	if (au == NULL) {
		printf("Error - %s\n", strerror(errno));
		goto error_exit_1;
	}

	// The theory: iterate though events
	// 1) when LOGIN is found, create a new session node
	// 2) if that session number exists, close out the old one
	// 3) when USER_LOGIN is found, update session node
	// 4) When USER_END is found update session node and close it out
	// 5) When BOOT record found make new record and check for previous
	// 6) If previous boot found, set status to crash and logout everyone
	// 7) When SHUTDOWN found, close out reboot record

	while (auparse_next_event(au) > 0) {
		// We will take advantage of the fact that all events
		// of interest are one record long
		int type = auparse_get_type(au);
		if (type < 0)
			continue;
		switch (type)
		{
			case AUDIT_LOGIN:
				create_new_session(au);
				break;
			case AUDIT_USER_LOGIN:
				update_session_login(au);
				break;
			case AUDIT_USER_END:
				update_session_logout(au);
				break;
			case AUDIT_SYSTEM_BOOT:
				process_bootup(au);
				break;
			case AUDIT_SYSTEM_SHUTDOWN:
				process_shutdown(au);
				break;
		}
	}
	auparse_destroy(au);

	// Now output the leftovers
	list_first(&l);
	do {
		lnode *cur = list_get_cur(&l);
		report_session(cur);
	} while (list_next(&l));

	list_clear(&l);
	return 0;

error_exit_1:
	list_clear(&l);
	return 1;
}

