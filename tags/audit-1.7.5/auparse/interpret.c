/*
* interpret.c - Lookup values to something more readable
* Copyright (c) 2007,08 Red Hat Inc., Durham, North Carolina.
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

#include "config.h"
#include "nvlist.h"
#include "nvpair.h"
#include "libaudit.h"
#include "internal.h"
#include "interpret.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <linux/net.h>
#include <netdb.h>
#include <sys/un.h>
#include <linux/ax25.h>
#include <linux/atm.h>
#include <linux/x25.h>
#include <linux/if.h>   // FIXME: remove when ipx.h is fixed
#include <linux/ipx.h>
#include "gen_tables.h"

/* This is from asm/ipc.h. Copying it for now as some platforms
 * have broken headers. */
#define SEMOP            1
#define SEMGET           2
#define SEMCTL           3
#define MSGSND          11
#define MSGRCV          12
#define MSGGET          13
#define MSGCTL          14
#define SHMAT           21
#define SHMDT           22
#define SHMGET          23
#define SHMCTL          24

#include "captabs.h"
#include "clone-flagtabs.h"
#include "famtabs.h"
#include "fcntl-cmdtabs.h"
#include "flagtabs.h"
#include "ipctabs.h"
#include "open-flagtabs.h"
#include "socktabs.h"
#include "typetabs.h"

typedef enum { AVC_UNSET, AVC_DENIED, AVC_GRANTED } avc_t;
typedef enum { S_UNSET=-1, S_FAILED, S_SUCCESS } success_t;

/*
 * This function will take a pointer to a 2 byte Ascii character buffer and
 * return the actual hex value.
 */
static unsigned char x2c(unsigned char *buf)
{
        static const char AsciiArray[17] = "0123456789ABCDEF";
        char *ptr;
        unsigned char total=0;

        ptr = strchr(AsciiArray, (char)toupper(buf[0]));
        if (ptr)
                total = (unsigned char)(((ptr-AsciiArray) & 0x0F)<<4);
        ptr = strchr(AsciiArray, (char)toupper(buf[1]));
        if (ptr)
                total += (unsigned char)((ptr-AsciiArray) & 0x0F);

        return total;
}

/* returns a freshly malloc'ed and converted buffer */
const char *au_unescape(char *buf)
{
        int len, i;
        char saved, *str, *ptr = buf;

        /* Find the end of the name */
        if (*ptr == '(') {
                ptr = strchr(ptr, ')');
                if (ptr == NULL)
                        return NULL;
                else
                        ptr++;
        } else {
                while (isxdigit(*ptr))
                        ptr++;
        }
        saved = *ptr;
        *ptr = 0;
        str = strdup(buf);
        *ptr = saved;

	/* See if its '(null)' from the kernel */
        if (*buf == '(')
                return str;

        /* We can get away with this since the buffer is 2 times
         * bigger than what we are putting there.
         */
        len = strlen(str);
        if (len < 2) {
                free(str);
                return NULL;
        }
        ptr = str;
        for (i=0; i<len; i+=2) {
                *ptr = x2c((unsigned char *)&str[i]);
                ptr++;
        }
        *ptr = 0;
        return str;
}

static const char *success[3]= { "unset", "no", "yes" };
static const char *aulookup_success(int s)
{
	switch (s)
	{
		default:
			return success[0];
			break;
		case S_FAILED:
			return success[1];
			break;
		case S_SUCCESS:
			return success[2];
			break;
	}
}

static nvpair uid_nvl;
static int uid_list_created=0;
static const char *aulookup_uid(uid_t uid, char *buf, size_t size)
{
	char *name = NULL;
	int rc;

	if (uid == -1) {
		snprintf(buf, size, "unset");
		return buf;
	}

	// Check the cache first
	if (uid_list_created == 0) {
		nvpair_create(&uid_nvl);
		nvpair_clear(&uid_nvl);
		uid_list_created = 1;
	}
	rc = nvpair_find_val(&uid_nvl, uid);
	if (rc) {
		name = uid_nvl.cur->name;
	} else {
		// Add it to cache
		struct passwd *pw;
		pw = getpwuid(uid);
		if (pw) {
			nvpnode nv;
			nv.name = strdup(pw->pw_name);
			nv.val = uid;
			nvpair_append(&uid_nvl, &nv);
			name = uid_nvl.cur->name;
		}
	}
	if (name != NULL)
		snprintf(buf, size, "%s", name);
	else
		snprintf(buf, size, "unknown(%d)", uid);
	return buf;
}

