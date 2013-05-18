/* ircd-micro, trie.c -- radix trie
 * Copyright (C) 2013 Alex Iadicicco
 * Copyright (C) 2013 Stephen Chandler Paul
 *
 * This file was taken from the ircd-micro project and modified to match the
 * styling guidelines of SquirrelChat. As such, it follows a different license
 * then the rest of the source tree.
 *
 * The original project may be found here:
 * https://github.com/aji/ircd-micro
 *
 * The license is as follows:
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation, advertising
 * materials, and other materials related to such distribution and use
 * acknowledge that the software was developed by Alexander Iadicicco.
 * The name of Alexander Iadicicco may not be used to endorse or
 * promote products derived from this software without specific
 * prior written permission.  THIS SOFTWARE IS PROVIDED ``AS IS''
 * AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
 * LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE.
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

#endif
