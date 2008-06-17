/* interpret.h --
 * Copyright 2007,08 Red Hat Inc., Durham, North Carolina.
 * All Rights Reserved.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors:
 *      Steve Grubb <sgrubb@redhat.com>
 */

#ifndef INTERPRET_HEADER
#define INTERPRET_HEADER

#include "config.h"
#include "private.h"
#include "rnode.h"
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* This is a list of field types that we can interpret */
enum { T_UID, T_GID, T_SYSCALL, T_ARCH, T_EXIT, T_ESCAPED, T_PERM, T_MODE,
T_SOCKADDR, T_FLAGS, T_PROMISC, T_CAPABILITY, T_SUCCESS, T_A0, T_A1, T_A2,
T_SIGNAL, T_LIST };

const char *interpret(const rnode *r);
void aulookup_destroy_uid_list(void);
void aulookup_destroy_gid_list(void);
const char *au_unescape(char *buf);

/* Make these hidden to prevent conflicts */
hidden_proto(interpret);
hidden_proto(aulookup_destroy_uid_list);
hidden_proto(aulookup_destroy_gid_list);
hidden_proto(au_unescape);

#ifdef __cplusplus
}
#endif
 
#endif