void aulookup_destroy_uid_list(void)
{
	if (uid_list_created == 0)
		return;

	nvpair_clear(&uid_nvl); 
	uid_list_created = 0;
}

static nvpair gid_nvl;
static int gid_list_created=0;
static const char *aulookup_gid(gid_t gid, char *buf, size_t size)
{
	char *name = NULL;
	int rc;

	if (gid == -1) {
		snprintf(buf, size, "unset");
		return buf;
	}

	// Check the cache first
	if (gid_list_created == 0) {
		nvpair_create(&gid_nvl);
		nvpair_clear(&gid_nvl);
		gid_list_created = 1;
	}
	rc = nvpair_find_val(&gid_nvl, gid);
	if (rc) {
		name = gid_nvl.cur->name;
	} else {
		// Add it to cache
		struct group *gr;
		gr = getgrgid(gid);
		if (gr) {
			nvpnode nv;
			nv.name = strdup(gr->gr_name);
			nv.val = gid;
			nvpair_append(&gid_nvl, &nv);
			name = gid_nvl.cur->name;
		}
	}
	if (name != NULL)
		snprintf(buf, size, "%s", name);
	else
		snprintf(buf, size, "unknown(%d)", gid);
	return buf;
}

void aulookup_destroy_gid_list(void)
{
	if (gid_list_created == 0)
		return;

	nvpair_clear(&gid_nvl); 
	gid_list_created = 0;
}

static const char *print_uid(const char *val)
{
        int uid;
        char name[64];

        errno = 0;
        uid = strtoul(val, NULL, 10);
        if (errno) {
		char *out;
                asprintf(&out, "conversion error(%s)", val);
                return out;
        }

        return strdup(aulookup_uid(uid, name, sizeof(name)));
}

static const char *print_gid(const char *val)
{
        int gid;
        char name[64];

        errno = 0;
        gid = strtoul(val, NULL, 10);
        if (errno) {
		char *out;
                asprintf(&out, "conversion error(%s)", val);
                return out;
        }

        return strdup(aulookup_gid(gid, name, sizeof(name)));
}

static const char *print_arch(const char *val, int machine)
{
        const char *ptr;
	char *out;

        if (machine < 0) {
                asprintf(&out, "unknown elf type(%s)", val);
                return out;
        }
        ptr = audit_machine_to_name(machine);
	if (ptr)
	        return strdup(ptr);
	else {
                asprintf(&out, "unknown machine type(%d)", machine);
                return out;
	}
}

static const char *print_syscall(const char *val, const rnode *r)
{
        const char *sys;
	char *out;
	int machine = r->machine, syscall = r->syscall;
	unsigned long long a0 = r->a0;

        if (machine < 0)
                machine = audit_detect_machine();
        if (machine < 0) {
                out = strdup(val);
                return out;
        }
        sys = audit_syscall_to_name(syscall, machine);
        if (sys) {
                const char *func = NULL;
                if (strcmp(sys, "socketcall") == 0) {
			if ((int)a0 == a0)
				func = sock_i2s(a0);
                } else if (strcmp(sys, "ipc") == 0)
			if ((int)a0 == a0)
				func = ipc_i2s(a0);
                if (func)
                        asprintf(&out, "%s(%s)", sys, func);
                else
                        return strdup(sys);
        }
        else
                asprintf(&out, "unknown syscall(%d)", syscall);

	return out;
}

