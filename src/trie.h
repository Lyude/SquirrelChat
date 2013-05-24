/* 
 * Copyright (C) 2013 Stephen Chandler Paul
 *
 * This file is free software: you may copy it, redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 2 of this License or (at your option) any
 * later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyright and permission
 * notice:
 *
 * 	Copyright (C) 2013 Alex Iadicicco
 *
 *	Redistribution and use in source and binary forms are permitted provided
 *	that the above copyright notice and this paragraph are duplicated in all
 *	such forms and that any documentation, advertising materials, and any
 *	other materials related to such distribution and use acknowledge that
 *	the software was developed by Alexander Iadicicco. The name of Alexander
 *	Iadicicco may not be used to endorse or promote products derived from
 *	this software without specific prior written permission. THIS SOFTWARE
 *	IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
 *	INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 *	AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * The project the incorporated work originates from may be found here:
 * <https://github.com/aji/ircd-micro>
 */

#ifndef __INC_TRIE_H__
#define __INC_TRIE_H__

/* This is used for some internal buffers in the trie functions. Since these
 * bytes are allocated on the stack, you can set this pretty high. Think of
 * the longest key you might need to insert, then multiply by 4x
 */
#define U_TRIE_KEY_MAX 2048

typedef struct trie trie;
typedef struct trie_e trie_e;

struct trie_e {
	void *val;
	trie_e *up;
	trie_e *n[16];
};

struct trie {
	void (*canonize)(); /* char *key */
	trie_e n;
};

extern trie *trie_new(void (*canonize)());
extern void trie_set(trie * trie, char * key, void * val);
extern void *trie_get(trie * trie, char * key);
/* void cb(void *value, void *priv); */
extern void trie_each(trie * trie, void(*cb)(), void * priv);
extern void *trie_del(trie * trie, char * key);

extern void trie_strtolower(char * s);
extern void trie_strtoupper(char * s);

#endif
