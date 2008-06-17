/*
* ausearch-int.h - Header file for ausearch-int.c
* Copyright (c) 2005 Red Hat Inc., Durham, North Carolina.
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

#ifndef AUINT_HEADER
#define AUINT_HEADER

#include "config.h"
#include <sys/types.h>
#include "libaudit.h"

/* This is the node of the linked list. Number & item are the only elements
 * at this time. Any data elements that are per item goes here. */
typedef struct _int_node{
  int num;		// The number
  unsigned int hits;	// The number of times this was attempted to be added
  int aux1;		// Extra spot for data
  unsigned int item;	// Which item of the same event
  struct _int_node* next;	// Next string node pointer
} int_node;

/* This is the linked list head. Only data elements that are 1 per
 * event goes here. */
typedef struct {
  int_node *head;		// List head
  int_node *cur;		// Pointer to current node
  unsigned int cnt;	// How many items in this list
} ilist;

void ilist_create(ilist *l);
static inline void ilist_first(ilist *l) { l->cur = l->head; }
void ilist_last(ilist *l);
int_node *ilist_next(ilist *l);
int_node *ilist_prev(ilist *l);
static inline int_node *ilist_get_cur(ilist *l) { return l->cur; }
void ilist_append(ilist *l, int num, unsigned int hits, int aux);
void ilist_clear(ilist* l);

/* Given a numeric index, find that record. */
int ilist_find_item(ilist *l, unsigned int i);

/* append a number if its not already on the list */
int ilist_add_if_uniq(ilist *l, int num, int aux);
void ilist_sort_by_hits(ilist *l);

#endif