static const char *print_exit(const char *val)
{
        int ival;
	char *out;

        errno = 0;
        ival = strtol(val, NULL, 10);
        if (errno) {
                asprintf(&out, "conversion error(%s)", val);
                return out;
        }

        if (ival < 0) {
                asprintf(&out, "%d(%s)", ival, strerror(-ival));
		return out;
        }
        return strdup(val);

}

static const char *print_escaped(const char *val)
{
	const char *out;

        if (*val == '"') {
                char *term;
                val++;
                term = strchr(val, '"');
                if (term == NULL)
                        return strdup(" ");
                *term = 0;
                out = strdup(val);
		*term = '"';
		return out;
// FIXME: working here...was trying to detect (null) and handle that
// differently. The other 2 should have " around the file names.
/*      } else if (*val == '(') {
                char *term;
                val++;
                term = strchr(val, ' ');
                if (term == NULL)
                        return;
                *term = 0;
                printf("%s ", val); */
        } else if (val[0] == '0' && val[1] == '0')
                out = au_unescape((char *)&val[2]); // Abstract name
	else
                out = au_unescape((char *)val);
	if (out)
		return out;
	return strdup(val); // Something is wrong with string, just send as is
}

static const char *print_perm(const char *val)
{
        int ival, printed=0;
	char buf[32];

        errno = 0;
        ival = strtol(val, NULL, 10);
        if (errno) {
		char *out;
                asprintf(&out, "conversion error(%s)", val);
                return out;
        }

	buf[0] = 0;

        /* The kernel treats nothing (0x00) as everything (0x0F) */
        if (ival == 0)
                ival = 0x0F;
        if (ival & AUDIT_PERM_READ) {
                strcat(buf, "read");
                printed = 1;
        }
        if (ival & AUDIT_PERM_WRITE) {
                if (printed)
                        strcat(buf, ",write");
                else
                        strcat(buf, "write");
                printed = 1;
        }
        if (ival & AUDIT_PERM_EXEC) {
                if (printed)
                        strcat(buf, ",exec");
                else
                        strcat(buf, "exec");
                printed = 1;
        }
        if (ival & AUDIT_PERM_ATTR) {
                if (printed)
                        strcat(buf, ",attr");
                else
                        strcat(buf, "attr");
        }
	return strdup(buf);
}

static const char *print_mode(const char *val)
{
        unsigned int ival;
	char *out, buf[48];

        errno = 0;
        ival = strtoul(val, NULL, 8);
        if (errno) {
                asprintf(&out, "conversion error(%s)", val);
                return out;
        }

	buf[0] = 0;

        // detect tthe file type
        strcat(buf, audit_ftype_to_name(ival & S_IFMT));
	strcat(buf, ",");

        // check on special bits
        if (S_ISUID & ival)
                strcat(buf, "suid,");
        if (S_ISGID & ival)
                strcat(buf, "sgid,");
        if (S_ISVTX & ival)
                strcat(buf, "sticky,");

	// and the read, write, execute flags in octal
        asprintf(&out, "%s %03o",  buf, (S_IRWXU|S_IRWXG|S_IRWXO) & ival);
	return out;
}

static const char *print_sockaddr(const char *val)
{
        int slen;
        const struct sockaddr *saddr;
        char name[NI_MAXHOST], serv[NI_MAXSERV];
        const char *host;
        char *out;
        const char *str;

        slen = strlen(val)/2;
        host = au_unescape((char *)val);
        saddr = (struct sockaddr *)host;


        str = fam_i2s(saddr->sa_family);
        if (str == NULL) {
                asprintf(&out, "unknown family(%d)", saddr->sa_family);
		return out;
	}

	// Now print address for some families
        switch (saddr->sa_family) {
                case AF_LOCAL:
                        {
                                const struct sockaddr_un *un =
                                        (struct sockaddr_un *)saddr;
                                if (un->sun_path[0])
                                        asprintf(&out, "%s %s", str,
							un->sun_path);
                                else // abstract name
                                        asprintf(&out, "%s %.108s", str,
							&un->sun_path[1]);
                        }
                        break;
                case AF_INET:
                        if (slen < sizeof(struct sockaddr_in)) {
                                asprintf(&out, "%s sockaddr len too short",
						 str);
                                free((char *)host);
                                return out;
                        }
                        slen = sizeof(struct sockaddr_in);
                        if (getnameinfo(saddr, slen, name, NI_MAXHOST, serv,
                                NI_MAXSERV, NI_NUMERICHOST |
                                        NI_NUMERICSERV) == 0 ) {
                                asprintf(&out, "%s host:%s serv:%s", str,
						name, serv);
                        } else
                                asprintf(&out, "%s (error resolving addr)",
						 str);
                        break;
                case AF_AX25:
                        {
                                const struct sockaddr_ax25 *x =
                                                (struct sockaddr_ax25 *)saddr;
                                asprintf(&out, "%s call:%c%c%c%c%c%c%c", str,
                                        x->sax25_call.ax25_call[0],
                                        x->sax25_call.ax25_call[1],
                                        x->sax25_call.ax25_call[2],
                                        x->sax25_call.ax25_call[3],
                                        x->sax25_call.ax25_call[4],
                                        x->sax25_call.ax25_call[5],
                                        x->sax25_call.ax25_call[6]
                                );
                        }
                        break;
                case AF_IPX:
                        {
                                const struct sockaddr_ipx *ip =
                                                (struct sockaddr_ipx *)saddr;
                                asprintf(&out, "%s port:%d net:%u", str,
                                        ip->sipx_port, ip->sipx_network);
                        }
                        break;
                case AF_ATMPVC:
                        {
                                const struct sockaddr_atmpvc* at =
                                        (struct sockaddr_atmpvc *)saddr;
                                asprintf(&out, "%s int:%d", str, 
						at->sap_addr.itf);
                        }
                        break;
                case AF_X25:
                        {
                                const struct sockaddr_x25* x =
                                        (struct sockaddr_x25 *)saddr;
                                asprintf(&out, "%s addr:%.15s", str,
						x->sx25_addr.x25_addr);
                        }
                        break;
                case AF_INET6:
                        if (slen < sizeof(struct sockaddr_in6)) {
                                asprintf(&out, "%s sockaddr6 len too short", 
						str);
                                free((char *)host);
                                return out;
                        }
                        slen = sizeof(struct sockaddr_in6);
                        if (getnameinfo(saddr, slen, name, NI_MAXHOST, serv,
                                NI_MAXSERV, NI_NUMERICHOST |
                                        NI_NUMERICSERV) == 0 ) {
                                asprintf(&out, "%s host:%s serv:%s", str,
						name, serv);
                        } else
                                asprintf(&out, "%s (error resolving addr)",
						str);
                        break;
                case AF_NETLINK:
                        {
                                const struct sockaddr_nl *n =
                                                (struct sockaddr_nl *)saddr;
                                asprintf(&out, "%s pid:%u", str, n->nl_pid);
                        }
                        break;
        }
        free((char *)host);
	return out;
}

static const char *print_flags(const char *val)
{
        int flags, cnt = 0;
	size_t i;
	char *out, buf[80];

        errno = 0;
        flags = strtoul(val, NULL, 16);
        if (errno) {
                asprintf(&out, "conversion error(%s)", val);
                return out;
        }
        if (flags == 0) {
                asprintf(&out, "none");
                return out;
        }
	buf[0] = 0;
        for (i=0; i<FLAG_NUM_ENTRIES; i++) {
                if (flag_table[i].value & flags) {
                        if (!cnt) {
                                strcat(buf,
				       flag_strings + flag_table[i].offset);
                                cnt++;
                        } else {
                                strcat(buf, ",");
                                strcat(buf,
				       flag_strings + flag_table[i].offset);
			}
                }
        }
	return strdup(buf);
}

static const char *print_promiscuous(const char *val)
{
        int ival;

        errno = 0;
        ival = strtol(val, NULL, 10);
        if (errno) {
		char *out;
                asprintf(&out, "conversion error(%s)", val);
                return out;
        }

        if (ival == 0)
                return strdup("no");
        else
                return strdup("yes");
}

static const char *print_capabilities(const char *val)
{
        int cap;
	char *out;
	const char *s;

        errno = 0;
        cap = strtoul(val, NULL, 10);
        if (errno) {
                asprintf(&out, "conversion error(%s)", val);
                return out;
        }

	s = cap_i2s(cap);
	if (s != NULL)
		return strdup(s);
	asprintf(&out, "unknown capability(%s)", val);
	return out;
}

static const char *print_success(const char *val)
{
        int res;

	if (isdigit(*val)) {
	        errno = 0;
        	res = strtoul(val, NULL, 10);
	        if (errno) {
			char *out;
                	asprintf(&out, "conversion error(%s)", val);
	                return out;
        	}

	        return strdup(aulookup_success(res));
	} else
		return strdup(val);
}

static const char *print_open_flags(int flags)
{
	size_t i;
	int cnt = 0;
	char buf[144];

	buf[0] = 0;
        if ((flags & O_ACCMODE) == 0) {
		// Handle O_RDONLY specially
                strcat(buf, "O_RDONLY");
                cnt++;
        }
        for (i=0; i<OPEN_FLAG_NUM_ENTRIES; i++) {
                if (open_flag_table[i].value & flags) {
                        if (!cnt) {
                                strcat(buf,
				open_flag_strings + open_flag_table[i].offset);
                                cnt++;
                        } else {
                                strcat(buf, "|");
                                strcat(buf,
				open_flag_strings + open_flag_table[i].offset);
			}
                }
        }
	return strdup(buf);
}

static const char *print_clone_flags(int flags)
{
        size_t i;
	int cnt = 0;
	char buf[352];

	buf[0] = 0;
        for (i=0; i<CLONE_FLAG_NUM_ENTRIES; i++) {
                if (clone_flag_table[i].value & flags) {
                        if (!cnt) {
                                strcat(buf,
			clone_flag_strings + clone_flag_table[i].offset);
                                cnt++;
                        } else {
                                strcat(buf, "|");
                                strcat(buf,
			clone_flag_strings + clone_flag_table[i].offset);
			}
                }
        }
	if (buf[0] == 0)
		snprintf(buf, sizeof(buf), "%d", flags);
	return strdup(buf);
}

static const char *print_fcntl_cmd(int cmd)
{
	char *out;
	const char *s;

	s = fcntl_i2s(cmd);
	if (s != NULL)
		return strdup(s);
	asprintf(&out, "unknown fcntl command(%d)", cmd);
	return out;
}

static const char *print_a0(const char *val, const rnode *r)
{
	int machine = r->machine, syscall = r->syscall;
	char *out;
	const char *sys = audit_syscall_to_name(syscall, machine);
	if (sys) {
		if (strcmp(sys, "clone") == 0) {
			int ival;

			errno = 0;
			ival = strtoul(val, NULL, 16);
		        if (errno) {
                		asprintf(&out, "conversion error(%s)", val);
	                	return out;
	        	}
			return print_clone_flags(ival);
		}
	}
	return strdup(val);
}

static const char *print_a1(const char *val, const rnode *r)
{
	int machine = r->machine, syscall = r->syscall;
	char *out;
	const char *sys = audit_syscall_to_name(syscall, machine);
	if (sys) {
		if (strcmp(sys, "open") == 0) {
			int ival;

			errno = 0;
			ival = strtoul(val, NULL, 16);
		        if (errno) {
                		asprintf(&out, "conversion error(%s)", val);
	                	return out;
	        	}
			return print_open_flags(ival);
		} else if (strncmp(sys, "fcntl", 5) == 0) {
			int ival;

			errno = 0;
			ival = strtoul(val, NULL, 16);
		        if (errno) {
                		asprintf(&out, "conversion error(%s)", val);
	                	return out;
	        	}
			return print_fcntl_cmd(ival);
		}
	}
	return strdup(val);
}

static const char *print_a2(const char *val, const rnode *r)
{
	int machine = r->machine, syscall = r->syscall;
	char *out;
	const char *sys = audit_syscall_to_name(syscall, machine);
	if (sys) {
		if (strncmp(sys, "fcntl", 5) == 0) {
			int ival;

			errno = 0;
			ival = strtoul(val, NULL, 16);
		        if (errno) {
                		asprintf(&out, "conversion error(%s)", val);
	                	return out;
	        	}
			switch (r->a1)
			{
				case F_SETOWN:
					return print_uid(val);
				case F_SETFD:
					if (ival == FD_CLOEXEC)
						return strdup("FD_CLOEXEC");
					/* Fall thru okay. */
				case F_SETFL:
				case F_SETLEASE:
				case F_GETLEASE:
				case F_NOTIFY:
					break;
			}
		}
	}
	return strdup(val);
}

static const char *print_signals(const char *val)
{
	int i;
	char *out;

	errno = 0;
        i = strtoul(val, NULL, 10);
	if (errno) 
		asprintf(&out, "conversion error(%s)", val);
	else
		out = strdup(strsignal(i));
	return out;
}

static const char *print_list(const char *val)
{
	int i;
	char *out;

	errno = 0;
        i = strtoul(val, NULL, 10);
	if (errno) 
		asprintf(&out, "conversion error(%s)", val);
	else
		out = strdup(audit_flag_to_name(i));
	return out;
}

static int audit_lookup_type(const char *name)
{
	int i;

	if (type_s2i(name, &i) != 0)
		return i;
	return -1;
}

static int is_hex_string(const char *str)
{
	while (*str) {
		if (!isxdigit(*str))
			return 0;
		str++;
	}
	return 1;
}

const char *interpret(const rnode *r)
{
	const nvlist *nv = &r->nv;
	int type, comma = 0;
	nvnode *n;
	const char *out;
	const char *name = nvlist_get_cur_name(nv);
	const char *val = nvlist_get_cur_val(nv);

	/* Do some fixups */
	if (r->type == AUDIT_EXECVE && name[0] == 'a')
		type = T_ESCAPED;
	else if (r->type == AUDIT_AVC && strcmp(name, "saddr") == 0)
		type = -1;
	else if (strcmp(name, "acct") == 0) {
		if (val[0] == '"')
			type = T_ESCAPED;
		else if (is_hex_string(val))
			type = T_ESCAPED;
		else
			type = -1;
	} else
		type = audit_lookup_type(name);

	switch(type) {
		case T_UID:
			out = print_uid(val);
			break;
		case T_GID:
			out = print_gid(val);
			break;
		case T_SYSCALL:
			out = print_syscall(val, r);
			break;
		case T_ARCH:
			out = print_arch(val, r->machine);
			break;
		case T_EXIT:
			out = print_exit(val);
			break;
		case T_ESCAPED:
			out = print_escaped(val);
                        break;
		case T_PERM:
			out = print_perm(val);
			break;
		case T_MODE:
			out = print_mode(val);
			break;
		case T_SOCKADDR:
			out = print_sockaddr(val);
			break;
		case T_FLAGS:
			out = print_flags(val);
			break;
		case T_PROMISC:
			out = print_promiscuous(val);
			break;
		case T_CAPABILITY:
			out = print_capabilities(val);
			break;
		case T_SUCCESS:
			out = print_success(val);
			break;
		case T_A0:
			out = print_a0(val, r);
			break;
		case T_A1:
			out = print_a1(val, r);
			break;
		case T_A2:
			out = print_a2(val, r);
			break; 
		case T_SIGNAL:
			out = print_signals(val);
			break; 
		case T_LIST:
			out = print_list(val);
			break; 
		default: {
			char *out2;
			if (comma)
				asprintf(&out2, "%s,", val);
			else
				out2 = strdup(val);
			out = out2;
			}
        }

	n = nvlist_get_cur(nv);
	n->interp_val = (char *)out;

	return out;
}